use crate::diagnosis::SyntaxErr;

use super::parse::Parser;
use nitrate_token::Token;
use nitrate_tree::{
    ast::{
        AttributeList, Await, BStringLit, BinExpr, BinExprOp, Block, BlockItem, Bool, BooleanLit,
        Break, Cast, Closure, Continue, ElseIf, Expr, ExprParentheses, ExprPath, ExprPathSegment,
        ExprSyntaxError, FieldAccess, Float32, Float64, FloatLit, ForEach, FuncParam, FunctionCall,
        If, IndexAccess, Int8, Int16, Int32, Int64, Int128, IntegerLit, List, LocalVariable,
        LocalVariableKind, MethodCall, Mutability, Return, Safety, StringLit, Tuple, Type,
        TypeArgument, TypeInfo, TypePath, TypePathSegment, UInt8, UInt16, UInt32, UInt64, UInt128,
        UnaryExpr, UnaryExprOp, WhileLoop,
    },
    tag::{
        ArgNameId, VariableNameId, intern_arg_name, intern_label_name, intern_parameter_name,
        intern_string_literal, intern_variable_name,
    },
};

type Precedence = u32;

#[repr(u32)]
enum PrecedenceRank {
    Assign,
    Range,
    LogicOr,
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
}

#[derive(PartialEq, PartialOrd, Eq, Clone, Copy)]
enum Associativity {
    LeftToRight,
    RightToLeft,
}

enum Operation {
    BinOp(BinExprOp),
    FunctionCall,
    Index,
    Cast,
    FieldAccessOrMethodCall,
}

fn get_precedence_of_binary_operator(op: BinExprOp) -> (Associativity, Precedence) {
    let (associativity, precedence) = match op {
        BinExprOp::Mul | BinExprOp::Div | BinExprOp::Mod => {
            (Associativity::LeftToRight, PrecedenceRank::MulDivMod)
        }

        BinExprOp::Add | BinExprOp::Sub => (Associativity::LeftToRight, PrecedenceRank::AddSub),

        BinExprOp::BitShl | BinExprOp::BitShr | BinExprOp::BitRol | BinExprOp::BitRor => (
            Associativity::LeftToRight,
            PrecedenceRank::BitShiftAndRotate,
        ),

        BinExprOp::BitAnd => (Associativity::LeftToRight, PrecedenceRank::BitAnd),
        BinExprOp::BitXor => (Associativity::LeftToRight, PrecedenceRank::BitXor),
        BinExprOp::BitOr => (Associativity::LeftToRight, PrecedenceRank::BitOr),

        BinExprOp::LogicEq
        | BinExprOp::LogicNe
        | BinExprOp::LogicLt
        | BinExprOp::LogicGt
        | BinExprOp::LogicLe
        | BinExprOp::LogicGe => (Associativity::LeftToRight, PrecedenceRank::Comparison),

        BinExprOp::LogicAnd => (Associativity::LeftToRight, PrecedenceRank::LogicAnd),
        BinExprOp::LogicOr => (Associativity::LeftToRight, PrecedenceRank::LogicOr),

        BinExprOp::Range => (Associativity::LeftToRight, PrecedenceRank::Range),

        BinExprOp::Set
        | BinExprOp::SetPlus
        | BinExprOp::SetMinus
        | BinExprOp::SetTimes
        | BinExprOp::SetSlash
        | BinExprOp::SetPercent
        | BinExprOp::SetBitAnd
        | BinExprOp::SetBitOr
        | BinExprOp::SetBitXor
        | BinExprOp::SetBitShl
        | BinExprOp::SetBitShr
        | BinExprOp::SetBitRotl
        | BinExprOp::SetBitRotr
        | BinExprOp::SetLogicAnd
        | BinExprOp::SetLogicOr => (Associativity::RightToLeft, PrecedenceRank::Assign),
    };

    (associativity, precedence as Precedence)
}

fn get_precedence(operation: Operation) -> (Associativity, Precedence) {
    match operation {
        Operation::BinOp(op) => get_precedence_of_binary_operator(op),

        Operation::FunctionCall | Operation::Index => (
            Associativity::LeftToRight,
            PrecedenceRank::FunctionCallAndIndexing as Precedence,
        ),

        Operation::Cast => (
            Associativity::LeftToRight,
            PrecedenceRank::Cast as Precedence,
        ),

        Operation::FieldAccessOrMethodCall => (
            Associativity::LeftToRight,
            PrecedenceRank::FieldAccess as Precedence,
        ),
    }
}

impl Parser<'_, '_> {
    fn detect_and_parse_unary_operator(&mut self) -> Option<UnaryExprOp> {
        match self.lexer.peek_t() {
            Token::Plus => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::Add)
            }

            Token::Minus => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::Sub)
            }

            Token::Star => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::Deref)
            }

            Token::And => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::Borrow)
            }

            Token::Tilde => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::BitNot)
            }

            Token::Bang => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::LogicNot)
            }

            Token::Typeof => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::Typeof)
            }

            _ => None,
        }
    }

    fn detect_and_parse_binary_operator(&mut self) -> Option<BinExprOp> {
        let rewind = self.lexer.current_pos();

        let result = match self.lexer.peek_t() {
            Token::Bang => {
                self.lexer.skip_tok();
                Some(BinExprOp::LogicNe)
            }

            Token::Percent => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetPercent)
                } else {
                    Some(BinExprOp::Mod)
                }
            }

            Token::And => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::And) {
                    if self.lexer.skip_if(&Token::Eq) {
                        Some(BinExprOp::SetLogicAnd)
                    } else {
                        Some(BinExprOp::LogicAnd)
                    }
                } else if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetBitAnd)
                } else {
                    Some(BinExprOp::BitAnd)
                }
            }

            Token::Star => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetTimes)
                } else {
                    Some(BinExprOp::Mul)
                }
            }

            Token::Plus => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetPlus)
                } else {
                    Some(BinExprOp::Add)
                }
            }

            Token::Minus => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetMinus)
                } else {
                    Some(BinExprOp::Sub)
                }
            }

            Token::Dot => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Dot) {
                    Some(BinExprOp::Range)
                } else {
                    None
                }
            }

            Token::Slash => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetSlash)
                } else {
                    Some(BinExprOp::Div)
                }
            }

            Token::Lt => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Lt) {
                    if self.lexer.skip_if(&Token::Lt) {
                        if self.lexer.skip_if(&Token::Eq) {
                            Some(BinExprOp::SetBitRotl)
                        } else {
                            Some(BinExprOp::BitRol)
                        }
                    } else if self.lexer.skip_if(&Token::Eq) {
                        Some(BinExprOp::SetBitShl)
                    } else {
                        Some(BinExprOp::BitShl)
                    }
                } else if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::LogicLe)
                } else {
                    Some(BinExprOp::LogicLt)
                }
            }

            Token::Eq => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::LogicEq)
                } else {
                    Some(BinExprOp::Set)
                }
            }

            Token::Gt => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Gt) {
                    if self.lexer.skip_if(&Token::Gt) {
                        if self.lexer.skip_if(&Token::Eq) {
                            Some(BinExprOp::SetBitRotr)
                        } else {
                            Some(BinExprOp::BitRor)
                        }
                    } else if self.lexer.skip_if(&Token::Eq) {
                        Some(BinExprOp::SetBitShr)
                    } else {
                        Some(BinExprOp::BitShr)
                    }
                } else if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::LogicGe)
                } else {
                    Some(BinExprOp::LogicGt)
                }
            }

            Token::Caret => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetBitXor)
                } else {
                    Some(BinExprOp::BitXor)
                }
            }

            Token::Or => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Eq) {
                    Some(BinExprOp::SetBitOr)
                } else if self.lexer.skip_if(&Token::Or) {
                    if self.lexer.skip_if(&Token::Eq) {
                        Some(BinExprOp::SetLogicOr)
                    } else {
                        Some(BinExprOp::LogicOr)
                    }
                } else {
                    Some(BinExprOp::BitOr)
                }
            }

            _ => None,
        };

        if result.is_none() {
            self.lexer.rewind(rewind);
        }

        result
    }

    fn parse_expression_primary(&mut self) -> Expr {
        match self.lexer.peek_t() {
            Token::Integer(int) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::Integer(Box::new(IntegerLit {
                    value: int.value(),
                    kind: int.kind(),
                })))
            }

            Token::Float(value) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::Float(FloatLit { value }))
            }

            Token::String(string) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::String(StringLit {
                    value: intern_string_literal(string),
                }))
            }

            Token::BString(data) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::BString(Box::new(BStringLit { value: data })))
            }

            Token::True => {
                self.lexer.skip_tok();
                Expr::Boolean(BooleanLit { value: true })
            }

            Token::False => {
                self.lexer.skip_tok();
                Expr::Boolean(BooleanLit { value: false })
            }

            Token::OpenBracket => Expr::List(Box::new(self.parse_list())),

            Token::Name(_) | Token::Colon => Expr::Path(Box::new(self.parse_path())),

            Token::Type => Expr::TypeInfo(Box::new(TypeInfo {
                the: self.parse_type_info(),
            })),

            Token::Fn | Token::OpenBrace | Token::Unsafe | Token::Safe => {
                Expr::Closure(Box::new(self.parse_closure()))
            }

            Token::If => Expr::If(Box::new(self.parse_if())),
            Token::For => Expr::For(Box::new(self.parse_for())),
            Token::While => Expr::While(Box::new(self.parse_while())),
            // Token::Match => Expr::Match(Box::new(self.parse_match())),
            Token::Break => Expr::Break(Box::new(self.parse_break())),
            Token::Continue => Expr::Continue(Box::new(self.parse_continue())),
            Token::Ret => Expr::Return(Box::new(self.parse_return())),
            Token::Await => Expr::Await(Box::new(self.parse_await())),

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxErr::ExpectedExpr(self.lexer.peek_pos());
                self.log.report(&bug);

                Expr::SyntaxError(ExprSyntaxError)
            }
        }
    }

    fn parse_prefix(&mut self) -> Expr {
        if let Some(operator) = self.detect_and_parse_unary_operator() {
            let precedence = PrecedenceRank::Unary as Precedence;
            let operand = self.parse_expression_precedence(precedence);

            return Expr::UnaryExpr(Box::new(UnaryExpr { operator, operand }));
        }

        if self.lexer.skip_if(&Token::OpenParen) {
            let inner = self.parse_expression();

            if !self.lexer.skip_if(&Token::Comma) {
                if !self.lexer.skip_if(&Token::CloseParen) {
                    let bug = SyntaxErr::ExpectedCloseParen(self.lexer.peek_pos());
                    self.log.report(&bug);
                }

                return Expr::Parentheses(Box::new(ExprParentheses { inner }));
            }

            let mut tuple_elements = vec![inner];

            while !self.lexer.skip_if(&Token::CloseParen) {
                if self.lexer.is_eof() {
                    let bug = SyntaxErr::ExpectedCloseParen(self.lexer.peek_pos());
                    self.log.report(&bug);
                    break;
                }

                let element = self.parse_expression();
                tuple_elements.push(element);

                if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                    let bug = SyntaxErr::ExpectedCloseParen(self.lexer.peek_pos());
                    self.log.report(&bug);

                    self.lexer.skip_while(&Token::CloseParen);
                    break;
                }
            }

            return Expr::Tuple(Box::new(Tuple {
                elements: tuple_elements,
            }));
        }

        self.parse_expression_primary()
    }

    fn parse_expression_precedence(&mut self, min_precedence_to_proceed: Precedence) -> Expr {
        let mut sofar = self.parse_prefix();

        loop {
            let pre_binop_pos = self.lexer.current_pos();
            if let Some(operator) = self.detect_and_parse_binary_operator() {
                let operation = Operation::BinOp(operator);
                let (assoc, op_precedence) = get_precedence(operation);

                if op_precedence < min_precedence_to_proceed {
                    self.lexer.rewind(pre_binop_pos);
                    return sofar;
                }

                let right_expr = if assoc == Associativity::LeftToRight {
                    self.parse_expression_precedence(op_precedence + 1)
                } else {
                    self.parse_expression_precedence(op_precedence)
                };

                sofar = Expr::BinExpr(Box::new(BinExpr {
                    left: sofar,
                    operator,
                    right: right_expr,
                }));
            } else {
                match self.lexer.peek_t() {
                    Token::Dot => {
                        let operation = Operation::FieldAccessOrMethodCall;
                        let (_, new_precedence) = get_precedence(operation);

                        if new_precedence < min_precedence_to_proceed {
                            return sofar;
                        }

                        self.lexer.skip_tok();

                        let member_name = self.lexer.next_if_name().unwrap_or_else(|| {
                            let bug = SyntaxErr::ExpectedFieldOrMethodName(self.lexer.peek_pos());
                            self.log.report(&bug);
                            "".into()
                        });

                        if self.lexer.next_is(&Token::OpenParen) {
                            let (positional, named) = self.parse_function_call_arguments();

                            sofar = Expr::MethodCall(Box::new(MethodCall {
                                object: sofar,
                                method_name: member_name,
                                positional,
                                named,
                            }));

                            continue;
                        } else {
                            sofar = Expr::FieldAccess(Box::new(FieldAccess {
                                object: sofar,
                                field: member_name,
                            }))
                        }
                    }

                    Token::As => {
                        let operation = Operation::Cast;
                        let (_, new_precedence) = get_precedence(operation);

                        if new_precedence < min_precedence_to_proceed {
                            return sofar;
                        }

                        self.lexer.skip_tok();

                        let to = self.parse_type();

                        sofar = Expr::Cast(Box::new(Cast { value: sofar, to }));
                    }

                    Token::OpenParen => {
                        let operation = Operation::FunctionCall;
                        let (_, new_precedence) = get_precedence(operation);

                        if new_precedence < min_precedence_to_proceed {
                            return sofar;
                        }

                        let (positional, named) = self.parse_function_call_arguments();

                        sofar = Expr::FunctionCall(Box::new(FunctionCall {
                            callee: sofar,
                            positional,
                            named,
                        }));
                    }

                    Token::OpenBracket => {
                        let operation = Operation::Index;
                        let (_, new_precedence) = get_precedence(operation);

                        if new_precedence < min_precedence_to_proceed {
                            return sofar;
                        }

                        self.lexer.skip_tok();

                        let index = self.parse_expression();

                        if !self.lexer.skip_if(&Token::CloseBracket) {
                            let bug = SyntaxErr::ExpectedCloseBracket(self.lexer.peek_pos());
                            self.log.report(&bug);
                        }

                        sofar = Expr::IndexAccess(Box::new(IndexAccess {
                            collection: sofar,
                            index,
                        }));
                    }

                    _ => {
                        return sofar;
                    }
                }
            }
        }
    }

    fn parse_literal_suffix(&mut self, value: Expr) -> Expr {
        let suffix = match self.lexer.peek_t() {
            Token::Bool => Type::Bool(Bool {}),
            Token::U8 => Type::UInt8(UInt8 {}),
            Token::U16 => Type::UInt16(UInt16 {}),
            Token::U32 => Type::UInt32(UInt32 {}),
            Token::U64 => Type::UInt64(UInt64 {}),
            Token::U128 => Type::UInt128(UInt128 {}),
            Token::I8 => Type::Int8(Int8 {}),
            Token::I16 => Type::Int16(Int16 {}),
            Token::I32 => Type::Int32(Int32 {}),
            Token::I64 => Type::Int64(Int64 {}),
            Token::I128 => Type::Int128(Int128 {}),
            Token::F8 => Type::TypePath(Box::new(self.create_type_path("f8".to_string()))),
            Token::F16 => Type::TypePath(Box::new(self.create_type_path("f16".to_string()))),
            Token::F32 => Type::Float32(Float32 {}),
            Token::F64 => Type::Float64(Float64 {}),
            Token::F128 => Type::TypePath(Box::new(self.create_type_path("f128".to_string()))),

            Token::Name(name) => Type::TypePath(Box::new(TypePath {
                segments: vec![TypePathSegment {
                    name: name,
                    type_arguments: None,
                }],
                resolved_path: None,
            })),

            _ => return value,
        };

        self.lexer.skip_tok();

        Expr::Cast(Box::new(Cast { value, to: suffix }))
    }

    fn parse_list(&mut self) -> List {
        assert!(self.lexer.peek_t() == Token::OpenBracket);
        self.lexer.skip_tok();

        let mut elements = Vec::new();
        let mut already_reported_too_many_elements = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseBracket) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::ListExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_LIST_ELEMENTS: usize = 65_536;

            if !already_reported_too_many_elements && elements.len() >= MAX_LIST_ELEMENTS {
                already_reported_too_many_elements = true;

                let bug = SyntaxErr::ListElementLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let element = self.parse_expression();
            elements.push(element);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBracket) {
                let bug = SyntaxErr::ListExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                self.lexer.skip_while(&Token::CloseBracket);
                break;
            }
        }

        List { elements }
    }

    pub(crate) fn parse_attributes(&mut self) -> Option<AttributeList> {
        let mut already_reported_too_many_attributes = false;

        if !self.lexer.skip_if(&Token::OpenBracket) {
            return None;
        }

        let mut elements = Vec::new();

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseBracket) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::AttributesExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_ATTRIBUTES: usize = 65_536;

            if !already_reported_too_many_attributes && elements.len() >= MAX_ATTRIBUTES {
                already_reported_too_many_attributes = true;

                let bug = SyntaxErr::AttributesElementLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let attrib = self.parse_expression();
            elements.push(attrib);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBracket) {
                let bug = SyntaxErr::AttributesExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }
        }

        Some(elements)
    }

    pub(crate) fn parse_generic_arguments(&mut self) -> Option<Vec<TypeArgument>> {
        fn parse_generic_argument(this: &mut Parser) -> TypeArgument {
            let mut name = None;

            let rewind_pos = this.lexer.current_pos();
            if let Some(argument_name) = this.lexer.next_if_name() {
                if this.lexer.skip_if(&Token::Colon) {
                    name = Some(intern_arg_name(argument_name));
                } else {
                    this.lexer.rewind(rewind_pos);
                }
            }

            let value = this.parse_type();

            TypeArgument { name, value }
        }

        if !self.lexer.skip_if(&Token::Lt) {
            return None;
        }

        let mut arguments = Vec::new();
        let mut already_reported_too_many_arguments = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::Gt) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::PathGenericArgumentExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_GENERIC_ARGUMENTS: usize = 65_536;

            if !already_reported_too_many_arguments && arguments.len() >= MAX_GENERIC_ARGUMENTS {
                already_reported_too_many_arguments = true;

                let bug = SyntaxErr::PathGenericArgumentLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let argument = parse_generic_argument(self);
            arguments.push(argument);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::Gt) {
                let bug = SyntaxErr::ExpectedCloseAngle(self.lexer.peek_pos());
                self.log.report(&bug);

                self.lexer.skip_while(&Token::Gt);
                break;
            }
        }

        Some(arguments)
    }

    pub(crate) fn parse_path(&mut self) -> ExprPath {
        fn parse_double_colon(this: &mut Parser) -> bool {
            if !this.lexer.skip_if(&Token::Colon) {
                return false;
            }

            if !this.lexer.skip_if(&Token::Colon) {
                let bug = SyntaxErr::ExpectedColon(this.lexer.peek_pos());
                this.log.report(&bug);
                return false;
            }

            true
        }

        assert!(matches!(self.lexer.peek_t(), Token::Name(_) | Token::Colon));

        let mut segments = Vec::new();
        let mut prev_scope = false;
        let mut already_reported_too_many_segments = false;

        if parse_double_colon(self) {
            prev_scope = true;

            let type_arguments = self.parse_generic_arguments();
            if type_arguments.is_some() {
                prev_scope = parse_double_colon(self);
            }

            segments.push(ExprPathSegment {
                name: "".into(),
                type_arguments,
            });
        }

        while prev_scope || segments.is_empty() {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::PathExpectedNameOrSeparator(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_PATH_SEGMENTS: usize = 65_536;
            if !already_reported_too_many_segments && segments.len() >= MAX_PATH_SEGMENTS {
                already_reported_too_many_segments = true;

                let bug = SyntaxErr::PathSegmentLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let Some(identifier) = self.lexer.next_if_name() else {
                let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            };

            prev_scope = parse_double_colon(self);

            if prev_scope {
                let type_arguments = self.parse_generic_arguments();
                if type_arguments.is_some() {
                    prev_scope = parse_double_colon(self);
                }

                segments.push(ExprPathSegment {
                    name: identifier,
                    type_arguments,
                });
            } else {
                segments.push(ExprPathSegment {
                    name: identifier,
                    type_arguments: None,
                });

                break;
            }
        }

        assert_ne!(segments.len(), 0);

        ExprPath {
            segments,
            resolved_path: None,
        }
    }

    fn parse_type_info(&mut self) -> Type {
        assert!(self.lexer.peek_t() == Token::Type);
        self.lexer.skip_tok();

        self.parse_type()
    }

    fn parse_if(&mut self) -> If {
        assert!(self.lexer.peek_t() == Token::If);
        self.lexer.skip_tok();

        let condition = self.parse_expression();
        let true_branch = self.parse_block();

        let false_branch = if self.lexer.skip_if(&Token::Else) {
            if self.lexer.next_is(&Token::If) {
                Some(ElseIf::If(Box::new(self.parse_if())))
            } else {
                Some(ElseIf::Block(self.parse_block()))
            }
        } else {
            None
        };

        If {
            condition,
            true_branch,
            false_branch,
        }
    }

    fn parse_for(&mut self) -> ForEach {
        fn parse_for_bindings(this: &mut Parser) -> Vec<VariableNameId> {
            if !this.lexer.skip_if(&Token::OpenParen) {
                let binding_name = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ForVariableBindingMissingName(this.lexer.peek_pos());
                    this.log.report(&bug);
                    "".into()
                });

                return vec![intern_variable_name(binding_name)];
            }

            let mut bindings = Vec::new();
            let mut already_reported_too_many_bindings = false;

            this.lexer.skip_if(&Token::Comma);

            while !this.lexer.skip_if(&Token::CloseParen) {
                if this.lexer.is_eof() {
                    let bug = SyntaxErr::ForVariableBindingExpectedEnd(this.lexer.peek_pos());
                    this.log.report(&bug);
                    break;
                }

                const MAX_BINDINGS: usize = 65_536;

                if !already_reported_too_many_bindings && bindings.len() >= MAX_BINDINGS {
                    already_reported_too_many_bindings = true;

                    let bug = SyntaxErr::ForVariableBindingLimit(this.lexer.peek_pos());
                    this.log.report(&bug);
                }

                let binding_name = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ForVariableBindingMissingName(this.lexer.peek_pos());
                    this.log.report(&bug);
                    "".into()
                });

                let binding_name = intern_variable_name(binding_name);
                bindings.push(binding_name);

                if !this.lexer.skip_if(&Token::Comma) && !this.lexer.next_is(&Token::CloseParen) {
                    let bug = SyntaxErr::ForVariableBindingExpectedEnd(this.lexer.peek_pos());
                    this.log.report(&bug);

                    this.lexer.skip_while(&Token::CloseParen);
                    break;
                }
            }

            bindings
        }

        assert!(self.lexer.peek_t() == Token::For);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let bindings = parse_for_bindings(self);

        if !self.lexer.skip_if(&Token::In) {
            let bug = SyntaxErr::ForExpectedInKeyword(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let iterable = self.parse_expression();
        let body = self.parse_block();

        ForEach {
            attributes,
            bindings,
            iterable,
            body,
        }
    }

    fn parse_while(&mut self) -> WhileLoop {
        assert!(self.lexer.peek_t() == Token::While);
        self.lexer.skip_tok();

        let condition = if self.lexer.next_is(&Token::OpenBrace) {
            None
        } else {
            Some(self.parse_expression())
        };

        let body = self.parse_block();

        WhileLoop { condition, body }
    }

    fn parse_break(&mut self) -> Break {
        assert!(self.lexer.peek_t() == Token::Break);
        self.lexer.skip_tok();

        let label = if self.lexer.skip_if(&Token::SingleQuote) {
            if let Some(name) = self.lexer.next_if_name() {
                Some(intern_label_name(name))
            } else {
                let bug = SyntaxErr::BreakMissingLabel(self.lexer.peek_pos());
                self.log.report(&bug);
                None
            }
        } else {
            None
        };

        Break { label }
    }

    fn parse_continue(&mut self) -> Continue {
        assert!(self.lexer.peek_t() == Token::Continue);
        self.lexer.skip_tok();

        let label = if self.lexer.skip_if(&Token::SingleQuote) {
            if let Some(name) = self.lexer.next_if_name() {
                Some(intern_label_name(name))
            } else {
                let bug = SyntaxErr::ContinueMissingLabel(self.lexer.peek_pos());
                self.log.report(&bug);
                None
            }
        } else {
            None
        };

        Continue { label }
    }

    fn parse_return(&mut self) -> Return {
        assert!(self.lexer.peek_t() == Token::Ret);
        self.lexer.skip_tok();

        let value = if self.lexer.next_is(&Token::Semi) {
            None
        } else {
            Some(self.parse_expression())
        };

        Return { value }
    }

    fn parse_await(&mut self) -> Await {
        assert!(self.lexer.peek_t() == Token::Await);
        self.lexer.skip_tok();

        let future = self.parse_expression();

        Await { future }
    }

    fn parse_closure_parameters(&mut self) -> Option<Vec<FuncParam>> {
        fn parse_closure_parameter(this: &mut Parser) -> FuncParam {
            let attributes = this.parse_attributes();

            let mut mutability = None;
            if this.lexer.skip_if(&Token::Mut) {
                mutability = Some(Mutability::Mut);
            } else if this.lexer.skip_if(&Token::Const) {
                mutability = Some(Mutability::Const);
            }

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::FunctionParameterMissingName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = intern_parameter_name(name);

            if !this.lexer.skip_if(&Token::Colon) {
                let bug = SyntaxErr::FunctionParameterExpectedType(this.lexer.peek_pos());
                this.log.report(&bug);
            }

            let ty = this.parse_type();

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            FuncParam {
                attributes,
                mutability,
                name,
                ty,
                default_value: default,
            }
        }

        if !self.lexer.skip_if(&Token::OpenParen) {
            return None;
        }

        let mut params = Vec::new();
        let mut already_reported_too_many_parameters = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_FUNCTION_PARAMETERS: usize = 65_536;

            if !already_reported_too_many_parameters && params.len() >= MAX_FUNCTION_PARAMETERS {
                already_reported_too_many_parameters = true;

                let bug = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let param = parse_closure_parameter(self);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        Some(params)
    }

    fn parse_closure(&mut self) -> Closure {
        let unique_id = self.closure_ctr;
        self.closure_ctr += 1;

        if matches!(
            self.lexer.peek_t(),
            Token::OpenBrace | Token::Unsafe | Token::Safe
        ) {
            let definition = self.parse_block();

            return Closure {
                attributes: None,
                unique_id,
                parameters: None,
                return_type: None,
                definition,
            };
        }

        assert!(self.lexer.peek_t() == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let parameters = self.parse_closure_parameters();

        let return_type = if self.lexer.skip_if(&Token::Minus) {
            if !self.lexer.skip_if(&Token::Gt) {
                let bug = SyntaxErr::ExpectedArrow(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            Some(self.parse_type())
        } else {
            None
        };

        let definition = self.parse_block();

        Closure {
            attributes,
            unique_id,
            parameters,
            return_type,
            definition,
        }
    }

    fn parse_function_call_arguments(&mut self) -> (Vec<Expr>, Vec<(ArgNameId, Expr)>) {
        struct ParsedArgument {
            name: Option<ArgNameId>,
            value: Expr,
        }

        fn parse_function_call_argument(this: &mut Parser) -> ParsedArgument {
            let mut name = None;

            let rewind_pos = this.lexer.current_pos();
            if let Some(argument_name) = this.lexer.next_if_name() {
                if this.lexer.skip_if(&Token::Colon) {
                    // Successfully parsed a named argument
                    name = Some(intern_arg_name(argument_name));
                } else {
                    // It was just an identifier that wasn't followed by a colon,
                    // so we treat it as the start of a positional expression.
                    this.lexer.rewind(rewind_pos);
                }
            }

            let value = this.parse_expression();

            ParsedArgument { name, value }
        }

        assert!(self.lexer.peek_t() == Token::OpenParen);
        self.lexer.skip_tok();

        let mut parsed_arguments = Vec::new();
        let mut already_reported_too_many_arguments = false;
        let mut named_argument_seen = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::FunctionCallExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_CARGUMENTS: usize = 65_536;

            if !already_reported_too_many_arguments && parsed_arguments.len() >= MAX_CARGUMENTS {
                already_reported_too_many_arguments = true;

                let bug = SyntaxErr::FunctionCallArgumentLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let argument = parse_function_call_argument(self);

            // 1. Enforce Positional-before-Named rule
            if argument.name.is_some() {
                named_argument_seen = true;
            } else {
                if named_argument_seen {
                    // Error: Positional argument follows a named argument
                    let bug = SyntaxErr::FunctionCallPositionFollowsNamed(self.lexer.peek_pos());
                    self.log.report(&bug);
                }
            }

            parsed_arguments.push(argument);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxErr::FunctionCallExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        // 2. Separate into positional and named vectors for the return type
        let mut positional_args = Vec::new();
        let mut named_args = Vec::new();

        for arg in parsed_arguments {
            match arg.name {
                Some(name_id) => named_args.push((name_id, arg.value)),
                None => positional_args.push(arg.value),
            }
        }

        (positional_args, named_args)
    }

    fn parse_local_variable(&mut self) -> LocalVariable {
        let kind = match self.lexer.next_t() {
            Token::Let => LocalVariableKind::Let,
            Token::Var => LocalVariableKind::Var,
            _ => unreachable!(),
        };

        let attributes = self.parse_attributes();

        let mut mutability = None;
        if self.lexer.skip_if(&Token::Mut) {
            mutability = Some(Mutability::Mut);
        } else if self.lexer.skip_if(&Token::Const) {
            mutability = Some(Mutability::Const);
        }

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::VariableMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_variable_name(name);

        let var_type = if self.lexer.skip_if(&Token::Colon) {
            Some(self.parse_type())
        } else {
            None
        };

        let initializer = if self.lexer.skip_if(&Token::Eq) {
            Some(self.parse_expression())
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Semi) {
            let bug = SyntaxErr::ExpectedSemicolon(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        LocalVariable {
            kind,
            attributes,
            mutability,
            name,
            ty: var_type,
            initializer,
        }
    }

    fn parse_block_item(&mut self) -> BlockItem {
        match self.lexer.peek_t() {
            Token::Let | Token::Var => {
                let var = self.parse_local_variable();
                BlockItem::Variable(var)
            }

            Token::Safe | Token::Unsafe | Token::OpenBrace => {
                let block = self.parse_block();

                if self.lexer.skip_if(&Token::Semi) {
                    BlockItem::Stmt(Expr::Block(Box::new(block)))
                } else {
                    BlockItem::Expr(Expr::Block(Box::new(block)))
                }
            }

            _ => {
                let expr = self.parse_expression();

                if self.lexer.skip_if(&Token::Semi) {
                    BlockItem::Stmt(expr)
                } else {
                    BlockItem::Expr(expr)
                }
            }
        }
    }

    pub(crate) fn parse_block(&mut self) -> Block {
        fn parse_safety_modifier(this: &mut Parser) -> Option<Safety> {
            if this.lexer.skip_if(&Token::Safe) {
                return Some(Safety::Safe);
            }

            if !this.lexer.skip_if(&Token::Unsafe) {
                return None;
            }

            if !this.lexer.skip_if(&Token::OpenParen) {
                return Some(Safety::Unsafe(None));
            }

            let modifier = this.parse_expression();

            if !this.lexer.skip_if(&Token::CloseParen) {
                let bug = SyntaxErr::ExpectedCloseParen(this.lexer.peek_pos());
                this.log.report(&bug);
            }

            Some(Safety::Unsafe(Some(modifier)))
        }

        let safety = parse_safety_modifier(self);

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxErr::ExpectedOpenBrace(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let mut elements = Vec::new();
        let mut already_reported_too_many_elements = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::BlockExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_BLOCK_ELEMENTS: usize = 65_536;

            if !already_reported_too_many_elements && elements.len() >= MAX_BLOCK_ELEMENTS {
                already_reported_too_many_elements = true;

                let bug = SyntaxErr::BlockElementLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let element = self.parse_block_item();
            elements.push(element);
        }

        Block { safety, elements }
    }

    pub fn parse_expression(&mut self) -> Expr {
        self.parse_expression_precedence(Precedence::MIN)
    }
}
