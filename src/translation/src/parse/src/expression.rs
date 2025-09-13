use super::parse::Parser;
use interned_string::IString;
use log::error;
use nitrate_parsetree::{
    Builder,
    kind::{BinExprOp, CallArguments, Expr, UnaryExprOp},
};
use nitrate_tokenize::{Keyword, Op, Punct, Token};

type Precedence = u32;

#[repr(u32)]
enum PrecedenceRank {
    Assign,
    Range,
    LogicOr,
    LogicXor,
    LogicAnd,
    Comparison,
    BitOr,
    BitXor,
    BitAnd,
    BitShiftAndRotate,
    AddSub,
    MulDivMod,
    Cast,
    Unary,
    FunctionCallAndIndexing,
    FieldAccess,
    Scope,
}

#[derive(PartialEq, PartialOrd, Eq, Clone, Copy)]
enum Associativity {
    LeftToRight,
    RightToLeft,
}

enum Operation {
    Operator(Op),
    FunctionCall,
    Index,
}

fn get_precedence_of_operator(operator: Op) -> Option<(Associativity, Precedence)> {
    let (associativity, precedence) = match operator {
        Op::Scope => (Associativity::LeftToRight, PrecedenceRank::Scope),

        Op::Dot | Op::Arrow => (Associativity::LeftToRight, PrecedenceRank::FieldAccess),

        Op::As | Op::BitcastAs => (Associativity::LeftToRight, PrecedenceRank::Cast),

        Op::Mul | Op::Div | Op::Mod => (Associativity::LeftToRight, PrecedenceRank::MulDivMod),

        Op::Add | Op::Sub => (Associativity::LeftToRight, PrecedenceRank::AddSub),

        Op::BitShl | Op::BitShr | Op::BitRol | Op::BitRor => (
            Associativity::LeftToRight,
            PrecedenceRank::BitShiftAndRotate,
        ),

        Op::BitAnd => (Associativity::LeftToRight, PrecedenceRank::BitAnd),
        Op::BitXor => (Associativity::LeftToRight, PrecedenceRank::BitXor),
        Op::BitOr => (Associativity::LeftToRight, PrecedenceRank::BitOr),

        Op::LogicEq | Op::LogicNe | Op::LogicLt | Op::LogicGt | Op::LogicLe | Op::LogicGe => {
            (Associativity::LeftToRight, PrecedenceRank::Comparison)
        }

        Op::LogicAnd => (Associativity::LeftToRight, PrecedenceRank::LogicAnd),
        Op::LogicXor => (Associativity::LeftToRight, PrecedenceRank::LogicXor),
        Op::LogicOr => (Associativity::LeftToRight, PrecedenceRank::LogicOr),

        Op::Range => (Associativity::LeftToRight, PrecedenceRank::Range),

        Op::Set
        | Op::SetPlus
        | Op::SetMinus
        | Op::SetTimes
        | Op::SetSlash
        | Op::SetPercent
        | Op::SetBitAnd
        | Op::SetBitOr
        | Op::SetBitXor
        | Op::SetBitShl
        | Op::SetBitShr
        | Op::SetBitRotl
        | Op::SetBitRotr
        | Op::SetLogicAnd
        | Op::SetLogicOr
        | Op::SetLogicXor => (Associativity::RightToLeft, PrecedenceRank::Assign),

        Op::BitNot | Op::LogicNot | Op::Typeof | Op::Ellipsis | Op::BlockArrow => {
            return None;
        }
    };

    Some((associativity, precedence as Precedence))
}

fn get_precedence(operation: Operation) -> Option<(Associativity, Precedence)> {
    match operation {
        Operation::Operator(operator) => get_precedence_of_operator(operator),

        Operation::FunctionCall | Operation::Index => Some((
            Associativity::LeftToRight,
            PrecedenceRank::FunctionCallAndIndexing as Precedence,
        )),
    }
}

fn get_prefix_precedence(op: Op) -> Option<Precedence> {
    let precedence = match op {
        Op::Add => PrecedenceRank::Unary,
        Op::Sub => PrecedenceRank::Unary,
        Op::LogicNot => PrecedenceRank::Unary,
        Op::BitNot => PrecedenceRank::Unary,
        Op::Mul => PrecedenceRank::Unary,
        Op::BitAnd => PrecedenceRank::Unary,
        Op::Typeof => PrecedenceRank::Unary,

        _ => return None,
    };

    Some(precedence as Precedence)
}

impl Parser<'_, '_> {
    fn parse_expression_primary(&mut self) -> Option<Expr> {
        match self.lexer.peek_t() {
            Token::Integer(int) => {
                self.lexer.skip_tok();
                let lit = Builder::create_integer(int.value(), int.kind());
                Some(self.parse_literal_suffix(lit))
            }

            Token::Float(float) => {
                self.lexer.skip_tok();
                let lit = Builder::create_float(float);
                Some(self.parse_literal_suffix(lit))
            }

            Token::String(string) => {
                self.lexer.skip_tok();
                let lit = Builder::create_string(string);
                Some(self.parse_literal_suffix(lit))
            }

            Token::BString(data) => {
                self.lexer.skip_tok();
                let lit = Builder::create_bstring(data);
                Some(self.parse_literal_suffix(lit))
            }

            Token::Punct(Punct::LeftBracket) => self.parse_list(),

            Token::Name(name) => {
                self.lexer.skip_tok();
                Some(Builder::create_identifier([name].to_vec()))
            }

            Token::Keyword(Keyword::True) => {
                self.lexer.skip_tok();
                Some(Builder::create_boolean(true))
            }

            Token::Keyword(Keyword::False) => {
                self.lexer.skip_tok();
                Some(Builder::create_boolean(false))
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
            Token::Keyword(Keyword::Fn) | Token::Punct(Punct::LeftBrace) => self.parse_function(),

            Token::Keyword(Keyword::If) => self.parse_if(),
            Token::Keyword(Keyword::For) => self.parse_for(),
            Token::Keyword(Keyword::While) => self.parse_while(),
            Token::Keyword(Keyword::Do) => self.parse_do(),
            Token::Keyword(Keyword::Switch) => self.parse_switch(),
            Token::Keyword(Keyword::Break) => self.parse_break(),
            Token::Keyword(Keyword::Continue) => self.parse_continue(),
            Token::Keyword(Keyword::Ret) => self.parse_return(),
            Token::Keyword(Keyword::Await) => self.parse_await(),
            Token::Keyword(Keyword::Asm) => self.parse_asm(),

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

    fn parse_prefix(&mut self) -> Option<Expr> {
        if let Token::Op(prefix_op) = self.lexer.peek_t() {
            if let Some(precedence) = get_prefix_precedence(prefix_op) {
                self.lexer.skip_tok();

                let operand = self.parse_expression_precedence(precedence)?;

                return Some(
                    Builder::create_unary_expr()
                        .with_prefix()
                        .with_operator(UnaryExprOp::try_from(prefix_op).expect("invalid unary_op"))
                        .with_operand(operand)
                        .build(),
                );
            }
        }

        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            let inner = self.parse_expression()?;

            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    "[P????]: expr: expected closing parenthesis\n--> {}",
                    self.lexer.sync_position()
                );
                return None;
            }

            return Some(Builder::create_parentheses(inner));
        }

        self.parse_expression_primary()
    }

    fn parse_expression_precedence(
        &mut self,
        min_precedence_to_proceed: Precedence,
    ) -> Option<Expr> {
        let mut sofar = self.parse_prefix()?;

        loop {
            match self.lexer.peek_t() {
                Token::Op(next_op) => {
                    let operation = Operation::Operator(next_op);
                    let Some((assoc, op_precedence)) = get_precedence(operation) else {
                        return Some(sofar);
                    };

                    if op_precedence < min_precedence_to_proceed {
                        return Some(sofar);
                    }

                    self.lexer.skip_tok();

                    let right_expr = if assoc == Associativity::LeftToRight {
                        self.parse_expression_precedence(op_precedence + 1)?
                    } else {
                        self.parse_expression_precedence(op_precedence)?
                    };

                    sofar = Builder::create_binexpr()
                        .with_left(sofar)
                        .with_operator(BinExprOp::try_from(next_op).expect("invalid bin_op"))
                        .with_right(right_expr)
                        .build();
                }

                Token::Punct(Punct::LeftParen) => {
                    let operation = Operation::FunctionCall;
                    let Some((_, new_precedence)) = get_precedence(operation) else {
                        return Some(sofar);
                    };

                    if new_precedence < min_precedence_to_proceed {
                        return Some(sofar);
                    }

                    let call_arguments = self.parse_function_arguments()?;

                    sofar = Builder::create_call()
                        .with_callee(sofar)
                        .add_arguments(call_arguments)
                        .build();
                }

                Token::Punct(Punct::LeftBracket) => {
                    let operation = Operation::Index;
                    let Some((_, new_precedence)) = get_precedence(operation) else {
                        return Some(sofar);
                    };

                    if new_precedence < min_precedence_to_proceed {
                        return Some(sofar);
                    }

                    self.lexer.skip_tok();

                    let index = self.parse_expression()?;

                    if !self.lexer.skip_if(&Token::Punct(Punct::RightBracket)) {
                        self.set_failed_bit();
                        error!(
                            "[P????]: expr: expected closing bracket\n--> {}",
                            self.lexer.sync_position()
                        );
                        return None;
                    }

                    sofar = Builder::create_index_access()
                        .with_collection(sofar)
                        .with_index(index)
                        .build();
                }

                _ => {
                    return Some(sofar);
                }
            }
        }
    }

    pub fn parse_expression(&mut self) -> Option<Expr> {
        self.parse_expression_precedence(Precedence::MIN)
    }

    fn parse_literal_suffix(&mut self, lit: Expr) -> Expr {
        let type_name = match self.lexer.peek_t() {
            Token::Name(name) => Some(Builder::create_type_name([name].to_vec())),

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

        if let Some(_type_name) = type_name {
            self.lexer.skip_tok();

            // TODO: Create a proper 'as' expression node
            todo!();
            // Builder::create_binexpr()
            //     .with_left(lit)
            //     .with_operator(BinExprOp::As)
            //     .with_right(type_name.into())
            //     .build()
        } else {
            lit
        }
    }

    pub(crate) fn parse_list(&mut self) -> Option<Expr> {
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

    pub(crate) fn parse_attributes(&mut self) -> Option<Vec<Expr>> {
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

    fn parse_type_or_type_alias(&mut self) -> Option<Expr> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Type));

        let rewind_pos = self.lexer.sync_position();
        self.lexer.skip_tok();

        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            let Some(for_type) = self.parse_type() else {
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

            return Some(Builder::create_type_info(for_type));
        }

        self.lexer.rewind(rewind_pos);

        self.parse_type_alias()
    }

    fn parse_if(&mut self) -> Option<Expr> {
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

    fn parse_for(&mut self) -> Option<Expr> {
        // TODO: for expression parsing logic
        self.set_failed_bit();
        error!("For expression parsing not implemented yet");
        None
    }

    fn parse_while(&mut self) -> Option<Expr> {
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

    fn parse_do(&mut self) -> Option<Expr> {
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

    fn parse_switch(&mut self) -> Option<Expr> {
        // TODO: switch expression parsing logic
        self.set_failed_bit();
        error!("Switch expression parsing not implemented yet");
        None
    }

    fn parse_break(&mut self) -> Option<Expr> {
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

            Some(label)
        } else {
            None
        };

        Some(Builder::create_break().with_label(branch_label).build())
    }

    fn parse_continue(&mut self) -> Option<Expr> {
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

            Some(label)
        } else {
            None
        };

        Some(Builder::create_continue().with_label(branch_label).build())
    }

    fn parse_return(&mut self) -> Option<Expr> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Ret));
        self.lexer.skip_tok();

        let value = if self.lexer.next_is(&Token::Punct(Punct::Semicolon)) {
            None
        } else {
            Some(self.parse_expression()?)
        };

        Some(Builder::create_return().with_value(value).build())
    }

    fn parse_await(&mut self) -> Option<Expr> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Await));
        self.lexer.skip_tok();

        let Some(expr) = self.parse_expression() else {
            self.set_failed_bit();
            return None;
        };

        Some(Builder::create_await().with_future(expr).build())
    }

    fn parse_asm(&mut self) -> Option<Expr> {
        // TODO: asm expression parsing logic
        self.set_failed_bit();
        error!("Asm expression parsing not implemented yet");
        None
    }

    fn parse_type_alias(&mut self) -> Option<Expr> {
        // TODO: type alias parsing logic
        self.set_failed_bit();
        error!("Type alias parsing not implemented yet");
        None
    }

    fn parse_enum(&mut self) -> Option<Expr> {
        // TODO: enum parsing logic
        self.set_failed_bit();
        error!("Enum parsing not implemented yet");
        None
    }

    fn parse_struct(&mut self) -> Option<Expr> {
        // TODO: struct parsing logic
        self.set_failed_bit();
        error!("Struct parsing not implemented yet");
        None
    }

    fn parse_class(&mut self) -> Option<Expr> {
        // TODO: class parsing logic
        self.set_failed_bit();
        error!("Class parsing not implemented yet");
        None
    }

    fn parse_trait(&mut self) -> Option<Expr> {
        // TODO: trait parsing logic
        self.set_failed_bit();
        error!("Trait parsing not implemented yet");
        None
    }

    fn parse_implementation(&mut self) -> Option<Expr> {
        // TODO: implementation parsing logic
        self.set_failed_bit();
        error!("Implementation parsing not implemented yet");
        None
    }

    fn parse_contract(&mut self) -> Option<Expr> {
        // TODO: contract parsing logic
        self.set_failed_bit();
        error!("Contract parsing not implemented yet");
        None
    }

    fn parse_function(&mut self) -> Option<Expr> {
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
        } else if self.lexer.skip_if(&Token::Op(Op::BlockArrow)) {
            Some(self.parse_expression()?)
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

    fn parse_let_variable(&mut self) -> Option<Expr> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Let));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let name_pos = self.lexer.peek_tok().start();
        let variable_name = if let Some(name_token) = self.lexer.next_if_name() {
            name_token
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
            .with_name(variable_name.clone())
            .with_type(type_annotation)
            .with_initializer(initializer)
            .build();

        let current_scope = self.scope.clone();
        if !self
            .symtab
            .insert(current_scope, variable_name.clone(), variable.clone())
        {
            self.set_failed_bit();
            error!("[P????]: let: duplicate variable '{variable_name}'\n--> {name_pos}");
            // Fallthrough
        }

        Some(variable)
    }

    fn parse_var_variable(&mut self) -> Option<Expr> {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Var));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes()?;

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let name_pos = self.lexer.peek_tok().start();
        let variable_name = if let Some(name_token) = self.lexer.next_if_name() {
            name_token
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
            .with_name(variable_name.clone())
            .with_type(type_annotation)
            .with_initializer(initializer)
            .build();

        let current_scope = self.scope.clone();
        if !self
            .symtab
            .insert(current_scope, variable_name.clone(), variable.clone())
        {
            self.set_failed_bit();
            error!("[P????]: var: duplicate variable '{variable_name}'\n--> {name_pos}");
            // Fallthrough
        }

        Some(variable)
    }

    fn parse_function_argument(&mut self) -> Option<(Option<IString>, Expr)> {
        let mut argument_name = None;

        if let Token::Name(name) = self.lexer.peek_t() {
            /* Named function argument syntax is ambiguous,
             * an identifier can be followed by a colon
             * to indicate a named argument (followed by the expression value).
             * However, if it is not followed by a colon, the identifier is
             * to be parsed as an expression.
             */
            let rewind_pos = self.lexer.sync_position();
            self.lexer.skip_tok();

            if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
                argument_name = Some(name);
            } else {
                self.lexer.rewind(rewind_pos);
            }
        }

        let argument_value = self.parse_expression()?;

        Some((argument_name, argument_value))
    }

    fn parse_function_arguments(&mut self) -> Option<CallArguments> {
        assert!(self.lexer.peek_t() == Token::Punct(Punct::LeftParen));
        self.lexer.skip_tok();

        let mut arguments = CallArguments::new();
        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        loop {
            if self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                break;
            }

            let function_argument = self.parse_function_argument()?;
            arguments.push(function_argument);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Punct(Punct::RightParen))
            {
                error!(
                    "[P0???]: function call: expected ',' or ')' after function argument\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            }
        }

        Some(arguments)
    }

    pub(crate) fn parse_block_as_elements(&mut self) -> Option<Vec<Expr>> {
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

            if self.lexer.skip_if(&Token::Punct(Punct::Semicolon))
                || self.lexer.next_if_comment().is_some()
            {
                continue;
            }

            let Some(expression) = self.parse_expression() else {
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

            // TODO: Handle the semicolon
            self.lexer.skip_if(&Token::Punct(Punct::Semicolon));

            // if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            //     expression = Builder::create_statement()
            //         .with_expression(expression)
            //         .build();
            // }

            elements.push(expression);
        }

        Some(elements)
    }

    pub(crate) fn parse_block(&mut self) -> Option<Expr> {
        Some(
            Builder::create_block()
                .add_expressions(self.parse_block_as_elements()?)
                .build(),
        )
    }
}
