use super::parse::Parser;
use crate::lexical::{IntegerKind, Keyword, Op, Punct, Token};
use crate::parsetree::{Builder, Expr, nodes::BinExprOp};
use log::error;

impl<'a> Parser<'a, '_> {
    fn parse_integer_literal(&mut self, value: u128, kind: IntegerKind) -> Expr<'a> {
        let mut bb = Builder::create_integer().with_kind(kind);

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

    fn parse_literal_suffix(&mut self, lit: Expr<'a>) -> Expr<'a> {
        let type_name = match self.lexer.peek_t() {
            Token::Name(name) => Some(Builder::create_type_name(name.name())),

            Token::Keyword(Keyword::Bool) => Some(Builder::get_bool()),
            Token::Keyword(Keyword::U8) => Some(Builder::get_u8()),
            Token::Keyword(Keyword::U16) => Some(Builder::get_u16()),
            Token::Keyword(Keyword::U32) => Some(Builder::get_u32()),
            Token::Keyword(Keyword::U64) => Some(Builder::get_u64()),
            Token::Keyword(Keyword::U128) => Some(Builder::get_u128()),
            Token::Keyword(Keyword::I8) => Some(Builder::get_i8()),
            Token::Keyword(Keyword::I16) => Some(Builder::get_i16()),
            Token::Keyword(Keyword::I32) => Some(Builder::get_i32()),
            Token::Keyword(Keyword::I64) => Some(Builder::get_i64()),
            Token::Keyword(Keyword::I128) => Some(Builder::get_i128()),
            Token::Keyword(Keyword::F8) => Some(Builder::get_f8()),
            Token::Keyword(Keyword::F16) => Some(Builder::get_f16()),
            Token::Keyword(Keyword::F32) => Some(Builder::get_f32()),
            Token::Keyword(Keyword::F64) => Some(Builder::get_f64()),
            Token::Keyword(Keyword::F128) => Some(Builder::get_f128()),

            _ => None,
        };

        if let Some(type_name) = type_name {
            self.lexer.skip_tok();

            Builder::create_binexpr()
                .with_left(lit)
                .with_operator(BinExprOp::As)
                .with_right(type_name.into())
                .build()
        } else {
            lit
        }
    }

    pub(crate) fn parse_list(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftBracket));
        self.lexer.skip_tok();

        let mut elements = Vec::new();
        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                break;
            }

            elements.push(self.parse_expression()?);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                    break;
                }
                error!(
                    "[P0???]: list: expected ',' or ']' after element expression\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }
        }

        Some(Builder::create_list().add_elements(elements).build())
    }

    pub(crate) fn parse_attributes(&mut self) -> Option<Vec<Expr<'a>>> {
        let mut attributes = Vec::new();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBracket)) {
            return Some(attributes);
        }

        self.lexer.skip_if(&Token::Punct(Punct::Comma));
        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                break;
            }

            attributes.push(self.parse_expression()?);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                if self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                    break;
                }
                error!(
                    "[P0???]: expected ',' or ']' after attribute expression\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }
        }

        Some(attributes)
    }

    fn parse_type_or_type_alias(&mut self) -> Option<Expr<'a>> {
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
                    "[P????]: type expression: expected closing parenthesis\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }

            return Some(the_type.into());
        }

        self.lexer.rewind(rewind_pos);

        self.parse_type_alias()
    }

    fn parse_if(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::If));
        self.lexer.skip_tok();

        let condition = self.parse_expression()?;
        let then_branch = self.parse_block()?;

        let else_branch = if self.lexer.skip_if(&Token::Keyword(Keyword::Else)) {
            if self.lexer.next_is(&Token::Keyword(Keyword::If)) {
                let Some(else_if_branch) = self.parse_if() else {
                    self.set_failed_bit();
                    error!(
                        "[P????]: if: expected else block after 'else if'\n--> {}",
                        self.lexer.sync_position()
                    );
                    return None;
                };

                Some(else_if_branch)
            } else {
                Some(self.parse_block()?)
            }
        } else {
            None
        };

        Some(
            Builder::create_if()
                .with_condition(condition)
                .with_then_branch(then_branch)
                .with_else_branch(else_branch)
                .build(),
        )
    }

    fn parse_for(&mut self) -> Option<Expr<'a>> {
        // TODO: for expression parsing logic
        self.set_failed_bit();
        error!("For expression parsing not implemented yet");
        None
    }

    fn parse_while(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::While));
        self.lexer.skip_tok();

        let condition = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            Builder::create_boolean(true)
        } else {
            self.parse_expression()?
        };

        let body = self.parse_block()?;

        Some(
            Builder::create_while_loop()
                .with_condition(condition)
                .with_body(body)
                .build(),
        )
    }

    fn parse_do(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Do));
        self.lexer.skip_tok();

        let body = self.parse_block()?;
        if !self.lexer.skip_if(&Token::Keyword(Keyword::While)) {
            error!(
                "[P????]: do-while: expected 'while' after 'do' block\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        }

        let condition = self.parse_expression()?;

        Some(
            Builder::create_do_while_loop()
                .with_body(body)
                .with_condition(condition)
                .build(),
        )
    }

    fn parse_switch(&mut self) -> Option<Expr<'a>> {
        // TODO: switch expression parsing logic
        self.set_failed_bit();
        error!("Switch expression parsing not implemented yet");
        None
    }

    fn parse_break(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Break));
        self.lexer.skip_tok();

        let branch_label = if self.lexer.skip_if(&Token::Punct(Punct::SingleQuote)) {
            let Some(label) = self.lexer.next_if_name() else {
                error!(
                    "[P????]: break: expected branch label after single quote\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            };

            Some(label.name())
        } else {
            None
        };

        Some(Builder::create_break().with_label(branch_label).build())
    }

    fn parse_continue(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Continue));
        self.lexer.skip_tok();

        let branch_label = if self.lexer.skip_if(&Token::Punct(Punct::SingleQuote)) {
            let Some(label) = self.lexer.next_if_name() else {
                error!(
                    "[P????]: continue: expected branch label after single quote\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            };

            Some(label.name())
        } else {
            None
        };

        Some(Builder::create_continue().with_label(branch_label).build())
    }

    fn parse_return(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Ret));
        self.lexer.skip_tok();

        let value = if self.lexer.next_is(&Token::Punct(Punct::Semicolon)) {
            None
        } else {
            Some(self.parse_expression()?)
        };

        Some(Builder::create_return().with_value(value).build())
    }

    fn parse_foreach(&mut self) -> Option<Expr<'a>> {
        // TODO: foreach expression parsing logic
        self.set_failed_bit();
        error!("Foreach expression parsing not implemented yet");
        None
    }

    fn parse_await(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Await));
        self.lexer.skip_tok();

        let Some(expr) = self.parse_expression() else {
            self.set_failed_bit();
            return None;
        };

        Some(Builder::create_await().with_expression(expr).build())
    }

    fn parse_asm(&mut self) -> Option<Expr<'a>> {
        // TODO: asm expression parsing logic
        self.set_failed_bit();
        error!("Asm expression parsing not implemented yet");
        None
    }

    fn parse_assert(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Assert));
        self.lexer.skip_tok();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            self.set_failed_bit();
            error!(
                "[P????]: assert: expected opening parenthesis\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        }

        self.lexer.skip_if(&Token::Punct(Punct::Comma));
        let condition = self.parse_expression()?;
        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        let message = if !self.lexer.next_is(&Token::Punct(Punct::RightParen)) {
            self.parse_expression()?
        } else {
            Builder::create_string_from_ref("")
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
            self.set_failed_bit();
            error!(
                "[P????]: assert: expected closing parenthesis\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        }

        Some(
            Builder::create_assert()
                .with_condition(condition)
                .with_message(message)
                .build(),
        )
    }

    fn parse_type_alias(&mut self) -> Option<Expr<'a>> {
        // TODO: type alias parsing logic
        self.set_failed_bit();
        error!("Type alias parsing not implemented yet");
        None
    }

    fn parse_scope(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Scope));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let Some(name_token) = self.lexer.next_if_name() else {
            error!(
                "[P????]: scope: expected scope name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        self.scope.push(name_token.name());

        let Some(elements) = self.parse_block_as_elements() else {
            self.scope.pop();
            return None;
        };

        self.scope.pop();

        Some(
            Builder::create_scope()
                .with_name(name_token.name())
                .add_attributes(attributes)
                .add_elements(elements)
                .build(),
        )
    }

    fn parse_enum(&mut self) -> Option<Expr<'a>> {
        // TODO: enum parsing logic
        self.set_failed_bit();
        error!("Enum parsing not implemented yet");
        None
    }

    fn parse_struct(&mut self) -> Option<Expr<'a>> {
        // TODO: struct parsing logic
        self.set_failed_bit();
        error!("Struct parsing not implemented yet");
        None
    }

    fn parse_class(&mut self) -> Option<Expr<'a>> {
        // TODO: class parsing logic
        self.set_failed_bit();
        error!("Class parsing not implemented yet");
        None
    }

    fn parse_trait(&mut self) -> Option<Expr<'a>> {
        // TODO: trait parsing logic
        self.set_failed_bit();
        error!("Trait parsing not implemented yet");
        None
    }

    fn parse_implementation(&mut self) -> Option<Expr<'a>> {
        // TODO: implementation parsing logic
        self.set_failed_bit();
        error!("Implementation parsing not implemented yet");
        None
    }

    fn parse_contract(&mut self) -> Option<Expr<'a>> {
        // TODO: contract parsing logic
        self.set_failed_bit();
        error!("Contract parsing not implemented yet");
        None
    }

    fn parse_function(&mut self) -> Option<Expr<'a>> {
        if self.lexer.peek_t() == Token::Punct(Punct::LeftBrace) {
            let block = Some(self.parse_block()?);

            let infer_type = Builder::get_infer_type();

            return Some(
                Builder::create_function()
                    .with_definition(block)
                    .with_return_type(infer_type)
                    .build(),
            );
        }

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;
        let parameters = self.parse_function_parameters()?;

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            self.parse_type()?
        } else {
            Builder::get_infer_type()
        };

        let body = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            Some(self.parse_block()?)
        } else {
            None
        };

        let function = Builder::create_function()
            .with_attributes(attributes)
            .with_parameters(parameters)
            .with_return_type(return_type)
            .with_definition(body)
            .build();

        Some(function)
    }

    fn parse_let_variable(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Let));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let name_pos = self.lexer.peek_tok().start();
        let variable_name = if let Some(name_token) = self.lexer.next_if_name() {
            name_token.name()
        } else {
            error!(
                "[P????]: let: expected variable name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let type_annotation = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            self.parse_type()?
        } else {
            Builder::get_infer_type()
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        let variable = Builder::create_let()
            .with_mutability(is_mutable)
            .with_attributes(attributes)
            .with_name(variable_name)
            .with_type(type_annotation)
            .with_value(initializer)
            .build();

        let current_scope = self.scope.clone();
        if !self
            .symtab
            .insert(current_scope, variable_name, variable.clone())
        {
            self.set_failed_bit();
            error!("[P????]: let: duplicate variable '{variable_name}'\n--> {name_pos}");
            // Fallthrough
        }

        Some(variable)
    }

    fn parse_var_variable(&mut self) -> Option<Expr<'a>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Var));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let name_pos = self.lexer.peek_tok().start();
        let variable_name = if let Some(name_token) = self.lexer.next_if_name() {
            name_token.name()
        } else {
            error!(
                "[P????]: var: expected variable name\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let type_annotation = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            self.parse_type()?
        } else {
            Builder::get_infer_type()
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        let variable = Builder::create_var()
            .with_mutability(is_mutable)
            .with_attributes(attributes)
            .with_name(variable_name)
            .with_type(type_annotation)
            .with_value(initializer)
            .build();

        let current_scope = self.scope.clone();
        if !self
            .symtab
            .insert(current_scope, variable_name, variable.clone())
        {
            self.set_failed_bit();
            error!("[P????]: var: duplicate variable '{variable_name}'\n--> {name_pos}");
            // Fallthrough
        }

        Some(variable)
    }

    fn parse_expression_primary(&mut self) -> Option<Expr<'a>> {
        match self.lexer.peek_t() {
            Token::Integer(int) => {
                self.lexer.skip_tok();
                let lit = self.parse_integer_literal(int.value(), int.kind());
                Some(self.parse_literal_suffix(lit))
            }

            Token::Float(float) => {
                self.lexer.skip_tok();
                let lit = Builder::create_float(float);
                Some(self.parse_literal_suffix(lit))
            }

            Token::String(string) => {
                self.lexer.skip_tok();
                let lit = Builder::create_string_from(string);
                Some(self.parse_literal_suffix(lit))
            }

            Token::BString(data) => {
                self.lexer.skip_tok();
                let lit = Builder::create_bstring_from(data);
                Some(self.parse_literal_suffix(lit))
            }

            Token::Punct(Punct::LeftBracket) => self.parse_list(),

            Token::Name(name) => {
                self.lexer.skip_tok();
                Some(Builder::create_identifier(name.name()))
            }

            Token::Keyword(Keyword::True) => {
                self.lexer.skip_tok();
                Some(Builder::create_boolean(true))
            }

            Token::Keyword(Keyword::False) => {
                self.lexer.skip_tok();
                Some(Builder::create_boolean(false))
            }

            Token::Keyword(Keyword::Scope) => self.parse_scope(),
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
            Token::Punct(Punct::LeftBrace) => self.parse_function(),

            Token::Keyword(keyword) => {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: unexpected keyword '{}'\n--> {}",
                    keyword,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Op(op) => {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: unexpected operator '{}'\n--> {}",
                    op,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Punct(punct) => {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: unexpected punctuation '{}'\n--> {}",
                    punct,
                    self.lexer.sync_position()
                );

                None
            }

            Token::Comment(_) => {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: unexpected comment\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Eof => {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: unexpected end of file\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Illegal => {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: illegal token\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }
        }
    }

    pub fn parse_expression(&mut self) -> Option<Expr<'a>> {
        let mut parenthesis_count = 0;
        while self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            parenthesis_count += 1;
        }

        let Some(mut expr) = self.parse_expression_primary() else {
            self.set_failed_bit();
            return None;
        };

        if parenthesis_count > 0 {
            expr = Builder::create_parentheses(expr);
        }

        for _ in 0..parenthesis_count {
            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: expected closing parenthesis\n--> {}",
                    self.lexer.sync_position()
                );
                return None;
            }
        }

        // TODO: binary expression parsing logic
        // TODO: unary prefix expression parsing logic
        // TODO: unary postfix expression parsing logic

        Some(expr)
    }

    pub(crate) fn parse_block_as_elements(&mut self) -> Option<Vec<Expr<'a>>> {
        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            self.set_failed_bit();
            error!(
                "[P????]: expr: block: expected opening brace\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        }

        let mut elements = Vec::new();
        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
                break;
            }

            if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                continue;
            }

            let Some(mut expression) = self.parse_expression() else {
                let before_pos = self.lexer.sync_position();
                loop {
                    match self.lexer.next_t() {
                        Token::Punct(Punct::Semicolon) | Token::Illegal | Token::Eof => {
                            // Resynchronize the lexer to the next semicolon
                            break;
                        }
                        _ => {}
                    }
                }

                if before_pos == self.lexer.sync_position() {
                    self.set_failed_bit();
                    error!(
                        "[P????]: block: failed to parse expression\n--> {}",
                        self.lexer.sync_position()
                    );

                    return None;
                }

                continue;
            };

            if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                expression = Builder::create_statement()
                    .with_expression(expression)
                    .build();
            }

            elements.push(expression);
        }

        Some(elements)
    }

    pub(crate) fn parse_block(&mut self) -> Option<Expr<'a>> {
        Some(
            Builder::create_block()
                .add_expressions(self.parse_block_as_elements()?)
                .build(),
        )
    }
}
