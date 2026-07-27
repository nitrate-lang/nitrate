use super::parse::Parser;
use crate::diagnosis::SyntaxErr;

use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_tree::ast::{Exclusivity, Expr, FuncParam, Mutability, Type, Visibility};

/// Default maximum for counted parser elements (items, parameters, fields, etc.).
pub(crate) const MAX_LIMIT: usize = 65_536;

impl Parser<'_, '_> {
    /// Consumes a `::` token pair. Returns `true` if both colons were consumed.
    pub(crate) fn parse_double_colon(&mut self) -> bool {
        if !self.lexer.skip_if(&Token::Colon) {
            return false;
        }

        !!self.lexer.skip_if(&Token::Colon)
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

    /// Parse an optional return type (`-> Type`). Returns `None` if no arrow is found.
    pub(crate) fn parse_return_type_arrow(&mut self) -> Option<Type> {
        if self.lexer.skip_if(&Token::Minus) {
            if !self.lexer.skip_if(&Token::Gt) {
                self.log.report(&SyntaxErr::ExpectedArrow(self.lexer.peek_pos()));
            }
            Some(self.parse_type())
        } else {
            None
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

    // === Extracted common patterns (Iteration 3) ===

    /// Parse a keyword + attributes + name prefix pattern used in item declarations.
    ///
    /// Consumes `keyword` token (asserted), then attributes and identifier.
    /// Returns `(attributes, name)`.
    #[allow(dead_code)]
    pub(crate) fn parse_keyword_attrs_name(
        &mut self,
        keyword: &Token,
        missing_name: SyntaxErr,
    ) -> (Option<Vec<Expr>>, NString) {
        debug_assert_eq!(self.lexer.peek_tok().token, *keyword);
        self.lexer.skip_tok();
        let attributes = self.parse_attributes();
        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            self.log.report(&missing_name);
            String::new()
        });
        (attributes, NString::from(name))
    }

    /// Parse a common function parameter (reused in named functions, closures, and function types).
    ///
    /// Parses `[attributes] [mutability] name: Type [= default]`.
    /// When `allow_default` is false, the `= default` part is skipped.
    #[allow(dead_code)]
    pub(crate) fn parse_common_func_param(&mut self, allow_default: bool) -> FuncParam {
        let attributes = self.parse_attributes();
        let mutability = self.parse_mutability();
        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::FunctionParameterMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            String::new()
        });
        let name = NString::from(name);
        self.expect_colon();
        let ty = self.parse_type();
        let default_value = if allow_default && self.lexer.skip_if(&Token::Eq) {
            Some(self.parse_expression())
        } else {
            None
        };
        FuncParam {
            attributes,
            mutability,
            name,
            ty,
            default_value,
        }
    }

    /// Parse a brace-delimited body of items with limit checking and EOF recovery.
    ///
    /// Parses `{ item item item ... }` using the provided `parse_one` closure.
    /// Each item is parsed via `parse_one`, errors `eof_err` on EOF, and `limit_err`
    /// when exceeding `max` items.
    #[allow(dead_code)]
    pub(crate) fn parse_brace_body<T>(
        &mut self,
        max: usize,
        eof_err: SyntaxErr,
        limit_err: SyntaxErr,
        mut parse_one: impl FnMut(&mut Self) -> T,
    ) -> Vec<T> {
        let mut items = Vec::new();
        let mut limit_reported = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                self.log.report(&eof_err);
                break;
            }

            Self::check_limit(items.len(), max, &mut limit_reported, &limit_err, self.log);
            items.push(parse_one(self));
        }

        items
    }
}
