use crate::bugs::SyntaxBug;

use super::parse::Parser;
use interned_string::IString;
use log::error;
use nitrate_parsetree::kind::{
    AnonymousFunction, Await, BinExpr, BinExprOp, Block, Break, Call, CallArguments, Cast,
    Continue, DoWhileLoop, Expr, ForEach, FunctionParameter, GenericArgument, If, IndexAccess,
    Integer, List, Path, Return, Type, UnaryExpr, UnaryExprOp, WhileLoop,
};
use nitrate_tokenize::{Keyword, Token};
use smallvec::{SmallVec, smallvec};

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
    Operator(Token),
    FunctionCall,
    Index,
}

fn get_precedence_of_operator(operator: &Token) -> Option<(Associativity, Precedence)> {
    // TODO: Cleanup

    let (associativity, precedence) = match operator {
        Token::Scope => (Associativity::LeftToRight, PrecedenceRank::Scope),

        Token::Dot | Token::Arrow => (Associativity::LeftToRight, PrecedenceRank::FieldAccess),

        Token::As => (Associativity::LeftToRight, PrecedenceRank::Cast),

        Token::Mul | Token::Div | Token::Mod => {
            (Associativity::LeftToRight, PrecedenceRank::MulDivMod)
        }

        Token::Add | Token::Sub => (Associativity::LeftToRight, PrecedenceRank::AddSub),

        Token::BitShl | Token::BitShr | Token::BitRol | Token::BitRor => (
            Associativity::LeftToRight,
            PrecedenceRank::BitShiftAndRotate,
        ),

        Token::BitAnd => (Associativity::LeftToRight, PrecedenceRank::BitAnd),
        Token::BitXor => (Associativity::LeftToRight, PrecedenceRank::BitXor),
        Token::BitOr => (Associativity::LeftToRight, PrecedenceRank::BitOr),

        Token::LogicEq
        | Token::LogicNe
        | Token::LogicLt
        | Token::LogicGt
        | Token::LogicLe
        | Token::LogicGe => (Associativity::LeftToRight, PrecedenceRank::Comparison),

        Token::LogicAnd => (Associativity::LeftToRight, PrecedenceRank::LogicAnd),
        Token::LogicXor => (Associativity::LeftToRight, PrecedenceRank::LogicXor),
        Token::LogicOr => (Associativity::LeftToRight, PrecedenceRank::LogicOr),

        Token::Range => (Associativity::LeftToRight, PrecedenceRank::Range),

        Token::Set
        | Token::SetPlus
        | Token::SetMinus
        | Token::SetTimes
        | Token::SetSlash
        | Token::SetPercent
        | Token::SetBitAnd
        | Token::SetBitOr
        | Token::SetBitXor
        | Token::SetBitShl
        | Token::SetBitShr
        | Token::SetBitRotl
        | Token::SetBitRotr
        | Token::SetLogicAnd
        | Token::SetLogicOr
        | Token::SetLogicXor => (Associativity::RightToLeft, PrecedenceRank::Assign),

        Token::BitNot | Token::LogicNot | Token::Typeof | Token::Ellipsis | Token::BlockArrow => {
            return None;
        }

        _ => return None,
    };

    Some((associativity, precedence as Precedence))
}

fn get_precedence(operation: Operation) -> Option<(Associativity, Precedence)> {
    // TODO: Cleanup

    match operation {
        Operation::Operator(operator) => get_precedence_of_operator(&operator),

        Operation::FunctionCall | Operation::Index => Some((
            Associativity::LeftToRight,
            PrecedenceRank::FunctionCallAndIndexing as Precedence,
        )),
    }
}

fn get_prefix_precedence(op: &Token) -> Option<Precedence> {
    // TODO: Cleanup

    let precedence = match op {
        Token::Add => PrecedenceRank::Unary,
        Token::Sub => PrecedenceRank::Unary,
        Token::LogicNot => PrecedenceRank::Unary,
        Token::BitNot => PrecedenceRank::Unary,
        Token::Mul => PrecedenceRank::Unary,
        Token::BitAnd => PrecedenceRank::Unary,
        Token::Typeof => PrecedenceRank::Unary,

        _ => return None,
    };

    Some(precedence as Precedence)
}

impl Parser<'_, '_> {
    fn parse_expression_primary(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        match self.lexer.peek_t() {
            Token::Integer(int) => {
                self.lexer.skip_tok();
                let lit = Expr::Integer(Box::new(Integer {
                    value: int.value(),
                    kind: int.kind(),
                }));

                Some(self.parse_literal_suffix(lit))
            }

            Token::Float(float) => {
                self.lexer.skip_tok();
                let lit = Expr::Float(float);
                Some(self.parse_literal_suffix(lit))
            }

            Token::String(string) => {
                self.lexer.skip_tok();
                let lit = Expr::String(string);
                Some(self.parse_literal_suffix(lit))
            }

            Token::BString(data) => {
                self.lexer.skip_tok();
                let lit = Expr::BString(Box::new(data));
                Some(self.parse_literal_suffix(lit))
            }

            Token::OpenBracket => self.parse_list(),

            Token::Name(_) | Token::Scope => {
                let path = self.parse_path();
                Some(Expr::Path(Box::new(path)))
            }

            Token::Keyword(Keyword::True) => {
                self.lexer.skip_tok();
                Some(Expr::Boolean(true))
            }

            Token::Keyword(Keyword::False) => {
                self.lexer.skip_tok();
                Some(Expr::Boolean(false))
            }

            Token::Keyword(Keyword::Type) => self.parse_type_info(),

            Token::Keyword(Keyword::Fn) | Token::OpenBrace => self.parse_anonymous_function(),

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

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxBug::ExpectedExpr(self.lexer.peek_pos());
                self.bugs.push(&bug);

                None
            }
        }
    }

    fn parse_prefix(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        let prefix_op = self.lexer.peek_t();
        if UnaryExprOp::try_from(prefix_op.clone()).is_ok() {
            if prefix_op != Token::Scope {
                if let Some(precedence) = get_prefix_precedence(&prefix_op) {
                    self.lexer.skip_tok();

                    let operand = self.parse_expression_precedence(precedence)?;

                    return Some(Expr::UnaryExpr(Box::new(UnaryExpr {
                        operator: UnaryExprOp::try_from(prefix_op).expect("invalid unary_op"),
                        operand,
                        is_postfix: false,
                    })));
                }
            }
        }

        if self.lexer.skip_if(&Token::OpenParen) {
            let inner = self.parse_expression();

            if !self.lexer.skip_if(&Token::CloseParen) {
                error!(
                    "[P????]: expr: expected closing parenthesis\n--> {}",
                    self.lexer.current_pos()
                );
                return None;
            }

            return Some(Expr::Parentheses(Box::new(inner)));
        }

        self.parse_expression_primary()
    }

    fn parse_expression_precedence(
        &mut self,
        min_precedence_to_proceed: Precedence,
    ) -> Option<Expr> {
        // TODO: Cleanup

        let mut sofar = self.parse_prefix()?;

        loop {
            match self.lexer.peek_t() {
                next_op if BinExprOp::try_from(next_op.clone()).is_ok() => {
                    let operation = Operation::Operator(next_op.clone());
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

                    sofar = Expr::BinExpr(Box::new(BinExpr {
                        left: sofar,
                        operator: BinExprOp::try_from(next_op).expect("invalid bin_op"),
                        right: right_expr,
                    }));
                }

                Token::OpenParen => {
                    let operation = Operation::FunctionCall;
                    let Some((_, new_precedence)) = get_precedence(operation) else {
                        return Some(sofar);
                    };

                    if new_precedence < min_precedence_to_proceed {
                        return Some(sofar);
                    }

                    let call_arguments = self.parse_function_arguments()?;

                    sofar = Expr::Call(Box::new(Call {
                        callee: sofar,
                        arguments: call_arguments,
                    }));
                }

                Token::OpenBracket => {
                    let operation = Operation::Index;
                    let Some((_, new_precedence)) = get_precedence(operation) else {
                        return Some(sofar);
                    };

                    if new_precedence < min_precedence_to_proceed {
                        return Some(sofar);
                    }

                    self.lexer.skip_tok();

                    let index = self.parse_expression();

                    if !self.lexer.skip_if(&Token::CloseBracket) {
                        error!(
                            "[P????]: expr: expected closing bracket\n--> {}",
                            self.lexer.current_pos()
                        );
                        return None;
                    }

                    sofar = Expr::IndexAccess(Box::new(IndexAccess {
                        collection: sofar,
                        index,
                    }));
                }

                _ => {
                    return Some(sofar);
                }
            }
        }
    }

    fn parse_literal_suffix(&mut self, lit: Expr) -> Expr {
        // TODO: Cleanup

        let type_name = match self.lexer.peek_t() {
            Token::Name(name) => Some(Type::TypeName(Box::new(Path {
                path: smallvec![name],
                type_arguments: Vec::new(),
            }))),

            Token::Keyword(Keyword::Bool) => Some(Type::Bool),
            Token::Keyword(Keyword::U8) => Some(Type::UInt8),
            Token::Keyword(Keyword::U16) => Some(Type::UInt16),
            Token::Keyword(Keyword::U32) => Some(Type::UInt32),
            Token::Keyword(Keyword::U64) => Some(Type::UInt64),
            Token::Keyword(Keyword::U128) => Some(Type::UInt128),
            Token::Keyword(Keyword::I8) => Some(Type::Int8),
            Token::Keyword(Keyword::I16) => Some(Type::Int16),
            Token::Keyword(Keyword::I32) => Some(Type::Int32),
            Token::Keyword(Keyword::I64) => Some(Type::Int64),
            Token::Keyword(Keyword::I128) => Some(Type::Int128),
            Token::Keyword(Keyword::F8) => Some(Type::Float8),
            Token::Keyword(Keyword::F16) => Some(Type::Float16),
            Token::Keyword(Keyword::F32) => Some(Type::Float32),
            Token::Keyword(Keyword::F64) => Some(Type::Float64),
            Token::Keyword(Keyword::F128) => Some(Type::Float128),

            _ => None,
        };

        if let Some(to) = type_name {
            self.lexer.skip_tok();

            Expr::Cast(Box::new(Cast { value: lit, to: to }))
        } else {
            lit
        }
    }

    fn parse_list(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::OpenBracket);
        self.lexer.skip_tok();

        let mut elements = Vec::new();
        self.lexer.skip_if(&Token::Comma);

        loop {
            if self.lexer.skip_if(&Token::CloseBracket) {
                break;
            }

            elements.push(self.parse_expression());

            if !self.lexer.skip_if(&Token::Comma) {
                if self.lexer.skip_if(&Token::CloseBracket) {
                    break;
                }
                error!(
                    "[P0???]: list: expected ',' or ']' after element expression\n--> {}",
                    self.lexer.current_pos()
                );

                return None;
            }
        }

        Some(Expr::List(Box::new(List { elements })))
    }

    pub(crate) fn parse_attributes(&mut self) -> Vec<Expr> {
        // TODO: Cleanup

        let mut attributes = Vec::new();

        if !self.lexer.skip_if(&Token::OpenBracket) {
            return attributes;
        }

        self.lexer.skip_if(&Token::Comma);
        loop {
            if self.lexer.skip_if(&Token::CloseBracket) {
                break;
            }

            let attrib = self.parse_expression();
            attributes.push(attrib);

            if !self.lexer.skip_if(&Token::Comma) {
                if self.lexer.skip_if(&Token::CloseBracket) {
                    break;
                }
                error!(
                    "[P0???]: expected ',' or ']' after attribute expression\n--> {}",
                    self.lexer.current_pos()
                );

                return attributes;
            }
        }

        attributes
    }

    fn parse_generic_arguments(&mut self) -> Vec<GenericArgument> {
        // TODO: Cleanup

        fn parse_generic_argument(this: &mut Parser) -> GenericArgument {
            // TODO: Cleanup

            let mut name: Option<IString> = None;

            let rewind_pos = this.lexer.current_pos();
            if let Some(argument_name) = this.lexer.next_if_name() {
                if this.lexer.skip_if(&Token::Colon) {
                    name = Some(argument_name);
                } else {
                    this.lexer.rewind(rewind_pos);
                }
            }

            let value = this.parse_type();

            GenericArgument { name, value }
        }

        assert!(self.lexer.peek_t() == Token::LogicLt);
        self.lexer.skip_tok();

        let mut arguments = Vec::new();
        let mut already_reported_too_many_arguments = false;

        self.lexer.skip_if(&Token::Comma);

        loop {
            let peek = self.lexer.peek_t();

            match peek {
                Token::LogicGt => {
                    self.lexer.skip_tok();
                    break;
                }

                Token::BitShr => {
                    self.lexer.modify_next_tok(Token::LogicGt);
                    break;
                }

                Token::BitRor => {
                    self.lexer.modify_next_tok(Token::BitShr);
                    break;
                }

                _ => {}
            }

            if self.lexer.is_eof() {
                let bug = SyntaxBug::ExpectedGenericArgumentEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_GENERIC_ARGUMENTS: usize = 65_536;

            if !already_reported_too_many_arguments && arguments.len() >= MAX_GENERIC_ARGUMENTS {
                already_reported_too_many_arguments = true;

                let bug = SyntaxBug::GenericArgumentLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let argument = parse_generic_argument(self);
            arguments.push(argument);

            if !self.lexer.skip_if(&Token::Comma) {
                let any_terminator = self.lexer.next_is(&Token::LogicGt)
                    || self.lexer.next_is(&Token::BitShr)
                    || self.lexer.next_is(&Token::BitRor);

                if !any_terminator {
                    let bug = SyntaxBug::ExpectedClosingAngle(self.lexer.peek_pos());
                    self.bugs.push(&bug);
                    break;
                }
            }
        }

        arguments
    }

    pub(crate) fn parse_path(&mut self) -> Path {
        // TODO: Cleanup

        assert!(matches!(self.lexer.peek_t(), Token::Name(_) | Token::Scope));

        // TODO: Cleanup

        let mut path = SmallVec::new();
        let mut last_was_scope = false;

        loop {
            match self.lexer.peek_t() {
                Token::Name(name) => {
                    if !last_was_scope && !path.is_empty() {
                        break;
                    }

                    self.lexer.skip_tok();

                    path.push(name);
                    last_was_scope = false;
                }

                Token::Scope => {
                    if last_was_scope {
                        error!(
                            "[P????]: path: unexpected '::'\n--> {}",
                            self.lexer.current_pos()
                        );

                        break;
                    }

                    if path.is_empty() {
                        path.push(IString::from(""));
                    }

                    self.lexer.skip_tok();
                    last_was_scope = true;
                }

                _ => break,
            }
        }

        if path.is_empty() {
            error!(
                "[P????]: path: expected at least one identifier in path\n--> {}",
                self.lexer.current_pos()
            );
        }

        if last_was_scope {
            error!(
                "[P????]: path: unexpected trailing '::'\n--> {}",
                self.lexer.current_pos()
            );
        }

        if !self.lexer.next_is(&Token::LogicLt) {
            return Path {
                path,
                type_arguments: Vec::new(),
            };
        }

        let type_arguments = self.parse_generic_arguments();

        Path {
            path,
            type_arguments,
        }
    }

    fn parse_type_info(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Type));
        self.lexer.skip_tok();

        let of = self.parse_type();

        Some(Expr::TypeInfo(Box::new(of)))
    }

    fn parse_if(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::If));
        self.lexer.skip_tok();

        let condition = self.parse_expression();
        let then_branch = self.parse_block();

        let else_branch = if self.lexer.skip_if(&Token::Keyword(Keyword::Else)) {
            if self.lexer.next_is(&Token::Keyword(Keyword::If)) {
                let Some(else_if_branch) = self.parse_if() else {
                    error!(
                        "[P????]: if: expected else block after 'else if'\n--> {}",
                        self.lexer.current_pos()
                    );
                    return None;
                };

                Some(Block {
                    elements: vec![else_if_branch],
                    ends_with_semi: false,
                })
            } else {
                Some(self.parse_block())
            }
        } else {
            None
        };

        Some(Expr::If(Box::new(If {
            condition,
            then_branch,
            else_branch,
        })))
    }

    fn parse_for_bindings(&mut self) -> Option<Vec<(IString, Option<Type>)>> {
        // TODO: Cleanup

        let mut bindings = Vec::new();

        if self.lexer.skip_if(&Token::OpenParen) {
            self.lexer.skip_if(&Token::Comma);

            loop {
                if self.lexer.skip_if(&Token::CloseParen) {
                    break;
                }

                let Some(variable_name) = self.lexer.next_if_name() else {
                    error!(
                        "[P????]: for: expected loop variable name\n--> {}",
                        self.lexer.current_pos()
                    );
                    return None;
                };

                let type_annotation = if self.lexer.skip_if(&Token::Colon) {
                    Some(self.parse_type())
                } else {
                    None
                };

                bindings.push((variable_name, type_annotation));

                if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                    error!(
                        "[P0???]: for: expected ',' or ')' after loop variable\n--> {}",
                        self.lexer.current_pos()
                    );

                    return None;
                }
            }
        } else {
            let Some(variable_name) = self.lexer.next_if_name() else {
                error!(
                    "[P????]: for: expected loop variable name\n--> {}",
                    self.lexer.current_pos()
                );
                return None;
            };

            let type_annotation = if self.lexer.skip_if(&Token::Colon) {
                Some(self.parse_type())
            } else {
                None
            };

            bindings.push((variable_name, type_annotation));
        }

        Some(bindings)
    }

    fn parse_for(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::For));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let bindings = self.parse_for_bindings()?;

        if !self.lexer.skip_if(&Token::Keyword(Keyword::In)) {
            error!(
                "[P????]: for: expected 'in' after loop variable\n--> {}",
                self.lexer.current_pos()
            );
            return None;
        }

        let iterable = self.parse_expression();
        let body = self.parse_block();

        Some(Expr::ForEach(Box::new(ForEach {
            attributes,
            bindings,
            iterable,
            body,
        })))
    }

    fn parse_while(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::While));
        self.lexer.skip_tok();

        let condition = if self.lexer.next_is(&Token::OpenBrace) {
            Expr::Boolean(true)
        } else {
            self.parse_expression()
        };

        let body = self.parse_block();

        Some(Expr::WhileLoop(Box::new(WhileLoop { condition, body })))
    }

    fn parse_do(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Do));
        self.lexer.skip_tok();

        let body = self.parse_block();
        if !self.lexer.skip_if(&Token::Keyword(Keyword::While)) {
            error!(
                "[P????]: do-while: expected 'while' after 'do' block\n--> {}",
                self.lexer.current_pos()
            );

            return None;
        }

        let condition = self.parse_expression();

        Some(Expr::DoWhileLoop(Box::new(DoWhileLoop { body, condition })))
    }

    fn parse_switch(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        // TODO: switch expression parsing logic
        error!("Switch expression parsing not implemented yet");
        None
    }

    fn parse_break(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Break));
        self.lexer.skip_tok();

        let label = if self.lexer.skip_if(&Token::SingleQuote) {
            let Some(name) = self.lexer.next_if_name() else {
                error!(
                    "[P????]: break: expected branch label after single quote\n--> {}",
                    self.lexer.current_pos()
                );

                return None;
            };

            Some(name)
        } else {
            None
        };

        Some(Expr::Break(Box::new(Break { label })))
    }

    fn parse_continue(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Continue));
        self.lexer.skip_tok();

        let label = if self.lexer.skip_if(&Token::SingleQuote) {
            let Some(name) = self.lexer.next_if_name() else {
                error!(
                    "[P????]: continue: expected branch label after single quote\n--> {}",
                    self.lexer.current_pos()
                );

                return None;
            };

            Some(name)
        } else {
            None
        };

        Some(Expr::Continue(Box::new(Continue { label })))
    }

    fn parse_return(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Ret));
        self.lexer.skip_tok();

        let value = if self.lexer.next_is(&Token::Semi) {
            None
        } else {
            Some(self.parse_expression())
        };

        Some(Expr::Return(Box::new(Return { value })))
    }

    fn parse_await(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Await));
        self.lexer.skip_tok();

        let future = self.parse_expression();

        Some(Expr::Await(Box::new(Await { future })))
    }

    fn parse_asm(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        // TODO: asm expression parsing logic
        error!("Asm expression parsing not implemented yet");
        None
    }

    fn parse_anonymous_function_parameter(&mut self) -> FunctionParameter {
        // TODO: Cleanup

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::FunctionParameterMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let param_type = if self.lexer.skip_if(&Token::Colon) {
            Some(self.parse_type())
        } else {
            None
        };

        let default = if self.lexer.skip_if(&Token::Set) {
            Some(self.parse_expression())
        } else {
            None
        };

        FunctionParameter {
            name,
            param_type,
            default,
            attributes,
        }
    }

    fn parse_anonymous_function_parameters(&mut self) -> Vec<FunctionParameter> {
        // TODO: Cleanup

        let mut params = Vec::new();

        if !self.lexer.skip_if(&Token::OpenParen) {
            let bug = SyntaxBug::ExpectedOpeningParen(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        self.lexer.skip_if(&Token::Comma);

        let mut already_reported_too_many_parameters = false;

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_FUNCTION_PARAMETERS: usize = 65_536;

            if !already_reported_too_many_parameters && params.len() >= MAX_FUNCTION_PARAMETERS {
                already_reported_too_many_parameters = true;

                let bug = SyntaxBug::FunctionParameterLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let param = self.parse_anonymous_function_parameter();
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        params
    }

    fn parse_anonymous_function(&mut self) -> Option<Expr> {
        // TODO: Cleanup

        if self.lexer.peek_t() == Token::OpenBrace {
            let definition = self.parse_block();

            return Some(Expr::Function(Box::new(AnonymousFunction {
                attributes: Vec::new(),
                parameters: Vec::new(),
                return_type: None,
                definition,
            })));
        }

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let parameters = self.parse_anonymous_function_parameters();

        let return_type = if self.lexer.skip_if(&Token::Arrow) {
            Some(self.parse_type())
        } else {
            None
        };

        let definition = if self.lexer.next_is(&Token::OpenBrace) {
            self.parse_block()
        } else if self.lexer.skip_if(&Token::BlockArrow) {
            let expr = self.parse_expression();
            Block {
                elements: vec![expr],
                ends_with_semi: false,
            }
        } else {
            error!(
                "[P????]: function: expected function body\n--> {}",
                self.lexer.current_pos()
            );
            return None;
        };

        let function = Expr::Function(Box::new(AnonymousFunction {
            attributes,
            parameters,
            return_type,
            definition,
        }));

        Some(function)
    }

    fn parse_function_argument(&mut self) -> Option<(Option<IString>, Expr)> {
        // TODO: Cleanup

        let mut argument_name = None;

        if let Token::Name(name) = self.lexer.peek_t() {
            /* Named function argument syntax is ambiguous,
             * an identifier can be followed by a colon
             * to indicate a named argument (followed by the expression value).
             * However, if it is not followed by a colon, the identifier is
             * to be parsed as an expression.
             */
            let rewind_pos = self.lexer.current_pos();
            self.lexer.skip_tok();

            if self.lexer.skip_if(&Token::Colon) {
                argument_name = Some(name);
            } else {
                self.lexer.rewind(rewind_pos);
            }
        }

        let argument_value = self.parse_expression();

        Some((argument_name, argument_value))
    }

    fn parse_function_arguments(&mut self) -> Option<CallArguments> {
        // TODO: Cleanup

        assert!(self.lexer.peek_t() == Token::OpenParen);
        self.lexer.skip_tok();

        let mut arguments = CallArguments::new();
        self.lexer.skip_if(&Token::Comma);

        loop {
            if self.lexer.skip_if(&Token::CloseParen) {
                break;
            }

            let function_argument = self.parse_function_argument()?;
            arguments.push(function_argument);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                error!(
                    "[P0???]: function call: expected ',' or ')' after function argument\n--> {}",
                    self.lexer.current_pos()
                );

                return None;
            }
        }

        Some(arguments)
    }

    pub(crate) fn parse_block(&mut self) -> Block {
        // TODO: Cleanup

        if !self.lexer.skip_if(&Token::OpenBrace) {
            error!(
                "[P????]: expr: block: expected opening brace\n--> {}",
                self.lexer.current_pos()
            );

            return Block {
                elements: Vec::new(),
                ends_with_semi: false,
            };
        }

        let mut elements = Vec::new();
        let mut ends_with_semi = false;

        loop {
            if self.lexer.skip_if(&Token::CloseBrace) {
                break;
            }

            if self.lexer.next_if_comment().is_some() {
                continue;
            }

            let expression = self.parse_expression();
            elements.push(expression);

            ends_with_semi = self.lexer.skip_if(&Token::Semi);
        }

        Block {
            elements,
            ends_with_semi,
        }
    }

    pub(crate) fn parse_expression(&mut self) -> Expr {
        // TODO: Cleanup

        self.parse_expression_precedence(Precedence::MIN)
            .unwrap_or(Expr::SyntaxError)
    }
}
