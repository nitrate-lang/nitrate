use std::sync::{Arc, RwLock};

use crate::bugs::SyntaxErr;

use super::parse::Parser;
use nitrate_parsetree::{
    kind::{
        Await, BStringLit, BinExpr, BinExprOp, Block, BlockItem, Bool, BooleanLit, Break, Call,
        CallArgument, Cast, Closure, Continue, DoWhileLoop, Expr, ExprParentheses, ExprSyntaxError,
        Float8, Float16, Float32, Float64, Float128, FloatLit, ForEach, FunctionParameter, If,
        IndexAccess, Int8, Int16, Int32, Int64, Int128, IntegerLit, List, Mutability, Path,
        PathExprSegment, Return, Safety, StringLit, Type, TypeArgument, TypeInfo, TypePath,
        TypePathSegment, UInt8, UInt16, UInt32, UInt64, UInt128, UnaryExpr, UnaryExprOp, WhileLoop,
    },
    tag::{
        VariableNameId, intern_arg_name, intern_label_name, intern_parameter_name,
        intern_string_literal, intern_variable_name,
    },
};
use nitrate_tokenize::Token;

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
}

fn get_precedence_of_binary_operator(op: BinExprOp) -> (Associativity, Precedence) {
    let (associativity, precedence) = match op {
        BinExprOp::Dot | BinExprOp::Arrow => {
            (Associativity::LeftToRight, PrecedenceRank::FieldAccess)
        }

        BinExprOp::As => (Associativity::LeftToRight, PrecedenceRank::Cast),

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
        BinExprOp::LogicXor => (Associativity::LeftToRight, PrecedenceRank::LogicXor),
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
        | BinExprOp::SetLogicOr
        | BinExprOp::SetLogicXor => (Associativity::RightToLeft, PrecedenceRank::Assign),
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
                Some(UnaryExprOp::AddressOf)
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
        match self.lexer.peek_t() {
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
                } else if self.lexer.skip_if(&Token::Gt) {
                    Some(BinExprOp::Arrow)
                } else {
                    Some(BinExprOp::Sub)
                }
            }

            Token::Dot => {
                self.lexer.skip_tok();
                if self.lexer.skip_if(&Token::Dot) {
                    Some(BinExprOp::Range)
                } else {
                    Some(BinExprOp::Dot)
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
                } else if self.lexer.skip_if(&Token::Caret) {
                    if self.lexer.skip_if(&Token::Eq) {
                        Some(BinExprOp::SetLogicXor)
                    } else {
                        Some(BinExprOp::LogicXor)
                    }
                } else {
                    Some(BinExprOp::BitXor)
                }
            }

            Token::As => {
                self.lexer.skip_tok();
                Some(BinExprOp::As)
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
        }
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
            Token::Do => Expr::DoWhileLoop(Box::new(self.parse_do())),
            // Token::Switch => Expr::Switch(Box::new(self.parse_switch())),
            Token::Break => Expr::Break(Box::new(self.parse_break())),
            Token::Continue => Expr::Continue(Box::new(self.parse_continue())),
            Token::Ret => Expr::Return(Box::new(self.parse_return())),
            Token::Await => Expr::Await(Box::new(self.parse_await())),

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxErr::ExpectedExpr(self.lexer.peek_pos());
                self.bugs.push(&bug);

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

            if !self.lexer.skip_if(&Token::CloseParen) {
                let bug = SyntaxErr::ExpectedCloseParen(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            return Expr::Parentheses(Box::new(ExprParentheses { inner }));
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
                self.lexer.rewind(pre_binop_pos);

                match self.lexer.peek_t() {
                    Token::OpenParen => {
                        let operation = Operation::FunctionCall;
                        let (_, new_precedence) = get_precedence(operation);

                        if new_precedence < min_precedence_to_proceed {
                            return sofar;
                        }

                        let arguments = self.parse_function_call_arguments();

                        sofar = Expr::Call(Box::new(Call {
                            callee: sofar,
                            arguments,
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
                            self.bugs.push(&bug);
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
            Token::F8 => Type::Float8(Float8 {}),
            Token::F16 => Type::Float16(Float16 {}),
            Token::F32 => Type::Float32(Float32 {}),
            Token::F64 => Type::Float64(Float64 {}),
            Token::F128 => Type::Float128(Float128 {}),

            Token::Name(name) => Type::TypePath(Box::new(TypePath {
                segments: vec![TypePathSegment {
                    identifier: name,
                    type_arguments: None,
                }],
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
                self.bugs.push(&bug);
                break;
            }

            const MAX_LIST_ELEMENTS: usize = 65_536;

            if !already_reported_too_many_elements && elements.len() >= MAX_LIST_ELEMENTS {
                already_reported_too_many_elements = true;

                let bug = SyntaxErr::ListElementLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let element = self.parse_expression();
            elements.push(element);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBracket) {
                let bug = SyntaxErr::ListExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                self.lexer.skip_while(&Token::CloseBracket);
                break;
            }
        }

        List { elements }
    }

    pub(crate) fn parse_attributes(&mut self) -> Option<Vec<Expr>> {
        let mut already_reported_too_many_attributes = false;

        if !self.lexer.skip_if(&Token::OpenBracket) {
            return None;
        }

        let mut attributes = Vec::new();

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseBracket) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::AttributesExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_ATTRIBUTES: usize = 65_536;

            if !already_reported_too_many_attributes && attributes.len() >= MAX_ATTRIBUTES {
                already_reported_too_many_attributes = true;

                let bug = SyntaxErr::AttributesElementLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let attrib = self.parse_expression();
            attributes.push(attrib);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBracket) {
                let bug = SyntaxErr::AttributesExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }
        }

        Some(attributes)
    }

    pub(crate) fn parse_generic_arguments(&mut self) -> Vec<TypeArgument> {
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
            return Vec::new();
        }

        let mut arguments = Vec::new();
        let mut already_reported_too_many_arguments = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::Gt) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::PathGenericArgumentExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_GENERIC_ARGUMENTS: usize = 65_536;

            if !already_reported_too_many_arguments && arguments.len() >= MAX_GENERIC_ARGUMENTS {
                already_reported_too_many_arguments = true;

                let bug = SyntaxErr::PathGenericArgumentLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let argument = parse_generic_argument(self);
            arguments.push(argument);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::Gt) {
                let bug = SyntaxErr::ExpectedCloseAngle(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::Gt);
                break;
            }
        }

        arguments
    }

    pub(crate) fn parse_path(&mut self) -> Path {
        // TODO: Cleanup

        assert!(matches!(self.lexer.peek_t(), Token::Name(_) | Token::Colon));

        let mut path = Vec::new();
        let mut last_was_scope = false;

        loop {
            match self.lexer.peek_t() {
                Token::Name(name) => {
                    if !last_was_scope && !path.is_empty() {
                        break;
                    }

                    self.lexer.skip_tok();

                    path.push(PathExprSegment {
                        identifier: name,
                        type_arguments: None,
                    });
                    last_was_scope = false;
                }

                Token::Colon => {
                    self.lexer.skip_tok();
                    if !self.lexer.skip_if(&Token::Colon) {
                        last_was_scope = false;
                        break;
                    }

                    if last_was_scope {
                        let bug = SyntaxErr::PathUnexpectedScopeSeparator(self.lexer.peek_pos());
                        self.bugs.push(&bug);
                        break;
                    }

                    if path.is_empty() {
                        path.push(PathExprSegment {
                            identifier: "".into(),
                            type_arguments: None,
                        });
                    }

                    last_was_scope = true;
                }

                _ => break,
            }
        }

        if path.is_empty() {
            let bug = SyntaxErr::PathIsEmpty(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        if last_was_scope {
            let bug = SyntaxErr::PathTrailingScopeSeparator(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        // TODO: Support generic type arguments in paths

        Path { segments: path }
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
        let then_branch = self.parse_block();

        let else_branch = if self.lexer.skip_if(&Token::Else) {
            if self.lexer.next_is(&Token::If) {
                let else_branch = Expr::If(Box::new(self.parse_if()));

                Some(Block {
                    safety: None,
                    elements: vec![BlockItem::Expr(else_branch)],
                    ends_with_semi: false,
                })
            } else {
                Some(self.parse_block())
            }
        } else {
            None
        };

        If {
            condition,
            then_branch,
            else_branch,
        }
    }

    fn parse_for(&mut self) -> ForEach {
        fn parse_for_bindings(this: &mut Parser) -> Vec<(VariableNameId, Option<Type>)> {
            if !this.lexer.skip_if(&Token::OpenParen) {
                let variable_name = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ForVariableBindingMissingName(this.lexer.peek_pos());
                    this.bugs.push(&bug);
                    "".into()
                });

                let variable_name = intern_variable_name(variable_name);

                let type_annotation = if this.lexer.skip_if(&Token::Colon) {
                    Some(this.parse_type())
                } else {
                    None
                };

                return vec![(variable_name, type_annotation)];
            }

            let mut bindings = Vec::new();
            let mut already_reported_too_many_bindings = false;

            this.lexer.skip_if(&Token::Comma);

            while !this.lexer.skip_if(&Token::CloseParen) {
                if this.lexer.is_eof() {
                    let bug = SyntaxErr::ForVariableBindingExpectedEnd(this.lexer.peek_pos());
                    this.bugs.push(&bug);
                    break;
                }

                const MAX_BINDINGS: usize = 65_536;

                if !already_reported_too_many_bindings && bindings.len() >= MAX_BINDINGS {
                    already_reported_too_many_bindings = true;

                    let bug = SyntaxErr::ForVariableBindingLimit(this.lexer.peek_pos());
                    this.bugs.push(&bug);
                }

                let variable_name = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ForVariableBindingMissingName(this.lexer.peek_pos());
                    this.bugs.push(&bug);
                    "".into()
                });

                let variable_name = intern_variable_name(variable_name);

                let type_annotation = if this.lexer.skip_if(&Token::Colon) {
                    Some(this.parse_type())
                } else {
                    None
                };

                bindings.push((variable_name, type_annotation));

                if !this.lexer.skip_if(&Token::Comma) && !this.lexer.next_is(&Token::CloseParen) {
                    let bug = SyntaxErr::ForVariableBindingExpectedEnd(this.lexer.peek_pos());
                    this.bugs.push(&bug);

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
            self.bugs.push(&bug);
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
            Expr::Boolean(BooleanLit { value: true })
        } else {
            self.parse_expression()
        };

        let body = self.parse_block();

        WhileLoop { condition, body }
    }

    fn parse_do(&mut self) -> DoWhileLoop {
        assert!(self.lexer.peek_t() == Token::Do);
        self.lexer.skip_tok();

        let body = self.parse_block();

        if !self.lexer.skip_if(&Token::While) {
            let bug = SyntaxErr::DoWhileExpectedWhileKeyword(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let condition = self.parse_expression();

        DoWhileLoop { body, condition }
    }

    fn parse_break(&mut self) -> Break {
        assert!(self.lexer.peek_t() == Token::Break);
        self.lexer.skip_tok();

        let label = if self.lexer.skip_if(&Token::SingleQuote) {
            if let Some(name) = self.lexer.next_if_name() {
                Some(intern_label_name(name))
            } else {
                let bug = SyntaxErr::BreakMissingLabel(self.lexer.peek_pos());
                self.bugs.push(&bug);
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
                self.bugs.push(&bug);
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

    fn parse_closure_parameters(&mut self) -> Vec<FunctionParameter> {
        fn parse_closure_parameter(this: &mut Parser) -> FunctionParameter {
            let attributes = this.parse_attributes();

            let mut mutability = None;
            if this.lexer.skip_if(&Token::Mut) {
                mutability = Some(Mutability::Mutable);
            } else if this.lexer.skip_if(&Token::Const) {
                mutability = Some(Mutability::Const);
            }

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::FunctionParameterMissingName(this.lexer.peek_pos());
                this.bugs.push(&bug);
                "".into()
            });

            let name = intern_parameter_name(name);

            let param_type = if this.lexer.skip_if(&Token::Colon) {
                Some(this.parse_type())
            } else {
                None
            };

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            FunctionParameter {
                attributes,
                mutability,
                name,
                param_type,
                default,
            }
        }

        if !self.lexer.skip_if(&Token::OpenParen) {
            return Vec::new();
        }

        let mut params = Vec::new();
        let mut already_reported_too_many_parameters = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_FUNCTION_PARAMETERS: usize = 65_536;

            if !already_reported_too_many_parameters && params.len() >= MAX_FUNCTION_PARAMETERS {
                already_reported_too_many_parameters = true;

                let bug = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let param = parse_closure_parameter(self);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        params
    }

    fn parse_closure(&mut self) -> Closure {
        if matches!(
            self.lexer.peek_t(),
            Token::OpenBrace | Token::Unsafe | Token::Safe
        ) {
            let definition = self.parse_block();

            return Closure {
                attributes: None,
                parameters: Vec::new(),
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
                self.bugs.push(&bug);
            }

            Some(self.parse_type())
        } else {
            None
        };

        let definition = self.parse_block();

        Closure {
            attributes,
            parameters,
            return_type,
            definition,
        }
    }

    fn parse_function_call_arguments(&mut self) -> Vec<CallArgument> {
        fn parse_function_call_argument(this: &mut Parser) -> CallArgument {
            let mut name = None;

            let rewind_pos = this.lexer.current_pos();
            if let Some(argument_name) = this.lexer.next_if_name() {
                if this.lexer.skip_if(&Token::Colon) {
                    name = Some(intern_arg_name(argument_name));
                } else {
                    this.lexer.rewind(rewind_pos);
                }
            }

            let value = this.parse_expression();

            CallArgument { name, value }
        }

        assert!(self.lexer.peek_t() == Token::OpenParen);
        self.lexer.skip_tok();

        let mut arguments = Vec::new();
        let mut already_reported_too_many_arguments = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::FunctionCallExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_CALL_ARGUMENTS: usize = 65_536;

            if !already_reported_too_many_arguments && arguments.len() >= MAX_CALL_ARGUMENTS {
                already_reported_too_many_arguments = true;

                let bug = SyntaxErr::FunctionCallArgumentLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let argument = parse_function_call_argument(self);
            arguments.push(argument);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxErr::FunctionCallExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        arguments
    }

    fn parse_block_item(&mut self) -> BlockItem {
        match self.lexer.peek_t() {
            Token::Static | Token::Const | Token::Let | Token::Var => {
                let var = self.parse_variable();
                BlockItem::Variable(Arc::new(RwLock::new(var)))
            }

            _ => BlockItem::Expr(self.parse_expression()),
        }
    }

    pub(crate) fn parse_block(&mut self) -> Block {
        let safety = if self.lexer.skip_if(&Token::Unsafe) {
            Some(Safety::Unsafe)
        } else if self.lexer.skip_if(&Token::Safe) {
            Some(Safety::Safe)
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxErr::ExpectedOpenBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut elements = Vec::new();
        let mut already_reported_too_many_elements = false;
        let mut ends_with_semi = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::BlockExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_BLOCK_ELEMENTS: usize = 65_536;

            if !already_reported_too_many_elements && elements.len() >= MAX_BLOCK_ELEMENTS {
                already_reported_too_many_elements = true;

                let bug = SyntaxErr::BlockElementLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let element = self.parse_block_item();
            let consumed_semi = matches!(element, BlockItem::Variable(_));
            elements.push(element);

            if consumed_semi || self.lexer.skip_if(&Token::Semi) {
                ends_with_semi = true;
            } else if self.lexer.next_is(&Token::CloseBrace) {
                ends_with_semi = false;
            } else {
                let bug = SyntaxErr::BlockExpectedEnd(self.lexer.current_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::CloseBrace);
                break;
            }
        }

        Block {
            safety,
            elements,
            ends_with_semi,
        }
    }

    pub(crate) fn parse_expression(&mut self) -> Expr {
        self.parse_expression_precedence(Precedence::MIN)
    }
}
