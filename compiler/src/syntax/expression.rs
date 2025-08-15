use super::parse::Parser;
use crate::lexical::{BStringData, IntegerKind, Keyword, Op, Punct, StringData, Token};
use crate::parsetree::{Builder, Expr, node::BinExprOp};
use log::error;
use std::ops::Deref;
use std::sync::Arc;

impl<'a> Parser<'a, '_> {
    fn parse_integer_literal(&mut self, value: u128, kind: IntegerKind) -> Arc<Expr<'a>> {
        let mut bb = Builder::new().create_integer().with_kind(kind);

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

    fn parse_float_literal(&mut self, value: f64) -> Arc<Expr<'a>> {
        Builder::new().create_float().with_value(value).build()
    }

    fn parse_string_literal(&mut self, content: StringData<'a>) -> Arc<Expr<'a>> {
        Builder::new().create_string().with_string(content).build()
    }

    fn parse_bstring_literal(&mut self, content: BStringData<'a>) -> Arc<Expr<'a>> {
        Builder::new().create_bstring().with_value(content).build()
    }

    fn parse_literal_suffix(&mut self, lit: Arc<Expr<'a>>) -> Arc<Expr<'a>> {
        let bb = Builder::new();
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

            Builder::new()
                .create_binexpr()
                .with_left(lit)
                .with_operator(BinExprOp::As)
                .with_right(Arc::new(type_name.deref().clone().into()))
                .build()
        } else {
            lit
        }
    }

    pub(crate) fn parse_attributes(&mut self) -> Option<Vec<Arc<Expr<'a>>>> {
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

    fn parse_type_or_type_alias(&mut self) -> Option<Arc<Expr<'a>>> {
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

            return Some(Arc::new(the_type.deref().clone().into()));
        }

        self.lexer.rewind(rewind_pos);

        self.parse_type_alias()
    }

    fn parse_if(&mut self) -> Option<Arc<Expr<'a>>> {
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
            Builder::new()
                .create_if()
                .with_condition(condition)
                .with_then_branch(then_branch)
                .with_else_branch(else_branch)
                .build(),
        )
    }

    fn parse_for(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement for expression parsing logic
        self.set_failed_bit();
        error!("For expression parsing not implemented yet");
        None
    }

    fn parse_while(&mut self) -> Option<Arc<Expr<'a>>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::While));
        self.lexer.skip_tok();

        let condition = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            Builder::new().create_boolean(true)
        } else {
            self.parse_expression()?
        };

        let body = self.parse_block()?;

        Some(
            Builder::new()
                .create_while_loop()
                .with_condition(condition)
                .with_body(body)
                .build(),
        )
    }

    fn parse_do(&mut self) -> Option<Arc<Expr<'a>>> {
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
            Builder::new()
                .create_do_while_loop()
                .with_body(body)
                .with_condition(condition)
                .build(),
        )
    }

    fn parse_switch(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement switch expression parsing logic
        self.set_failed_bit();
        error!("Switch expression parsing not implemented yet");
        None
    }

    fn parse_break(&mut self) -> Option<Arc<Expr<'a>>> {
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

        Some(
            Builder::new()
                .create_break()
                .with_label(branch_label)
                .build(),
        )
    }

    fn parse_continue(&mut self) -> Option<Arc<Expr<'a>>> {
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

        Some(
            Builder::new()
                .create_continue()
                .with_label(branch_label)
                .build(),
        )
    }

    fn parse_return(&mut self) -> Option<Arc<Expr<'a>>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Ret));
        self.lexer.skip_tok();

        let value = if self.lexer.next_is(&Token::Punct(Punct::Semicolon)) {
            None
        } else {
            Some(self.parse_expression()?)
        };

        Some(Builder::new().create_return().with_value(value).build())
    }

    fn parse_foreach(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement foreach expression parsing logic
        self.set_failed_bit();
        error!("Foreach expression parsing not implemented yet");
        None
    }

    fn parse_await(&mut self) -> Option<Arc<Expr<'a>>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Await));
        self.lexer.skip_tok();

        let Some(expr) = self.parse_expression() else {
            self.set_failed_bit();
            return None;
        };

        Some(Builder::new().create_await().with_expression(expr).build())
    }

    fn parse_asm(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement asm expression parsing logic
        self.set_failed_bit();
        error!("Asm expression parsing not implemented yet");
        None
    }

    fn parse_assert(&mut self) -> Option<Arc<Expr<'a>>> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Assert));
        self.lexer.skip_tok();

        let condition = self.parse_expression()?;

        Some(
            Builder::new()
                .create_assert()
                .with_condition(condition)
                .build(),
        )
    }

    fn parse_type_alias(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement type alias parsing logic
        self.set_failed_bit();
        error!("Type alias parsing not implemented yet");
        None
    }

    fn parse_scope(&mut self) -> Option<Arc<Expr<'a>>> {
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
        let new_scope = self.scope.clone();

        let Some(block) = self.parse_block() else {
            self.scope.pop();
            return None;
        };

        self.scope.pop();

        Some(
            Builder::new()
                .create_scope()
                .with_scope(new_scope)
                .add_attributes(attributes)
                .with_block(block)
                .build(),
        )
    }

    fn parse_enum(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement enum parsing logic
        self.set_failed_bit();
        error!("Enum parsing not implemented yet");
        None
    }

    fn parse_struct(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement struct parsing logic
        self.set_failed_bit();
        error!("Struct parsing not implemented yet");
        None
    }

    fn parse_class(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement class parsing logic
        self.set_failed_bit();
        error!("Class parsing not implemented yet");
        None
    }

    fn parse_trait(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement trait parsing logic
        self.set_failed_bit();
        error!("Trait parsing not implemented yet");
        None
    }

    fn parse_implementation(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement implementation parsing logic
        self.set_failed_bit();
        error!("Implementation parsing not implemented yet");
        None
    }

    fn parse_contract(&mut self) -> Option<Arc<Expr<'a>>> {
        // TODO: Implement contract parsing logic
        self.set_failed_bit();
        error!("Contract parsing not implemented yet");
        None
    }

    fn parse_function(&mut self) -> Option<Arc<Expr<'a>>> {
        if self.lexer.peek_t() == Token::Punct(Punct::LeftBrace) {
            let block = Some(self.parse_block()?);

            let infer_type = Builder::new().get_infer_type();

            return Some(
                Builder::new()
                    .create_function()
                    .with_definition(block)
                    .with_return_type(infer_type)
                    .build(),
            );
        }

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let name_pos = self.lexer.peek_tok().start();
        let function_name = self
            .lexer
            .next_if_name()
            .map(|t| t.name())
            .unwrap_or_default();

        let parameters = self.parse_function_parameters()?;

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            self.parse_type()?
        } else {
            Builder::new().get_infer_type()
        };

        let body = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            Some(self.parse_block()?)
        } else {
            None
        };

        let function = Builder::new()
            .create_function()
            .with_attributes(attributes)
            .with_name(function_name)
            .with_parameters(parameters)
            .with_return_type(return_type)
            .with_definition(body)
            .build();

        if !function_name.is_empty() {
            let current_scope = self.scope.clone();
            if !self
                .symtab
                .insert(current_scope, function_name, function.clone())
            {
                self.set_failed_bit();
                error!(
                    "[P????]: function: duplicate function '{}'\n--> {}",
                    function_name, name_pos
                );

                return None;
            }
        }

        Some(function)
    }

    fn parse_let_variable(&mut self) -> Option<Arc<Expr<'a>>> {
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
            Builder::new().get_infer_type()
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        let variable = Builder::new()
            .create_let()
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
            error!(
                "[P????]: let: duplicate variable '{}'\n--> {}",
                variable_name, name_pos
            );

            return None;
        }

        Some(variable)
    }

    fn parse_var_variable(&mut self) -> Option<Arc<Expr<'a>>> {
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
            Builder::new().get_infer_type()
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression()?)
        } else {
            None
        };

        let variable = Builder::new()
            .create_var()
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
            error!(
                "[P????]: var: duplicate variable '{}'\n--> {}",
                variable_name, name_pos
            );

            return None;
        }

        Some(variable)
    }

    fn parse_expression_primary(&mut self) -> Option<Arc<Expr<'a>>> {
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

            Token::Name(name) => {
                self.lexer.skip_tok();
                Some(Builder::new().create_identifier(name.name()))
            }

            Token::Keyword(Keyword::True) => {
                self.lexer.skip_tok();
                Some(Builder::new().create_boolean(true))
            }

            Token::Keyword(Keyword::False) => {
                self.lexer.skip_tok();
                Some(Builder::new().create_boolean(false))
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

    pub fn parse_expression(&mut self) -> Option<Arc<Expr<'a>>> {
        let mut parenthesis_count = 0;
        while self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            parenthesis_count += 1;
        }

        let Some(expr) = self.parse_expression_primary() else {
            self.set_failed_bit();
            return None;
        };

        // FIXME: Handle parenthesis correctly
        // expr.add_parentheses();

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

        // TODO: Binary expression parsing logic
        // TODO: Unary prefix expression parsing logic
        // TODO: Unary postfix expression parsing logic

        Some(expr)
    }

    pub(crate) fn parse_block(&mut self) -> Option<Arc<Expr<'a>>> {
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
                expression = Builder::new()
                    .create_statement()
                    .with_expression(expression)
                    .build();
            }

            elements.push(expression);
        }

        Some(
            Builder::new()
                .create_block()
                .add_expressions(elements)
                .build(),
        )
    }
}
