use super::parse::Parser;
use crate::lexer::{BStringData, IntegerKind, Keyword, Punct, StringData, Token};
use crate::parsetree::{Builder, ExprKey, node::BinExprOp};
use slog::error;

impl<'a> Parser<'_, '_, 'a> {
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

    fn parse_literal_suffix(&mut self, lit: ExprKey<'a>) -> ExprKey<'a> {
        let bb = Builder::new(self.storage);
        let type_name = match self.lexer.peek_t() {
            Token::Name(name) => Some(bb.create_type_name(name.name())),

            Token::Keyword(Keyword::Bool) => Some(bb.get_bool()),
            Token::Keyword(Keyword::U8) => Some(bb.get_u8()),
            Token::Keyword(Keyword::U16) => Some(bb.get_u16()),
            Token::Keyword(Keyword::U32) => Some(bb.get_u32()),
            Token::Keyword(Keyword::U64) => Some(bb.get_u64()),
            Token::Keyword(Keyword::U128) => Some(bb.get_u128()),
            Token::Keyword(Keyword::I8) => Some(bb.get_i8()),
            Token::Keyword(Keyword::I16) => Some(bb.get_i16()),
            Token::Keyword(Keyword::I32) => Some(bb.get_i32()),
            Token::Keyword(Keyword::I64) => Some(bb.get_i64()),
            Token::Keyword(Keyword::I128) => Some(bb.get_i128()),
            Token::Keyword(Keyword::F8) => Some(bb.get_f8()),
            Token::Keyword(Keyword::F16) => Some(bb.get_f16()),
            Token::Keyword(Keyword::F32) => Some(bb.get_f32()),
            Token::Keyword(Keyword::F64) => Some(bb.get_f64()),
            Token::Keyword(Keyword::F128) => Some(bb.get_f128()),

            _ => None,
        };

        if let Some(type_name) = type_name {
            self.lexer.skip_tok();

            Builder::new(self.storage)
                .create_binexpr()
                .with_left(lit)
                .with_operator(BinExprOp::As)
                .with_right(type_name.into())
                .build()
        } else {
            lit
        }
    }

    fn parse_type_or_type_alias(&mut self) -> Option<ExprKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Type));

        let rewind_pos = self.lexer.sync_position();
        self.lexer.skip_tok();

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

    fn parse_if(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement if expression parsing logic
        self.set_failed_bit();
        error!(self.log, "If expression parsing not implemented yet");
        None
    }

    fn parse_for(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement for expression parsing logic
        self.set_failed_bit();
        error!(self.log, "For expression parsing not implemented yet");
        None
    }

    fn parse_while(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement while expression parsing logic
        self.set_failed_bit();
        error!(self.log, "While expression parsing not implemented yet");
        None
    }

    fn parse_do(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement do expression parsing logic
        self.set_failed_bit();
        error!(self.log, "Do expression parsing not implemented yet");
        None
    }

    fn parse_switch(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement switch expression parsing logic
        self.set_failed_bit();
        error!(self.log, "Switch expression parsing not implemented yet");
        None
    }

    fn parse_break(&mut self) -> Option<ExprKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Break));
        self.lexer.skip_tok();

        // TODO: Implement break branch label parsing logic

        Some(Builder::new(self.storage).create_break().build())
    }

    fn parse_continue(&mut self) -> Option<ExprKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Continue));
        self.lexer.skip_tok();

        // TODO: Implement continue branch label parsing logic

        Some(Builder::new(self.storage).create_continue().build())
    }

    fn parse_return(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement return expression parsing logic
        self.set_failed_bit();
        error!(self.log, "Return expression parsing not implemented yet");
        None
    }

    fn parse_foreach(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement foreach expression parsing logic
        self.set_failed_bit();
        error!(self.log, "Foreach expression parsing not implemented yet");
        None
    }

    fn parse_await(&mut self) -> Option<ExprKey<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Await));
        self.lexer.skip_tok();

        let Some(expr) = self.parse_expression() else {
            self.set_failed_bit();
            return None;
        };

        Some(
            Builder::new(self.storage)
                .create_await()
                .with_expression(expr)
                .build(),
        )
    }

    fn parse_asm(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement asm expression parsing logic
        self.set_failed_bit();
        error!(self.log, "Asm expression parsing not implemented yet");
        None
    }

    fn parse_assert(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement assert expression parsing logic
        self.set_failed_bit();
        error!(self.log, "Assert expression parsing not implemented yet");
        None
    }

    fn parse_type_alias(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement type alias parsing logic
        self.set_failed_bit();
        error!(self.log, "Type alias parsing not implemented yet");
        None
    }

    fn parse_enum(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement enum parsing logic
        self.set_failed_bit();
        error!(self.log, "Enum parsing not implemented yet");
        None
    }

    fn parse_struct(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement struct parsing logic
        self.set_failed_bit();
        error!(self.log, "Struct parsing not implemented yet");
        None
    }

    fn parse_class(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement class parsing logic
        self.set_failed_bit();
        error!(self.log, "Class parsing not implemented yet");
        None
    }

    fn parse_trait(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement trait parsing logic
        self.set_failed_bit();
        error!(self.log, "Trait parsing not implemented yet");
        None
    }

    fn parse_implementation(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement implementation parsing logic
        self.set_failed_bit();
        error!(self.log, "Implementation parsing not implemented yet");
        None
    }

    fn parse_contract(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement contract parsing logic
        self.set_failed_bit();
        error!(self.log, "Contract parsing not implemented yet");
        None
    }

    fn parse_function(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement function parsing logic
        self.set_failed_bit();
        error!(self.log, "Function parsing not implemented yet");
        None
    }

    fn parse_let_variable(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement let variable parsing logic
        self.set_failed_bit();
        error!(self.log, "Let variable parsing not implemented yet");
        None
    }

    fn parse_var_variable(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement var variable parsing logic
        self.set_failed_bit();
        error!(self.log, "Var variable parsing not implemented yet");
        None
    }

    fn parse_expression_primary(&mut self) -> Option<ExprKey<'a>> {
        match self.lexer.peek_t() {
            Token::Integer(int) => {
                self.lexer.skip_tok();
                let lit = self.parse_integer_literal(int.value(), int.kind());
                Some(self.parse_literal_suffix(lit))
            }

            Token::Float(float) => {
                self.lexer.skip_tok();
                let lit = self.parse_float_literal(float);
                Some(self.parse_literal_suffix(lit))
            }

            Token::String(string) => {
                self.lexer.skip_tok();
                let lit = self.parse_string_literal(string);
                Some(self.parse_literal_suffix(lit))
            }

            Token::BString(data) => {
                self.lexer.skip_tok();
                let lit = self.parse_bstring_literal(data);
                Some(self.parse_literal_suffix(lit))
            }

            Token::Char(character) => {
                self.lexer.skip_tok();
                let lit = self.parse_char_literal(character);
                Some(self.parse_literal_suffix(lit))
            }

            Token::Name(name) => {
                self.lexer.skip_tok();
                Some(Builder::new(self.storage).create_identifier(name.name()))
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

    pub(crate) fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        let mut parenthesis_count = 0;
        while self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            parenthesis_count += 1;
        }

        let Some(mut expr) = self.parse_expression_primary() else {
            self.set_failed_bit();
            return None;
        };

        if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            expr = Builder::new(self.storage)
                .create_statement()
                .with_expression(expr)
                .build();
        }

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
}
