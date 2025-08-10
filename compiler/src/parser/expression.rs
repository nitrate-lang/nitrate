use super::parse::*;
use crate::lexer::*;
use crate::parsetree::*;
use slog::error;

impl<'storage, 'logger, 'a> Parser<'storage, 'logger, 'a> {
    pub(crate) fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        let mut parenthesis_count = 0;
        while self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            parenthesis_count += 1;
        }

        let Some(expr) = self.parse_expression_primary() else {
            self.set_failed_bit();
            return None;
        };

        self.storage.add_parentheses(expr);

        for _ in 0..parenthesis_count {
            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: expected closing parenthesis\n--> {}",
                    self.lexer.sync_position()
                );
                return None;
            }
        }

        // TODO: Binary expression parsing logic
        // TODO: Unary prefix expression parsing logic
        // TODO: Unary postfix expression parsing logic

        Some(expr)
    }

    fn parse_integer_literal(&mut self, value: u128, kind: IntegerKind) -> ExprKey<'a> {
        let mut bb = Builder::new(self.storage).create_integer().with_kind(kind);

        if value <= 0xff {
            bb = bb.with_u8(value as u8);
        } else if value <= 0xffff {
            bb = bb.with_u16(value as u16);
        } else if value <= 0xffff_ffff {
            bb = bb.with_u32(value as u32);
        } else if value <= 0xffff_ffff_ffff_ffff {
            bb = bb.with_u64(value as u64);
        } else {
            bb = bb.with_u128(value);
        }

        bb.build()
    }

    fn parse_float_literal(&mut self, value: f64) -> ExprKey<'a> {
        Builder::new(self.storage)
            .create_float()
            .with_value(value)
            .build()
    }

    fn parse_string_literal(&mut self, content: StringData<'a>) -> ExprKey<'a> {
        Builder::new(self.storage)
            .create_string()
            .with_string(content)
            .build()
    }

    fn parse_bstring_literal(&mut self, content: BStringData<'a>) -> ExprKey<'a> {
        Builder::new(self.storage)
            .create_bstring()
            .with_value(content)
            .build()
    }

    fn parse_char_literal(&mut self, value: char) -> ExprKey<'a> {
        Builder::new(self.storage).create_char(value)
    }

    fn parse_literal_suffix(&mut self, _lit: ExprKey<'a>) -> Option<ExprKey<'a>> {
        // TODO: Implement literal suffix parsing logic
        unimplemented!()
    }

    fn parse_type_or_type_alias(&mut self) -> Option<ExprKey<'a>> {
        assert!(&self.lexer.peek_t() == &Token::Keyword(Keyword::Type));

        let rewind_pos = self.lexer.sync_position();
        self.lexer.skip();

        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            let Some(the_type) = self.parse_type() else {
                self.lexer.rewind(rewind_pos);
                return None;
            };

            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: type: expected closing parenthesis\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }

            return Some(the_type.into());
        }

        self.lexer.rewind(rewind_pos);

        self.parse_type_alias()
    }

    fn parse_expression_primary(&mut self) -> Option<ExprKey<'a>> {
        match self.lexer.peek_t() {
            Token::Integer(int) => {
                self.lexer.skip();
                let lit = self.parse_integer_literal(int.value(), int.kind());
                self.parse_literal_suffix(lit)
            }

            Token::Float(float) => {
                self.lexer.skip();
                let lit = self.parse_float_literal(float);
                self.parse_literal_suffix(lit)
            }

            Token::String(string) => {
                self.lexer.skip();
                let lit = self.parse_string_literal(string);
                self.parse_literal_suffix(lit)
            }

            Token::BString(data) => {
                self.lexer.skip();
                let lit = self.parse_bstring_literal(data);
                self.parse_literal_suffix(lit)
            }

            Token::Char(character) => {
                self.lexer.skip();
                let lit = self.parse_char_literal(character);
                self.parse_literal_suffix(lit)
            }

            Token::Name(_name) => {
                // TODO: Implement identifier parsing logic
                None
            }

            Token::Keyword(Keyword::Enum) => self.parse_enum(),
            Token::Keyword(Keyword::Struct) => self.parse_struct(),
            Token::Keyword(Keyword::Class) => self.parse_class(),
            Token::Keyword(Keyword::Trait) => self.parse_trait(),
            Token::Keyword(Keyword::Impl) => self.parse_implementation(),
            Token::Keyword(Keyword::Contract) => self.parse_contract(),
            Token::Keyword(Keyword::Let) => self.parse_let_variable(),
            Token::Keyword(Keyword::Var) => self.parse_var_variable(),
            Token::Keyword(Keyword::Type) => self.parse_type_or_type_alias(),
            Token::Keyword(Keyword::Fn) => self.parse_function(),

            Token::Keyword(Keyword::If) => self.parse_if(),
            Token::Keyword(Keyword::For) => self.parse_for(),
            Token::Keyword(Keyword::While) => self.parse_while(),
            Token::Keyword(Keyword::Do) => self.parse_do(),
            Token::Keyword(Keyword::Switch) => self.parse_switch(),
            Token::Keyword(Keyword::Break) => self.parse_break(),
            Token::Keyword(Keyword::Continue) => self.parse_continue(),
            Token::Keyword(Keyword::Ret) => self.parse_return(),
            Token::Keyword(Keyword::Foreach) => self.parse_foreach(),
            Token::Keyword(Keyword::Await) => self.parse_await(),
            Token::Keyword(Keyword::Asm) => self.parse_asm(),
            Token::Keyword(Keyword::Assert) => self.parse_assert(),

            Token::Keyword(keyword) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: unexpected keyword '{}'\n--> {}",
                    keyword,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Op(op) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: unexpected operator '{}'\n--> {}",
                    op,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Punct(punct) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: unexpected punctuation '{}'\n--> {}",
                    punct,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Comment(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: unexpected comment\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Eof => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: unexpected end of file\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Illegal => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: illegal token\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }
        }
    }
}
