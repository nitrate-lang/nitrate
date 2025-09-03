use crate::lexical::{Integer, Keyword, Name, Op, Punct, Token};
use crate::parsetree::nodes::{
    ArrayType, Assert, Await, BinExpr, BinExprOp, Block, Break, Call, Continue, DoWhileLoop,
    ForEach, Function, FunctionType, GenericType, Identifier, If, List, ManagedRefType, MapType,
    Object, RefinementType, Return, Scope, SliceType, Statement, StructType, Switch, TupleType,
    UnaryExpr, UnaryExprOp, UnmanagedRefType, Variable, VariableKind, WhileLoop,
};
use crate::parsetree::{Expr, Type};
use std::ops::Deref;

// FIXME: Keep this in sync with the parser

#[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
pub struct CodeFormat {}

pub trait ToCode<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat);
}

impl<'a> ToCode<'a> for RefinementType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.base().to_code(tokens, options);

        if let Some(width) = self.width() {
            tokens.push(Token::Punct(Punct::Colon));
            width.to_code(tokens, options);
        }

        if self.min().is_some() || self.max().is_some() {
            tokens.push(Token::Punct(Punct::Colon));
            tokens.push(Token::Punct(Punct::LeftBracket));
            if let Some(min) = self.min() {
                min.to_code(tokens, options);
            }
            tokens.push(Token::Punct(Punct::Colon));
            if let Some(max) = self.max() {
                max.to_code(tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }
    }
}

impl<'a> ToCode<'a> for TupleType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBrace));
        for (i, ty) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            ty.to_code(tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightBrace));
    }
}

impl<'a> ToCode<'a> for ArrayType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        self.element().to_code(tokens, options);
        tokens.push(Token::Punct(Punct::Semicolon));
        self.count().to_code(tokens, options);
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for MapType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        self.key().to_code(tokens, options);
        tokens.push(Token::Op(Op::Arrow));
        self.value().to_code(tokens, options);
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for SliceType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        self.element().to_code(tokens, options);
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for FunctionType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punct(Punct::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
                attr.to_code(tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }

        tokens.push(Token::Punct(Punct::LeftParen));
        for (i, param) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));

            tokens.push(Token::Name(Name::new(param.name())));

            if param.type_().is_known() {
                tokens.push(Token::Punct(Punct::Colon));
                param.type_().to_code(tokens, options);
            }

            if let Some(default_expr) = param.default() {
                tokens.push(Token::Op(Op::Set));
                default_expr.to_code(tokens, options);
            }
        }
        tokens.push(Token::Punct(Punct::RightParen));

        if self.return_type().is_known() {
            tokens.push(Token::Op(Op::Arrow));
            self.return_type().to_code(tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for ManagedRefType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Op(Op::BitAnd));
        if self.is_mutable() {
            tokens.push(Token::Keyword(Keyword::Mut));
        }

        self.target().to_code(tokens, options);
    }
}

impl<'a> ToCode<'a> for UnmanagedRefType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Op(Op::Mul));
        if self.is_mutable() {
            tokens.push(Token::Keyword(Keyword::Mut));
        } else {
            tokens.push(Token::Keyword(Keyword::Const));
        }

        self.target().to_code(tokens, options);
    }
}

impl<'a> ToCode<'a> for GenericType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.base().to_code(tokens, options);

        tokens.push(Token::Op(Op::LogicLt));
        for (i, (name, value)) in self.arguments().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            if !name.is_empty() {
                tokens.push(Token::Name(Name::new(name)));
                tokens.push(Token::Punct(Punct::Colon));
            }
            value.to_code(tokens, options);
        }
        tokens.push(Token::Op(Op::LogicGt));
    }
}

impl<'a> ToCode<'a> for StructType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Struct));

        tokens.push(Token::Punct(Punct::LeftBrace));
        for (i, (name, ty, default)) in self.fields().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            tokens.push(Token::Name(Name::new(name)));
            tokens.push(Token::Punct(Punct::Colon));
            ty.to_code(tokens, options);
            if let Some(default) = default {
                tokens.push(Token::Op(Op::Set));
                default.to_code(tokens, options);
            }
        }
        tokens.push(Token::Punct(Punct::RightBrace));
    }
}

impl<'a> ToCode<'a> for crate::parsetree::nodes::Integer {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let u128 = self
            .get()
            .try_to_u128()
            .expect("IntegerLit apint::UInt value should fit in u128");

        let number = Integer::new(u128, self.kind());
        tokens.push(Token::Integer(number));
    }
}

impl<'a> ToCode<'a> for List<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        for (i, expr) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for Object<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        for (key, value) in self.fields() {
            tokens.push(Token::Name(Name::new(key)));
            tokens.push(Token::Punct(Punct::Colon));

            value.to_code(tokens, options);
            tokens.push(Token::Punct(Punct::Comma));
        }
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for UnaryExprOp {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let operator = Token::Op(match self {
            UnaryExprOp::Add => Op::Add,
            UnaryExprOp::Sub => Op::Sub,
            UnaryExprOp::Mul => Op::Mul,
            UnaryExprOp::BitAnd => Op::BitAnd,
            UnaryExprOp::BitNot => Op::BitNot,
            UnaryExprOp::LogicNot => Op::LogicNot,
            UnaryExprOp::Inc => Op::Inc,
            UnaryExprOp::Dec => Op::Dec,
            UnaryExprOp::Sizeof => Op::Sizeof,
            UnaryExprOp::Alignof => Op::Alignof,
            UnaryExprOp::Typeof => Op::Typeof,
            UnaryExprOp::Question => Op::Question,
        });

        tokens.push(operator);
    }
}

impl<'a> ToCode<'a> for UnaryExpr<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        if self.is_postfix() {
            self.operand().to_code(tokens, options);
            self.operator().to_code(tokens, options);
        } else {
            self.operator().to_code(tokens, options);
            self.operand().to_code(tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for BinExprOp {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let operator = Token::Op(match self {
            BinExprOp::Add => Op::Add,
            BinExprOp::Sub => Op::Sub,
            BinExprOp::Mul => Op::Mul,
            BinExprOp::Div => Op::Div,
            BinExprOp::Mod => Op::Mod,
            BinExprOp::BitAnd => Op::BitAnd,
            BinExprOp::BitOr => Op::BitOr,
            BinExprOp::BitXor => Op::BitXor,
            BinExprOp::BitShl => Op::BitShl,
            BinExprOp::BitShr => Op::BitShr,
            BinExprOp::BitRotl => Op::BitRotl,
            BinExprOp::BitRotr => Op::BitRotr,
            BinExprOp::LogicAnd => Op::LogicAnd,
            BinExprOp::LogicOr => Op::LogicOr,
            BinExprOp::LogicXor => Op::LogicXor,
            BinExprOp::LogicLt => Op::LogicLt,
            BinExprOp::LogicGt => Op::LogicGt,
            BinExprOp::LogicLe => Op::LogicLe,
            BinExprOp::LogicGe => Op::LogicGe,
            BinExprOp::LogicEq => Op::LogicEq,
            BinExprOp::LogicNe => Op::LogicNe,
            BinExprOp::Set => Op::Set,
            BinExprOp::SetPlus => Op::SetPlus,
            BinExprOp::SetMinus => Op::SetMinus,
            BinExprOp::SetTimes => Op::SetTimes,
            BinExprOp::SetSlash => Op::SetSlash,
            BinExprOp::SetPercent => Op::SetPercent,
            BinExprOp::SetBitAnd => Op::SetBitAnd,
            BinExprOp::SetBitOr => Op::SetBitOr,
            BinExprOp::SetBitXor => Op::SetBitXor,
            BinExprOp::SetBitShl => Op::SetBitShl,
            BinExprOp::SetBitShr => Op::SetBitShr,
            BinExprOp::SetBitRotl => Op::SetBitRotl,
            BinExprOp::SetBitRotr => Op::SetBitRotr,
            BinExprOp::SetLogicAnd => Op::SetLogicAnd,
            BinExprOp::SetLogicOr => Op::SetLogicOr,
            BinExprOp::SetLogicXor => Op::SetLogicXor,
            BinExprOp::As => Op::As,
            BinExprOp::Dot => Op::Dot,
            BinExprOp::Ellipsis => Op::Ellipsis,
            BinExprOp::Scope => Op::Scope,
            BinExprOp::Arrow => Op::Arrow,
            BinExprOp::BlockArrow => Op::BlockArrow,
            BinExprOp::Range => Op::Range,
            BinExprOp::Question => Op::Question,
            BinExprOp::Spaceship => Op::Spaceship,
        });

        tokens.push(operator);
    }
}

impl<'a> ToCode<'a> for BinExpr<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.left().to_code(tokens, options);
        self.op().to_code(tokens, options);
        self.right().to_code(tokens, options);
    }
}

impl<'a> ToCode<'a> for Statement<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.get().to_code(tokens, options);
        tokens.push(Token::Punct(Punct::Semicolon));
    }
}

impl<'a> ToCode<'a> for Block<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBrace));
        for expr in self.elements() {
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightBrace));
    }
}

impl<'a> ToCode<'a> for Function<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punct(Punct::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
                attr.to_code(tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }

        tokens.push(Token::Punct(Punct::LeftParen));
        for (i, param) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));

            tokens.push(Token::Name(Name::new(param.name())));

            if param.type_().is_known() {
                tokens.push(Token::Punct(Punct::Colon));
                param.type_().to_code(tokens, options);
            }

            if let Some(default_expr) = param.default() {
                tokens.push(Token::Op(Op::Set));
                default_expr.to_code(tokens, options);
            }
        }
        tokens.push(Token::Punct(Punct::RightParen));

        if self.return_type().is_known() {
            tokens.push(Token::Op(Op::Arrow));
            self.return_type().to_code(tokens, options);
        }

        if let Some(definition) = self.definition() {
            definition.to_code(tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for Variable<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        match self.kind() {
            VariableKind::Let => tokens.push(Token::Keyword(Keyword::Let)),
            VariableKind::Var => tokens.push(Token::Keyword(Keyword::Var)),
        }

        tokens.push(Token::Name(Name::new(self.name())));

        let var_type = self.get_type();
        tokens.push(Token::Punct(Punct::Colon));
        var_type.to_code(tokens, options);

        if let Some(value) = self.value() {
            tokens.push(Token::Op(Op::Set));
            value.to_code(tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for Identifier<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        for (i, segment) in self.segments().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Op(Op::Scope)));
            tokens.push(Token::Name(Name::new(segment)));
        }
    }
}

impl<'a> ToCode<'a> for Scope<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Scope));
        tokens.push(Token::Name(Name::new(self.name())));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punct(Punct::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
                attr.to_code(tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }

        tokens.push(Token::Punct(Punct::LeftBrace));
        for expr in self.elements() {
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightBrace));
    }
}

impl<'a> ToCode<'a> for If<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::If));
        self.condition().to_code(tokens, options);
        self.then_branch().to_code(tokens, options);
        if let Some(else_branch) = self.else_branch() {
            tokens.push(Token::Keyword(Keyword::Else));
            else_branch.to_code(tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for WhileLoop<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::While));
        self.condition().to_code(tokens, options);
        self.body().to_code(tokens, options);
    }
}

impl<'a> ToCode<'a> for DoWhileLoop<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Do));
        self.body().to_code(tokens, options);
        tokens.push(Token::Keyword(Keyword::While));
        self.condition().to_code(tokens, options);
    }
}

impl<'a> ToCode<'a> for Switch<'a> {
    fn to_code(&self, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Switch to_code
    }
}

impl<'a> ToCode<'a> for Break<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Break));
        if let Some(label) = self.label() {
            tokens.push(Token::Punct(Punct::SingleQuote));
            tokens.push(Token::Name(Name::new(label)));
        }
    }
}

impl<'a> ToCode<'a> for Continue<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Continue));
        if let Some(label) = self.label() {
            tokens.push(Token::Punct(Punct::SingleQuote));
            tokens.push(Token::Name(Name::new(label)));
        }
    }
}

impl<'a> ToCode<'a> for Return<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Ret));
        if let Some(value) = self.value() {
            value.to_code(tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for ForEach<'a> {
    fn to_code(&self, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: ForEach to_code
    }
}

impl<'a> ToCode<'a> for Await<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Await));
        self.expression().to_code(tokens, options);
    }
}

impl<'a> ToCode<'a> for Assert<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Assert));

        tokens.push(Token::Punct(Punct::LeftParen));
        self.condition().to_code(tokens, options);
        tokens.push(Token::Punct(Punct::Comma));
        self.message().to_code(tokens, options);
        tokens.push(Token::Punct(Punct::RightParen));
    }
}

impl<'a> ToCode<'a> for Call<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.callee().to_code(tokens, options);

        tokens.push(Token::Punct(Punct::LeftParen));
        for (i, arg) in self.arguments().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            if let Some(param_name) = arg.0 {
                tokens.push(Token::Name(Name::new(param_name)));
                tokens.push(Token::Punct(Punct::Colon));
            }

            arg.1.to_code(tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightParen));
    }
}

impl<'a> ToCode<'a> for Expr<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        match self {
            Expr::Discard => {}

            Expr::HasParentheses(inner) => {
                tokens.push(Token::Punct(Punct::LeftParen));
                inner.to_code(tokens, options);
                tokens.push(Token::Punct(Punct::RightParen));
            }

            Expr::Bool => tokens.push(Token::Name(Name::new("bool"))),
            Expr::UInt8 => tokens.push(Token::Name(Name::new("u8"))),
            Expr::UInt16 => tokens.push(Token::Name(Name::new("u16"))),
            Expr::UInt32 => tokens.push(Token::Name(Name::new("u32"))),
            Expr::UInt64 => tokens.push(Token::Name(Name::new("u64"))),
            Expr::UInt128 => tokens.push(Token::Name(Name::new("u128"))),
            Expr::Int8 => tokens.push(Token::Name(Name::new("i8"))),
            Expr::Int16 => tokens.push(Token::Name(Name::new("i16"))),
            Expr::Int32 => tokens.push(Token::Name(Name::new("i32"))),
            Expr::Int64 => tokens.push(Token::Name(Name::new("i64"))),
            Expr::Int128 => tokens.push(Token::Name(Name::new("i128"))),
            Expr::Float8 => tokens.push(Token::Name(Name::new("f8"))),
            Expr::Float16 => tokens.push(Token::Name(Name::new("f16"))),
            Expr::Float32 => tokens.push(Token::Name(Name::new("f32"))),
            Expr::Float64 => tokens.push(Token::Name(Name::new("f64"))),
            Expr::Float128 => tokens.push(Token::Name(Name::new("f128"))),
            Expr::UnitType => {
                tokens.push(Token::Punct(Punct::LeftParen));
                tokens.push(Token::Punct(Punct::RightParen));
            }

            Expr::InferType => tokens.push(Token::Name(Name::new("_"))),
            Expr::TypeName(e) => tokens.push(Token::Name(Name::new(e))),
            Expr::RefinementType(e) => e.to_code(tokens, options),
            Expr::TupleType(e) => e.to_code(tokens, options),
            Expr::ArrayType(e) => e.to_code(tokens, options),
            Expr::MapType(e) => e.to_code(tokens, options),
            Expr::SliceType(e) => e.to_code(tokens, options),
            Expr::FunctionType(e) => e.to_code(tokens, options),
            Expr::ManagedRefType(e) => e.to_code(tokens, options),
            Expr::UnmanagedRefType(e) => e.to_code(tokens, options),
            Expr::GenericType(e) => e.to_code(tokens, options),
            Expr::OpaqueType(e) => {
                tokens.push(Token::Keyword(Keyword::Opaque));
                tokens.push(Token::Punct(Punct::LeftParen));
                tokens.push(Token::String(e.deref().clone()));
                tokens.push(Token::Punct(Punct::RightParen));
            }
            Expr::StructType(e) => e.to_code(tokens, options),
            Expr::LatentType(e) => {
                tokens.push(Token::Op(Op::Add));
                e.to_code(tokens, options);
            }
            Expr::HasParenthesesType(e) => {
                tokens.push(Token::Punct(Punct::LeftParen));
                e.to_code(tokens, options);
                tokens.push(Token::Punct(Punct::RightParen));
            }

            Expr::Boolean(e) => tokens.push(Token::Keyword(if *e {
                Keyword::True
            } else {
                Keyword::False
            })),
            Expr::Integer(e) => e.to_code(tokens, options),
            Expr::Float(e) => tokens.push(Token::Float(*e)),
            Expr::String(e) => tokens.push(Token::String(e.deref().clone())),
            Expr::BString(e) => tokens.push(Token::BString(e.deref().clone())),
            Expr::Unit => {
                tokens.push(Token::Punct(Punct::LeftParen));
                tokens.push(Token::Punct(Punct::RightParen));
            }

            Expr::TypeEnvelop(t) => {
                tokens.push(Token::Keyword(Keyword::Type));
                t.to_code(tokens, options);
            }
            Expr::List(e) => e.to_code(tokens, options),
            Expr::Object(e) => e.to_code(tokens, options),
            Expr::UnaryExpr(e) => e.to_code(tokens, options),
            Expr::BinExpr(e) => e.to_code(tokens, options),
            Expr::Statement(e) => e.to_code(tokens, options),
            Expr::Block(e) => e.to_code(tokens, options),

            Expr::Function(e) => e.to_code(tokens, options),
            Expr::Variable(e) => e.to_code(tokens, options),
            Expr::Identifier(e) => e.to_code(tokens, options),
            Expr::Scope(e) => e.to_code(tokens, options),

            Expr::If(e) => e.to_code(tokens, options),
            Expr::WhileLoop(e) => e.to_code(tokens, options),
            Expr::DoWhileLoop(e) => e.to_code(tokens, options),
            Expr::Switch(e) => e.to_code(tokens, options),
            Expr::Break(e) => e.to_code(tokens, options),
            Expr::Continue(e) => e.to_code(tokens, options),
            Expr::Return(e) => e.to_code(tokens, options),
            Expr::ForEach(e) => e.to_code(tokens, options),
            Expr::Await(e) => e.to_code(tokens, options),
            Expr::Assert(e) => e.to_code(tokens, options),
            Expr::Call(e) => e.to_code(tokens, options),
        }
    }
}

impl<'a> ToCode<'a> for Type<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        match self {
            Type::Bool => tokens.push(Token::Name(Name::new("bool"))),
            Type::UInt8 => tokens.push(Token::Name(Name::new("u8"))),
            Type::UInt16 => tokens.push(Token::Name(Name::new("u16"))),
            Type::UInt32 => tokens.push(Token::Name(Name::new("u32"))),
            Type::UInt64 => tokens.push(Token::Name(Name::new("u64"))),
            Type::UInt128 => tokens.push(Token::Name(Name::new("u128"))),
            Type::Int8 => tokens.push(Token::Name(Name::new("i8"))),
            Type::Int16 => tokens.push(Token::Name(Name::new("i16"))),
            Type::Int32 => tokens.push(Token::Name(Name::new("i32"))),
            Type::Int64 => tokens.push(Token::Name(Name::new("i64"))),
            Type::Int128 => tokens.push(Token::Name(Name::new("i128"))),
            Type::Float8 => tokens.push(Token::Name(Name::new("f8"))),
            Type::Float16 => tokens.push(Token::Name(Name::new("f16"))),
            Type::Float32 => tokens.push(Token::Name(Name::new("f32"))),
            Type::Float64 => tokens.push(Token::Name(Name::new("f64"))),
            Type::Float128 => tokens.push(Token::Name(Name::new("f128"))),
            Type::UnitType => {
                tokens.push(Token::Punct(Punct::LeftParen));
                tokens.push(Token::Punct(Punct::RightParen));
            }

            Type::InferType => tokens.push(Token::Name(Name::new("_"))),
            Type::TypeName(e) => tokens.push(Token::Name(Name::new(e))),
            Type::RefinementType(e) => e.to_code(tokens, options),
            Type::TupleType(e) => e.to_code(tokens, options),
            Type::ArrayType(e) => e.to_code(tokens, options),
            Type::MapType(e) => e.to_code(tokens, options),
            Type::SliceType(e) => e.to_code(tokens, options),
            Type::FunctionType(e) => e.to_code(tokens, options),
            Type::ManagedRefType(e) => e.to_code(tokens, options),
            Type::UnmanagedRefType(e) => e.to_code(tokens, options),
            Type::GenericType(e) => e.to_code(tokens, options),
            Type::OpaqueType(e) => {
                tokens.push(Token::Keyword(Keyword::Opaque));
                tokens.push(Token::Punct(Punct::LeftParen));
                tokens.push(Token::String(e.deref().clone()));
                tokens.push(Token::Punct(Punct::RightParen));
            }
            Type::StructType(e) => e.to_code(tokens, options),
            Type::LatentType(e) => {
                tokens.push(Token::Op(Op::Add));
                e.to_code(tokens, options);
            }
            Type::HasParenthesesType(e) => {
                tokens.push(Token::Punct(Punct::LeftParen));
                e.to_code(tokens, options);
                tokens.push(Token::Punct(Punct::RightParen));
            }
        }
    }
}
