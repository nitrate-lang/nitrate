use crate::diagnosis::SyntaxErr;
use crate::helper::MAX_LIMIT;

use super::parse::Parser;
use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_tree::ByteSpan;
use nitrate_tree::ast::{
    AttributeList, Await, BStringLit, BinExpr, BinExprOp, Block, BlockItem, Bool, BooleanLit, Break, Cast, Closure,
    Continue, ElseIf, Expr, ExprParentheses, ExprPath, ExprPathSegment, ExprSyntaxError, FieldAccess, Float32, Float64,
    FloatLit, ForEach, FuncParam, FunctionCall, If, IndexAccess, Int8, Int16, Int32, Int64, Int128, IntegerLit, List,
    LocalVariable, LocalVariableKind, MethodCall, Return, Safety, StringLit, StructInit, Tuple, Type, TypeArgument,
    TypeInfo, TypePath, TypePathSegment, UInt8, UInt16, UInt32, UInt64, UInt128, USize, UnaryExpr, UnaryExprOp,
    WhileLoop,
};

type Precedence = u32;

/// A single operator-pattern entry: a sequence of tokens to skip and the resulting `BinExprOp`.
struct OpPattern {
    tokens: &'static [Token],
    op: BinExprOp,
}

/// All binary operator patterns, ordered so longer sequences are tried first.
const BINOP_PATTERNS: &[OpPattern] = &[
    // Three-token patterns (must come before two-token)
    OpPattern {
        tokens: &[Token::Lt, Token::Lt, Token::Lt, Token::Eq],
        op: BinExprOp::SetBitRotl,
    },
    OpPattern {
        tokens: &[Token::Gt, Token::Gt, Token::Gt, Token::Eq],
        op: BinExprOp::SetBitRotr,
    },
    OpPattern {
        tokens: &[Token::And, Token::And, Token::Eq],
        op: BinExprOp::SetLogicAnd,
    },
    OpPattern {
        tokens: &[Token::Or, Token::Or, Token::Eq],
        op: BinExprOp::SetLogicOr,
    },
    OpPattern {
        tokens: &[Token::Lt, Token::Lt, Token::Lt],
        op: BinExprOp::BitRol,
    },
    OpPattern {
        tokens: &[Token::Gt, Token::Gt, Token::Gt],
        op: BinExprOp::BitRor,
    },
    // Two-token patterns
    OpPattern {
        tokens: &[Token::Bang, Token::Eq],
        op: BinExprOp::SetLogicAnd,
    }, // != is handled by ! alone below, this is for !=
    OpPattern {
        tokens: &[Token::Percent, Token::Eq],
        op: BinExprOp::SetPercent,
    },
    OpPattern {
        tokens: &[Token::And, Token::Eq],
        op: BinExprOp::SetBitAnd,
    },
    OpPattern {
        tokens: &[Token::And, Token::And],
        op: BinExprOp::LogicAnd,
    },
    OpPattern {
        tokens: &[Token::Star, Token::Eq],
        op: BinExprOp::SetTimes,
    },
    OpPattern {
        tokens: &[Token::Plus, Token::Eq],
        op: BinExprOp::SetPlus,
    },
    OpPattern {
        tokens: &[Token::Minus, Token::Eq],
        op: BinExprOp::SetMinus,
    },
    OpPattern {
        tokens: &[Token::Slash, Token::Eq],
        op: BinExprOp::SetSlash,
    },
    OpPattern {
        tokens: &[Token::Lt, Token::Lt, Token::Eq],
        op: BinExprOp::SetBitShl,
    },
    OpPattern {
        tokens: &[Token::Lt, Token::Lt],
        op: BinExprOp::BitShl,
    },
    OpPattern {
        tokens: &[Token::Lt, Token::Eq],
        op: BinExprOp::LogicLe,
    },
    OpPattern {
        tokens: &[Token::Eq, Token::Eq],
        op: BinExprOp::LogicEq,
    },
    OpPattern {
        tokens: &[Token::Gt, Token::Gt, Token::Eq],
        op: BinExprOp::SetBitShr,
    },
    OpPattern {
        tokens: &[Token::Gt, Token::Gt],
        op: BinExprOp::BitShr,
    },
    OpPattern {
        tokens: &[Token::Gt, Token::Eq],
        op: BinExprOp::LogicGe,
    },
    OpPattern {
        tokens: &[Token::Caret, Token::Eq],
        op: BinExprOp::SetBitXor,
    },
    OpPattern {
        tokens: &[Token::Or, Token::Eq],
        op: BinExprOp::SetBitOr,
    },
    OpPattern {
        tokens: &[Token::Or, Token::Or],
        op: BinExprOp::LogicOr,
    },
    // Single-token patterns (try these last)
    OpPattern {
        tokens: &[Token::Bang],
        op: BinExprOp::LogicNe,
    },
    OpPattern {
        tokens: &[Token::Percent],
        op: BinExprOp::Mod,
    },
    OpPattern {
        tokens: &[Token::And],
        op: BinExprOp::BitAnd,
    },
    OpPattern {
        tokens: &[Token::Star],
        op: BinExprOp::Mul,
    },
    OpPattern {
        tokens: &[Token::Plus],
        op: BinExprOp::Add,
    },
    OpPattern {
        tokens: &[Token::Minus],
        op: BinExprOp::Sub,
    },
    OpPattern {
        tokens: &[Token::Slash],
        op: BinExprOp::Div,
    },
    OpPattern {
        tokens: &[Token::Eq],
        op: BinExprOp::Set,
    },
    OpPattern {
        tokens: &[Token::Caret],
        op: BinExprOp::BitXor,
    },
    OpPattern {
        tokens: &[Token::Or],
        op: BinExprOp::BitOr,
    },
    OpPattern {
        tokens: &[Token::Lt],
        op: BinExprOp::LogicLt,
    },
    OpPattern {
        tokens: &[Token::Gt],
        op: BinExprOp::LogicGt,
    },
    // Range
    OpPattern {
        tokens: &[Token::Dot, Token::Dot],
        op: BinExprOp::Range,
    },
];

/// Precedence levels in ascending order (lower = binds tighter).
#[repr(u32)]
#[derive(Clone, Copy)]
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
    use BinExprOp::*;
    let (associativity, precedence) = match op {
        Mul | Div | Mod => (Associativity::LeftToRight, PrecedenceRank::MulDivMod),
        Add | Sub => (Associativity::LeftToRight, PrecedenceRank::AddSub),
        BitShl | BitShr | BitRol | BitRor => (Associativity::LeftToRight, PrecedenceRank::BitShiftAndRotate),
        BitAnd => (Associativity::LeftToRight, PrecedenceRank::BitAnd),
        BitXor => (Associativity::LeftToRight, PrecedenceRank::BitXor),
        BitOr => (Associativity::LeftToRight, PrecedenceRank::BitOr),
        LogicEq | LogicNe | LogicLt | LogicGt | LogicLe | LogicGe => {
            (Associativity::LeftToRight, PrecedenceRank::Comparison)
        }
        LogicAnd => (Associativity::LeftToRight, PrecedenceRank::LogicAnd),
        LogicOr => (Associativity::LeftToRight, PrecedenceRank::LogicOr),
        Range => (Associativity::LeftToRight, PrecedenceRank::Range),
        Set | SetPlus | SetMinus | SetTimes | SetSlash | SetPercent | SetBitAnd | SetBitOr | SetBitXor | SetBitShl
        | SetBitShr | SetBitRotl | SetBitRotr | SetLogicAnd | SetLogicOr => {
            (Associativity::RightToLeft, PrecedenceRank::Assign)
        }
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
        Operation::Cast => (Associativity::LeftToRight, PrecedenceRank::Cast as Precedence),
        Operation::FieldAccessOrMethodCall => (Associativity::LeftToRight, PrecedenceRank::FieldAccess as Precedence),
    }
}

impl Parser<'_, '_> {
    fn detect_and_parse_unary_operator(&mut self) -> Option<UnaryExprOp> {
        match self.lexer.peek_tok().token {
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

            Token::Bang => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::Not)
            }

            Token::Typeof => {
                self.lexer.skip_tok();
                Some(UnaryExprOp::Typeof)
            }

            _ => None,
        }
    }

    /// Tries to match a binary operator pattern starting at the current lexer position.
    /// Returns `None` without advancing the lexer if no pattern matches.
    fn detect_and_parse_binary_operator(&mut self) -> Option<BinExprOp> {
        for pattern in BINOP_PATTERNS {
            // Quick reject: check the first token without advancing
            if self.lexer.peek_tok().token != pattern.tokens[0] {
                continue;
            }

            let saved = self.lexer.current_pos();

            // Try the full sequence
            let mut matched = true;
            for tok in pattern.tokens {
                if !self.lexer.skip_if(tok) {
                    matched = false;
                    break;
                }
            }

            if matched {
                return Some(pattern.op);
            }

            // Rewind to try next pattern - clone to avoid move issues
            self.lexer.rewind(saved.clone());
        }

        None
    }

    fn parse_expression_primary(&mut self) -> Expr {
        match self.lexer.peek_tok().token {
            Token::Integer(int) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::Integer(Box::new(IntegerLit {
                    span: ByteSpan::default(),
                    value: int.value(),
                    kind: int.kind(),
                })))
            }

            Token::Float(value) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::Float(FloatLit {
                    span: ByteSpan::default(),
                    value,
                }))
            }

            Token::String(string) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::String(StringLit {
                    span: ByteSpan::default(),
                    value: string,
                }))
            }

            Token::BString(data) => {
                self.lexer.skip_tok();
                self.parse_literal_suffix(Expr::BString(Box::new(BStringLit {
                    span: ByteSpan::default(),
                    value: data,
                })))
            }

            Token::True => {
                self.lexer.skip_tok();
                Expr::Boolean(BooleanLit {
                    span: ByteSpan::default(),
                    value: true,
                })
            }

            Token::False => {
                self.lexer.skip_tok();
                Expr::Boolean(BooleanLit {
                    span: ByteSpan::default(),
                    value: false,
                })
            }

            Token::OpenBracket => Expr::List(Box::new(self.parse_list())),

            Token::Name(_) | Token::Colon | Token::SelfKeyword => {
                let path = self.parse_path();
                if self.lexer.next_is(&Token::OpenBrace) && self.peek_is_struct_field_start() {
                    Expr::StructInit(Box::new(self.parse_struct_object(path)))
                } else {
                    Expr::Path(Box::new(path))
                }
            }

            Token::Type => Expr::TypeInfo(Box::new(TypeInfo {
                span: ByteSpan::default(),
                the: self.parse_type_info(),
            })),

            Token::Fn | Token::OpenBrace | Token::Unsafe | Token::Safe => Expr::Closure(Box::new(self.parse_closure())),

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

                Expr::SyntaxError(ExprSyntaxError {
                    span: ByteSpan::default(),
                })
            }
        }
    }

    fn parse_prefix(&mut self) -> Expr {
        if let Some(operator) = self.detect_and_parse_unary_operator() {
            let precedence = PrecedenceRank::Unary as Precedence;
            let operand = self.parse_expression_precedence(precedence);

            return Expr::UnaryExpr(Box::new(UnaryExpr {
                span: ByteSpan::default(),
                operator,
                operand,
            }));
        }

        if self.lexer.skip_if(&Token::OpenParen) {
            if self.lexer.skip_if(&Token::CloseParen) {
                return Expr::Tuple(Box::new(Tuple {
                    span: ByteSpan::default(),
                    elements: vec![],
                }));
            }

            let inner = self.parse_expression();

            if !self.lexer.skip_if(&Token::Comma) {
                self.expect_close_paren();

                return Expr::Parentheses(Box::new(ExprParentheses {
                    span: ByteSpan::default(),
                    inner,
                }));
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
                span: ByteSpan::default(),
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
                    span: ByteSpan::default(),
                    left: sofar,
                    operator,
                    right: right_expr,
                }));
            } else {
                match self.lexer.peek_tok().token {
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
                                span: ByteSpan::default(),
                                object: sofar,
                                method_name: member_name,
                                positional,
                                named,
                            }));

                            continue;
                        } else {
                            sofar = Expr::FieldAccess(Box::new(FieldAccess {
                                span: ByteSpan::default(),
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

                        sofar = Expr::Cast(Box::new(Cast {
                            span: ByteSpan::default(),
                            value: sofar,
                            to,
                        }));
                    }

                    Token::OpenParen => {
                        let operation = Operation::FunctionCall;
                        let (_, new_precedence) = get_precedence(operation);

                        if new_precedence < min_precedence_to_proceed {
                            return sofar;
                        }

                        let (positional, named) = self.parse_function_call_arguments();

                        sofar = Expr::FunctionCall(Box::new(FunctionCall {
                            span: ByteSpan::default(),
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

                        self.expect_close_bracket();

                        sofar = Expr::IndexAccess(Box::new(IndexAccess {
                            span: ByteSpan::default(),
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
        let suffix = match self.lexer.peek_tok().token {
            Token::Bool => Type::Bool(Bool::default()),
            Token::U8 => Type::UInt8(UInt8::default()),
            Token::U16 => Type::UInt16(UInt16::default()),
            Token::U32 => Type::UInt32(UInt32::default()),
            Token::U64 => Type::UInt64(UInt64::default()),
            Token::U128 => Type::UInt128(UInt128::default()),
            Token::USize => Type::USize(USize::default()),
            Token::I8 => Type::Int8(Int8::default()),
            Token::I16 => Type::Int16(Int16::default()),
            Token::I32 => Type::Int32(Int32::default()),
            Token::I64 => Type::Int64(Int64::default()),
            Token::I128 => Type::Int128(Int128::default()),
            Token::F8 => Type::TypePath(Box::new(self.create_type_path("f8".to_string()))),
            Token::F16 => Type::TypePath(Box::new(self.create_type_path("f16".to_string()))),
            Token::F32 => Type::Float32(Float32::default()),
            Token::F64 => Type::Float64(Float64::default()),
            Token::F128 => Type::TypePath(Box::new(self.create_type_path("f128".to_string()))),

            Token::Name(name) => Type::TypePath(Box::new(TypePath {
                span: ByteSpan::default(),
                segments: vec![TypePathSegment {
                    span: ByteSpan::default(),
                    name,
                    type_arguments: None,
                }],
                resolved_path: None,
            })),

            _ => return value,
        };

        self.lexer.skip_tok();

        Expr::Cast(Box::new(Cast {
            span: ByteSpan::default(),
            value,
            to: suffix,
        }))
    }

    fn parse_list(&mut self) -> List {
        assert!(self.lexer.peek_tok().token == Token::OpenBracket);
        self.lexer.skip_tok();

        let eof = SyntaxErr::ListExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::ListElementLimit(self.lexer.peek_pos());
        let end = SyntaxErr::ListExpectedEnd(self.lexer.peek_pos());

        let elements =
            self.parse_comma_separated_list(&Token::CloseBracket, MAX_LIMIT, true, eof, limit, end, |this| {
                this.parse_expression()
            });

        List {
            span: ByteSpan::default(),
            elements,
        }
    }

    pub(crate) fn parse_attributes(&mut self) -> Option<AttributeList> {
        let mut elements: Vec<Expr> = Vec::new();
        let mut already_reported_too_many_attributes = false;

        while self.lexer.skip_if(&Token::OpenBracket) {
            let eof = SyntaxErr::AttributesExpectedEnd(self.lexer.peek_pos());
            let limit = SyntaxErr::AttributesElementLimit(self.lexer.peek_pos());
            let end = SyntaxErr::AttributesExpectedEnd(self.lexer.peek_pos());

            let inner =
                self.parse_comma_separated_list(&Token::CloseBracket, MAX_LIMIT, true, eof, limit, end, |this| {
                    this.parse_expression()
                });

            let total = elements.len() + inner.len();
            if !already_reported_too_many_attributes && total > MAX_LIMIT {
                already_reported_too_many_attributes = true;
                let bug = SyntaxErr::AttributesElementLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            elements.extend(inner);
        }

        if elements.is_empty() { None } else { Some(elements) }
    }

    pub(crate) fn parse_generic_arguments(&mut self) -> Option<Vec<TypeArgument>> {
        fn parse_generic_argument(this: &mut Parser) -> TypeArgument {
            let mut name = None;

            let rewind_pos = this.lexer.current_pos();
            if let Some(argument_name) = this.lexer.next_if_name() {
                if this.lexer.skip_if(&Token::Colon) {
                    name = Some(NString::from(argument_name));
                } else {
                    this.lexer.rewind(rewind_pos);
                }
            }

            let value = this.parse_type();

            TypeArgument {
                span: ByteSpan::default(),
                name,
                value,
            }
        }

        if !self.lexer.skip_if(&Token::Lt) {
            return None;
        }

        let eof = SyntaxErr::PathGenericArgumentExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::PathGenericArgumentLimit(self.lexer.peek_pos());
        let end = SyntaxErr::ExpectedCloseAngle(self.lexer.peek_pos());

        let arguments =
            self.parse_comma_separated_list(&Token::Gt, MAX_LIMIT, true, eof, limit, end, parse_generic_argument);

        Some(arguments)
    }

    pub(crate) fn parse_path(&mut self) -> ExprPath {
        assert!(matches!(
            self.lexer.peek_tok().token,
            Token::Name(_) | Token::Colon | Token::SelfKeyword
        ));

        let mut segments = Vec::new();
        let mut prev_scope = false;
        let mut already_reported_too_many_segments = false;

        if self.parse_double_colon() {
            prev_scope = true;

            let type_arguments = self.parse_generic_arguments();
            if type_arguments.is_some() {
                prev_scope = self.parse_double_colon();
            }

            segments.push(ExprPathSegment {
                span: ByteSpan::default(),
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

            if !already_reported_too_many_segments && segments.len() >= MAX_LIMIT {
                already_reported_too_many_segments = true;

                let bug = SyntaxErr::PathSegmentLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let Some(identifier) = self.lexer.next_if_name() else {
                let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            };

            prev_scope = self.parse_double_colon();

            if prev_scope {
                let type_arguments = self.parse_generic_arguments();
                if type_arguments.is_some() {
                    prev_scope = self.parse_double_colon();
                }

                segments.push(ExprPathSegment {
                    span: ByteSpan::default(),
                    name: identifier,
                    type_arguments,
                });
            } else {
                segments.push(ExprPathSegment {
                    span: ByteSpan::default(),
                    name: identifier,
                    type_arguments: None,
                });

                break;
            }
        }

        ExprPath {
            span: ByteSpan::default(),
            segments,
            resolved_path: None,
        }
    }

    fn parse_struct_object(&mut self, path: ExprPath) -> StructInit {
        assert!(self.lexer.peek_tok().token == Token::OpenBrace);
        self.lexer.skip_tok();

        let mut fields = Vec::new();

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::StructExpectedFieldOrEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            let field_name = self.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::StructExpectedFieldName(self.lexer.peek_pos());
                self.log.report(&bug);
                "".into()
            });

            self.expect_colon();

            let field_value = self.parse_expression();

            fields.push((field_name.into(), field_value));

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBrace) {
                let bug = SyntaxErr::StructExpectedFieldOrEnd(self.lexer.peek_pos());
                self.log.report(&bug);

                self.lexer.skip_while(&Token::CloseBrace);
                break;
            }
        }

        StructInit {
            span: ByteSpan::default(),
            path,
            fields,
        }
    }

    fn parse_type_info(&mut self) -> Type {
        assert!(self.lexer.peek_tok().token == Token::Type);
        self.lexer.skip_tok();

        self.parse_type()
    }

    fn parse_if(&mut self) -> If {
        let start = self.lexer.peek_pos().offset;
        assert!(self.lexer.peek_tok().token == Token::If);
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
            span: ByteSpan::new(start, self.lexer.current_pos().offset),
            condition,
            true_branch,
            false_branch,
        }
    }

    fn parse_for(&mut self) -> ForEach {
        fn parse_for_bindings(this: &mut Parser) -> Vec<NString> {
            if !this.lexer.skip_if(&Token::OpenParen) {
                let binding_name = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ForVariableBindingMissingName(this.lexer.peek_pos());
                    this.log.report(&bug);
                    "".into()
                });

                return vec![NString::from(binding_name)];
            }

            let eof = SyntaxErr::ForVariableBindingExpectedEnd(this.lexer.peek_pos());
            let limit = SyntaxErr::ForVariableBindingLimit(this.lexer.peek_pos());
            let end = SyntaxErr::ForVariableBindingExpectedEnd(this.lexer.peek_pos());

            this.parse_comma_separated_list(&Token::CloseParen, MAX_LIMIT, true, eof, limit, end, |this| {
                let binding_name = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ForVariableBindingMissingName(this.lexer.peek_pos());
                    this.log.report(&bug);
                    "".into()
                });

                NString::from(binding_name)
            })
        }

        assert!(self.lexer.peek_tok().token == Token::For);
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
            span: ByteSpan::default(),
            attributes,
            bindings,
            iterable,
            body,
        }
    }

    fn parse_while(&mut self) -> WhileLoop {
        assert!(self.lexer.peek_tok().token == Token::While);
        self.lexer.skip_tok();

        let condition = if self.lexer.next_is(&Token::OpenBrace) {
            None
        } else {
            Some(self.parse_expression())
        };

        let body = self.parse_block();

        WhileLoop {
            span: ByteSpan::default(),
            condition,
            body,
        }
    }

    fn parse_break(&mut self) -> Break {
        assert!(self.lexer.peek_tok().token == Token::Break);
        self.lexer.skip_tok();

        let label = if self.lexer.skip_if(&Token::SingleQuote) {
            if let Some(name) = self.lexer.next_if_name() {
                Some(NString::from(name))
            } else {
                let bug = SyntaxErr::BreakMissingLabel(self.lexer.peek_pos());
                self.log.report(&bug);
                None
            }
        } else {
            None
        };

        self.expect_semicolon();

        Break {
            span: ByteSpan::default(),
            label,
        }
    }

    fn parse_continue(&mut self) -> Continue {
        assert!(self.lexer.peek_tok().token == Token::Continue);
        self.lexer.skip_tok();

        let label = if self.lexer.skip_if(&Token::SingleQuote) {
            if let Some(name) = self.lexer.next_if_name() {
                Some(NString::from(name))
            } else {
                let bug = SyntaxErr::ContinueMissingLabel(self.lexer.peek_pos());
                self.log.report(&bug);
                None
            }
        } else {
            None
        };

        self.expect_semicolon();

        Continue {
            span: ByteSpan::default(),
            label,
        }
    }

    fn parse_return(&mut self) -> Return {
        assert!(self.lexer.peek_tok().token == Token::Ret);
        self.lexer.skip_tok();

        let value = if self.lexer.next_is(&Token::Semi) {
            None
        } else {
            Some(self.parse_expression())
        };

        self.expect_semicolon();

        Return {
            span: ByteSpan::default(),
            value,
        }
    }

    fn parse_await(&mut self) -> Await {
        assert!(self.lexer.peek_tok().token == Token::Await);
        self.lexer.skip_tok();

        let future = self.parse_expression();

        Await {
            span: ByteSpan::default(),
            future,
        }
    }

    fn parse_closure_parameters(&mut self) -> Option<Vec<FuncParam>> {
        if !self.lexer.skip_if(&Token::OpenParen) {
            return None;
        }

        let eof = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
        let end = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());

        let params = self.parse_comma_separated_list(&Token::CloseParen, MAX_LIMIT, true, eof, limit, end, |this| {
            this.parse_common_func_param(true)
        });

        Some(params)
    }

    fn parse_closure(&mut self) -> Closure {
        if matches!(
            self.lexer.peek_tok().token,
            Token::OpenBrace | Token::Unsafe | Token::Safe
        ) {
            let definition = self.parse_block();

            return Closure {
                span: ByteSpan::default(),
                attributes: None,
                parameters: None,
                return_type: None,
                definition,
            };
        }

        assert!(self.lexer.peek_tok().token == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let parameters = self.parse_closure_parameters();

        let return_type = self.parse_return_type_arrow();
        let definition = self.parse_block();

        Closure {
            span: ByteSpan::default(),
            attributes,
            parameters,
            return_type,
            definition,
        }
    }

    fn parse_function_call_arguments(&mut self) -> (Vec<Expr>, Vec<(NString, Expr)>) {
        struct ParsedArgument {
            name: Option<NString>,
            value: Expr,
        }

        fn parse_function_call_argument(this: &mut Parser) -> ParsedArgument {
            let mut name = None;

            let rewind_pos = this.lexer.current_pos();
            if let Some(argument_name) = this.lexer.next_if_name() {
                if this.lexer.skip_if(&Token::Colon) {
                    name = Some(NString::from(argument_name));
                } else {
                    this.lexer.rewind(rewind_pos);
                }
            }

            let value = this.parse_expression();

            ParsedArgument { name, value }
        }

        assert!(self.lexer.peek_tok().token == Token::OpenParen);
        self.lexer.skip_tok();

        let mut parsed_arguments = Vec::new();
        let mut named_argument_seen = false;

        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::FunctionCallExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            if parsed_arguments.len() >= MAX_LIMIT {
                let bug = SyntaxErr::FunctionCallArgumentLimit(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            let argument = parse_function_call_argument(self);

            if argument.name.is_some() {
                named_argument_seen = true;
            } else if named_argument_seen {
                let bug = SyntaxErr::FunctionCallPositionFollowsNamed(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            parsed_arguments.push(argument);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxErr::FunctionCallExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);

                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

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
        let kind = match self.lexer.next_tok().token {
            Token::Let => LocalVariableKind::Let,
            Token::Var => LocalVariableKind::Var,
            _ => unreachable!(),
        };

        let attributes = self.parse_attributes();

        let mutability = self.parse_mutability();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::VariableMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = NString::from(name);

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

        self.expect_semicolon();

        LocalVariable {
            span: ByteSpan::default(),
            kind,
            attributes,
            mutability,
            name,
            ty: var_type,
            initializer,
        }
    }

    fn parse_block_item(&mut self) -> BlockItem {
        match self.lexer.peek_tok().token {
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

            this.expect_close_paren();

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

            if !already_reported_too_many_elements && elements.len() >= MAX_LIMIT {
                already_reported_too_many_elements = true;

                let bug = SyntaxErr::BlockElementLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let element = self.parse_block_item();
            elements.push(element);
        }

        Block {
            span: ByteSpan::default(),
            safety,
            elements,
        }
    }

    pub fn parse_expression(&mut self) -> Expr {
        self.parse_expression_precedence(Precedence::MIN)
    }
}
