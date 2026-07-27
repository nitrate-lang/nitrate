use super::parse::Parser;
use crate::diagnosis::SyntaxErr;

use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_tree::ast::{Exclusivity, Mutability, Visibility};

impl Parser<'_, '_> {
    /// Consumes a `::` token pair. Returns `true` if both colons were consumed.
    pub(crate) fn parse_double_colon(&mut self) -> bool {
        if !self.lexer.skip_if(&Token::Colon) {
            return false;
        }

        if !self.lexer.skip_if(&Token::Colon) {
            false
        } else {
            true
        }
    }

    /// Expect and consume an identifier token. Reports the given error if missing.
    pub(crate) fn parse_name(&mut self, on_missing: SyntaxErr) -> NString {
        self.lexer
            .next_if_name()
            .unwrap_or_else(|| {
                self.log.report(&on_missing);
                String::new()
            })
            .into()
    }

    /// Expect a specific token. Reports the given error if not found.
    pub(crate) fn expect_token(&mut self, expected: &Token, on_missing: SyntaxErr) {
        if !self.lexer.skip_if(expected) {
            self.log.report(&on_missing);
        }
    }

    /// Expect a semicolon (`;`).
    pub(crate) fn expect_semicolon(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::Semi) {
            self.log.report(&SyntaxErr::ExpectedSemicolon(pos));
        }
    }

    /// Expect an opening brace (`{`).
    pub(crate) fn expect_open_brace(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::OpenBrace) {
            self.log.report(&SyntaxErr::ExpectedOpenBrace(pos));
        }
    }

    /// Expect a closing brace (`}`).
    pub(crate) fn expect_close_brace(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::CloseBrace) {
            self.log.report(&SyntaxErr::ExpectedCloseBrace(pos));
        }
    }

    /// Expect an opening parenthesis (`(`).
    pub(crate) fn expect_open_paren(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::OpenParen) {
            self.log.report(&SyntaxErr::ExpectedOpenParen(pos));
        }
    }

    /// Expect a closing parenthesis (`)`).
    pub(crate) fn expect_close_paren(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::CloseParen) {
            self.log.report(&SyntaxErr::ExpectedCloseParen(pos));
        }
    }

    /// Expect an opening bracket (`[`).
    pub(crate) fn expect_open_bracket(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::OpenBracket) {
            self.log.report(&SyntaxErr::ExpectedOpenBracket(pos));
        }
    }

    /// Expect a closing bracket (`]`).
    pub(crate) fn expect_close_bracket(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::CloseBracket) {
            self.log.report(&SyntaxErr::ExpectedCloseBracket(pos));
        }
    }

    /// Expect a colon (`:`).
    pub(crate) fn expect_colon(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::Colon) {
            self.log.report(&SyntaxErr::ExpectedColon(pos));
        }
    }

    /// Expect an arrow (`->`).
    pub(crate) fn expect_arrow(&mut self) {
        let pos = self.lexer.peek_pos();
        self.lexer.skip_if(&Token::Minus);
        let found = self.lexer.skip_if(&Token::Gt);
        if !found {
            self.log.report(&SyntaxErr::ExpectedArrow(pos));
        }
    }

    /// Expect a closing angle bracket (`>`).
    pub(crate) fn expect_close_angle(&mut self) {
        let pos = self.lexer.peek_pos();
        if !self.lexer.skip_if(&Token::Gt) {
            self.log.report(&SyntaxErr::ExpectedCloseAngle(pos));
        }
    }

    /// Parse optional mutability modifier (`mut` / `const`).
    pub(crate) fn parse_mutability(&mut self) -> Option<Mutability> {
        if self.lexer.skip_if(&Token::Mut) {
            Some(Mutability::Mut)
        } else if self.lexer.skip_if(&Token::Const) {
            Some(Mutability::Const)
        } else {
            None
        }
    }

    /// Parse optional exclusivity modifier (`^` / `!`).
    pub(crate) fn parse_exclusivity(&mut self) -> Option<Exclusivity> {
        if self.lexer.skip_if(&Token::Poly) {
            Some(Exclusivity::Poly)
        } else if self.lexer.skip_if(&Token::Iso) {
            Some(Exclusivity::Iso)
        } else {
            None
        }
    }

    /// Parse optional visibility modifier (`pub` / `sec` / `pro`).
    pub(crate) fn parse_visibility(&mut self) -> Option<Visibility> {
        if self.lexer.skip_if(&Token::Pub) {
            Some(Visibility::Public)
        } else if self.lexer.skip_if(&Token::Sec) {
            Some(Visibility::Private)
        } else if self.lexer.skip_if(&Token::Pro) {
            Some(Visibility::Protected)
        } else {
            None
        }
    }

    /// Check if a limit has been exceeded, reporting the error only once.
    pub(crate) fn check_limit(
        count: usize,
        limit: usize,
        already_reported: &mut bool,
        err: &SyntaxErr,
        log: &nitrate_diagnosis::CompilerLog,
    ) {
        if !*already_reported && count >= limit {
            *already_reported = true;
            log.report(err);
        }
    }

    /// Parse a comma-separated list of elements up to a closing `close` delimiter.
    ///
    /// The `open` delimiter (if any) should already have been consumed before calling
    /// this method. Returns the parsed elements.
    pub(crate) fn parse_comma_separated_list<T>(
        &mut self,
        close: &Token,
        max: usize,
        allow_leading_comma: bool,
        eof_err: SyntaxErr,
        limit_err: SyntaxErr,
        end_err: SyntaxErr,
        mut parse_one: impl FnMut(&mut Self) -> T,
    ) -> Vec<T> {
        let mut items = Vec::new();
        let mut limit_reported = false;

        if allow_leading_comma {
            self.lexer.skip_if(&Token::Comma);
        }

        while !self.lexer.skip_if(close) {
            if self.lexer.is_eof() {
                self.log.report(&eof_err);
                break;
            }

            Self::check_limit(items.len(), max, &mut limit_reported, &limit_err, self.log);

            items.push(parse_one(self));

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(close) {
                self.log.report(&end_err);
                self.lexer.skip_while(close);
                break;
            }
        }

        items
    }
}
