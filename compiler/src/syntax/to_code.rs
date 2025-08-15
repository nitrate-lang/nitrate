use crate::lexical::{Integer, Keyword, Name, Op, Punct, Token};
use crate::parsetree::node::{
    ArrayType, Assert, Await, BinExpr, BinExprOp, Block, Break, Continue, DoWhileLoop, ForEach,
    Function, FunctionType, GenericType, If, IntegerLit, ListLit, ManagedRefType, MapType,
    ObjectLit, RefinementType, Return, Scope, SliceType, Statement, Switch, TupleType, UnaryExpr,
    UnaryExprOp, UnmanagedRefType, Variable, VariableKind, WhileLoop,
};
use crate::parsetree::{ExprKey, ExprRef, Storage, TypeKey, TypeRef};

#[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
pub struct CodeFormat {}

pub trait ToCode<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat);
}

impl<'a> ToCode<'a> for RefinementType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.base().to_code(bank, tokens, options);

        if let Some(width) = self.width() {
            tokens.push(Token::Punct(Punct::Colon));
            width.to_code(bank, tokens, options);
        }

        if self.min().is_some() || self.max().is_some() {
            tokens.push(Token::Punct(Punct::Colon));
            tokens.push(Token::Punct(Punct::LeftBracket));
            if let Some(min) = self.min() {
                min.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punct(Punct::Colon));
            if let Some(max) = self.max() {
                max.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }
    }
}

impl<'a> ToCode<'a> for TupleType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBrace));
        for (i, ty) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            ty.to_code(bank, tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightBrace));
    }
}

impl<'a> ToCode<'a> for ArrayType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        self.element().to_code(bank, tokens, options);
        tokens.push(Token::Punct(Punct::Semicolon));
        self.count().to_code(bank, tokens, options);
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for MapType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        self.key().to_code(bank, tokens, options);
        tokens.push(Token::Op(Op::Arrow));
        self.value().to_code(bank, tokens, options);
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for SliceType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        self.element().to_code(bank, tokens, options);
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for FunctionType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punct(Punct::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
                attr.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }

        tokens.push(Token::Punct(Punct::LeftParen));
        for (i, param) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));

            tokens.push(Token::Name(Name::new(param.name())));

            let param_ty = param.param_type();
            if !matches!(param_ty.get(bank), TypeRef::InferType) {
                tokens.push(Token::Punct(Punct::Colon));
                param_ty.to_code(bank, tokens, options);
            }

            if let Some(default_expr) = param.default_value() {
                tokens.push(Token::Op(Op::Set));
                default_expr.to_code(bank, tokens, options);
            }
        }
        tokens.push(Token::Punct(Punct::RightParen));

        let return_type = self.return_type();
        if !matches!(return_type.get(bank), TypeRef::InferType) {
            tokens.push(Token::Op(Op::Arrow));
            return_type.to_code(bank, tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for ManagedRefType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Op(Op::BitAnd));
        if self.is_mutable() {
            tokens.push(Token::Keyword(Keyword::Mut));
        }

        self.target().to_code(bank, tokens, options);
    }
}

impl<'a> ToCode<'a> for UnmanagedRefType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Op(Op::Mul));
        if self.is_mutable() {
            tokens.push(Token::Keyword(Keyword::Mut));
        } else {
            tokens.push(Token::Keyword(Keyword::Const));
        }

        self.target().to_code(bank, tokens, options);
    }
}

impl<'a> ToCode<'a> for GenericType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.base().to_code(bank, tokens, options);

        tokens.push(Token::Op(Op::LogicLt));
        for (i, (name, value)) in self.arguments().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            if !name.is_empty() {
                tokens.push(Token::Name(Name::new(name)));
                tokens.push(Token::Punct(Punct::Colon));
            }
            value.to_code(bank, tokens, options);
        }
        tokens.push(Token::Op(Op::LogicGt));
    }
}

impl<'a> ToCode<'a> for IntegerLit {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let u128 = self
            .try_to_u128()
            .expect("IntegerLit apint::UInt value should fit in u128");

        let number = Integer::new(u128, self.kind());
        tokens.push(Token::Integer(number));
    }
}

impl<'a> ToCode<'a> for ListLit<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        for (i, expr) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            expr.to_code(bank, tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for ObjectLit<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBracket));
        for (key, value) in self.get() {
            tokens.push(Token::Name(Name::new(key)));
            tokens.push(Token::Punct(Punct::Colon));

            value.to_code(bank, tokens, options);
            tokens.push(Token::Punct(Punct::Comma));
        }
        tokens.push(Token::Punct(Punct::RightBracket));
    }
}

impl<'a> ToCode<'a> for UnaryExprOp {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
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
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        if self.is_postfix() {
            self.operand().to_code(bank, tokens, options);
            self.operator().to_code(bank, tokens, options);
        } else {
            self.operator().to_code(bank, tokens, options);
            self.operand().to_code(bank, tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for BinExprOp {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
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
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.left().to_code(bank, tokens, options);
        self.op().to_code(bank, tokens, options);
        self.right().to_code(bank, tokens, options);
    }
}

impl<'a> ToCode<'a> for Statement<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.get().to_code(bank, tokens, options);
        tokens.push(Token::Punct(Punct::Semicolon));
    }
}

impl<'a> ToCode<'a> for Block<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punct(Punct::LeftBrace));
        for expr in self.elements() {
            expr.to_code(bank, tokens, options);
        }
        tokens.push(Token::Punct(Punct::RightBrace));
    }
}

impl<'a> ToCode<'a> for Function<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punct(Punct::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
                attr.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }

        if !self.name().is_empty() {
            tokens.push(Token::Name(Name::new(self.name())));
        }

        tokens.push(Token::Punct(Punct::LeftParen));
        for (i, param) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));

            tokens.push(Token::Name(Name::new(param.name())));

            let param_ty = param.param_type();
            if !matches!(param_ty.get(bank), TypeRef::InferType) {
                tokens.push(Token::Punct(Punct::Colon));
                param_ty.to_code(bank, tokens, options);
            }

            if let Some(default_expr) = param.default_value() {
                tokens.push(Token::Op(Op::Set));
                default_expr.to_code(bank, tokens, options);
            }
        }
        tokens.push(Token::Punct(Punct::RightParen));

        let return_type = self.return_type();
        if !matches!(return_type.get(bank), TypeRef::InferType) {
            tokens.push(Token::Op(Op::Arrow));
            return_type.to_code(bank, tokens, options);
        }

        if let Some(definition) = self.definition() {
            definition.to_code(bank, tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for Variable<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        match self.kind() {
            VariableKind::Let => tokens.push(Token::Keyword(Keyword::Let)),
            VariableKind::Var => tokens.push(Token::Keyword(Keyword::Var)),
        }

        tokens.push(Token::Name(Name::new(self.name())));

        let var_type = self.get_type();
        tokens.push(Token::Punct(Punct::Colon));
        var_type.to_code(bank, tokens, options);

        if let Some(value) = self.value() {
            tokens.push(Token::Op(Op::Set));
            value.to_code(bank, tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for Scope<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Scope));

        for (i, name) in self.scope().names().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Op(Op::Scope)));
            tokens.push(Token::Name(Name::new(name)));
        }

        if !self.attributes().is_empty() {
            tokens.push(Token::Punct(Punct::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
                attr.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }

        self.block().to_code(bank, tokens, options);
    }
}

impl<'a> ToCode<'a> for If<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement If to_code
    }
}

impl<'a> ToCode<'a> for WhileLoop<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement WhileLoop to_code
    }
}

impl<'a> ToCode<'a> for DoWhileLoop<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement DoWhileLoop to_code
    }
}

impl<'a> ToCode<'a> for Switch<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement Switch to_code
    }
}

impl<'a> ToCode<'a> for Break<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement Break to_code
    }
}

impl<'a> ToCode<'a> for Continue<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement Continue to_code
    }
}

impl<'a> ToCode<'a> for Return<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Ret));
        if let Some(value) = self.value() {
            value.to_code(bank, tokens, options);
        }
    }
}

impl<'a> ToCode<'a> for ForEach<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement ForEach to_code
    }
}

impl<'a> ToCode<'a> for Await<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement Await to_code
    }
}

impl<'a> ToCode<'a> for Assert<'a> {
    fn to_code(&self, _bank: &Storage<'a>, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement Assert to_code
    }
}

impl<'a> ToCode<'a> for ExprKey<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        if self.is_discard() {
            return;
        }

        let has_parentheses = self.has_parentheses(bank);
        if has_parentheses {
            tokens.push(Token::Punct(Punct::LeftParen));
        }

        match self.get(bank) {
            ExprRef::Bool => tokens.push(Token::Name(Name::new("bool"))),
            ExprRef::UInt8 => tokens.push(Token::Name(Name::new("u8"))),
            ExprRef::UInt16 => tokens.push(Token::Name(Name::new("u16"))),
            ExprRef::UInt32 => tokens.push(Token::Name(Name::new("u32"))),
            ExprRef::UInt64 => tokens.push(Token::Name(Name::new("u64"))),
            ExprRef::UInt128 => tokens.push(Token::Name(Name::new("u128"))),
            ExprRef::Int8 => tokens.push(Token::Name(Name::new("i8"))),
            ExprRef::Int16 => tokens.push(Token::Name(Name::new("i16"))),
            ExprRef::Int32 => tokens.push(Token::Name(Name::new("i32"))),
            ExprRef::Int64 => tokens.push(Token::Name(Name::new("i64"))),
            ExprRef::Int128 => tokens.push(Token::Name(Name::new("i128"))),
            ExprRef::Float8 => tokens.push(Token::Name(Name::new("f8"))),
            ExprRef::Float16 => tokens.push(Token::Name(Name::new("f16"))),
            ExprRef::Float32 => tokens.push(Token::Name(Name::new("f32"))),
            ExprRef::Float64 => tokens.push(Token::Name(Name::new("f64"))),
            ExprRef::Float128 => tokens.push(Token::Name(Name::new("f128"))),

            ExprRef::InferType => tokens.push(Token::Name(Name::new("_"))),
            ExprRef::TypeName(e) => tokens.push(Token::Name(Name::new(e))),
            ExprRef::RefinementType(e) => e.to_code(bank, tokens, options),
            ExprRef::TupleType(e) => e.to_code(bank, tokens, options),
            ExprRef::ArrayType(e) => e.to_code(bank, tokens, options),
            ExprRef::MapType(e) => e.to_code(bank, tokens, options),
            ExprRef::SliceType(e) => e.to_code(bank, tokens, options),
            ExprRef::FunctionType(e) => e.to_code(bank, tokens, options),
            ExprRef::ManagedRefType(e) => e.to_code(bank, tokens, options),
            ExprRef::UnmanagedRefType(e) => e.to_code(bank, tokens, options),
            ExprRef::GenericType(e) => e.to_code(bank, tokens, options),
            ExprRef::OpaqueType(e) => {
                tokens.push(Token::Keyword(Keyword::Opaque));
                tokens.push(Token::Punct(Punct::LeftParen));
                tokens.push(Token::String(e.clone()));
                tokens.push(Token::Punct(Punct::RightParen));
            }

            ExprRef::Discard => {}

            ExprRef::BooleanLit(e) => tokens.push(Token::Keyword(if e {
                Keyword::True
            } else {
                Keyword::False
            })),
            ExprRef::IntegerLit(e) => e.to_code(bank, tokens, options),
            ExprRef::FloatLit(e) => tokens.push(Token::Float(e)),
            ExprRef::StringLit(e) => tokens.push(Token::String(e.clone())),
            ExprRef::BStringLit(e) => tokens.push(Token::BString(e.clone())),
            ExprRef::ListLit(e) => e.to_code(bank, tokens, options),
            ExprRef::ObjectLit(e) => e.to_code(bank, tokens, options),

            ExprRef::UnaryExpr(e) => e.to_code(bank, tokens, options),
            ExprRef::BinExpr(e) => e.to_code(bank, tokens, options),
            ExprRef::Statement(e) => e.to_code(bank, tokens, options),
            ExprRef::Block(e) => e.to_code(bank, tokens, options),

            ExprRef::Function(e) => e.to_code(bank, tokens, options),
            ExprRef::Variable(e) => e.to_code(bank, tokens, options),
            ExprRef::Identifier(name) => tokens.push(Token::Name(Name::new(name))),
            ExprRef::Scope(e) => e.to_code(bank, tokens, options),

            ExprRef::If(e) => e.to_code(bank, tokens, options),
            ExprRef::WhileLoop(e) => e.to_code(bank, tokens, options),
            ExprRef::DoWhileLoop(e) => e.to_code(bank, tokens, options),
            ExprRef::Switch(e) => e.to_code(bank, tokens, options),
            ExprRef::Break(e) => e.to_code(bank, tokens, options),
            ExprRef::Continue(e) => e.to_code(bank, tokens, options),
            ExprRef::Return(e) => e.to_code(bank, tokens, options),
            ExprRef::ForEach(e) => e.to_code(bank, tokens, options),
            ExprRef::Await(e) => e.to_code(bank, tokens, options),
            ExprRef::Assert(e) => e.to_code(bank, tokens, options),
        }

        if has_parentheses {
            tokens.push(Token::Punct(Punct::RightParen));
        }
    }
}

impl<'a> ToCode<'a> for TypeKey<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        let has_parentheses = self.has_parentheses(bank);

        if has_parentheses {
            tokens.push(Token::Punct(Punct::LeftParen));
        }

        match self.get(bank) {
            TypeRef::Bool => tokens.push(Token::Name(Name::new("bool"))),
            TypeRef::UInt8 => tokens.push(Token::Name(Name::new("u8"))),
            TypeRef::UInt16 => tokens.push(Token::Name(Name::new("u16"))),
            TypeRef::UInt32 => tokens.push(Token::Name(Name::new("u32"))),
            TypeRef::UInt64 => tokens.push(Token::Name(Name::new("u64"))),
            TypeRef::UInt128 => tokens.push(Token::Name(Name::new("u128"))),
            TypeRef::Int8 => tokens.push(Token::Name(Name::new("i8"))),
            TypeRef::Int16 => tokens.push(Token::Name(Name::new("i16"))),
            TypeRef::Int32 => tokens.push(Token::Name(Name::new("i32"))),
            TypeRef::Int64 => tokens.push(Token::Name(Name::new("i64"))),
            TypeRef::Int128 => tokens.push(Token::Name(Name::new("i128"))),
            TypeRef::Float8 => tokens.push(Token::Name(Name::new("f8"))),
            TypeRef::Float16 => tokens.push(Token::Name(Name::new("f16"))),
            TypeRef::Float32 => tokens.push(Token::Name(Name::new("f32"))),
            TypeRef::Float64 => tokens.push(Token::Name(Name::new("f64"))),
            TypeRef::Float128 => tokens.push(Token::Name(Name::new("f128"))),

            TypeRef::InferType => tokens.push(Token::Name(Name::new("_"))),
            TypeRef::TypeName(e) => tokens.push(Token::Name(Name::new(e))),
            TypeRef::RefinementType(e) => e.to_code(bank, tokens, options),
            TypeRef::TupleType(e) => e.to_code(bank, tokens, options),
            TypeRef::ArrayType(e) => e.to_code(bank, tokens, options),
            TypeRef::MapType(e) => e.to_code(bank, tokens, options),
            TypeRef::SliceType(e) => e.to_code(bank, tokens, options),
            TypeRef::FunctionType(e) => e.to_code(bank, tokens, options),
            TypeRef::ManagedRefType(e) => e.to_code(bank, tokens, options),
            TypeRef::UnmanagedRefType(e) => e.to_code(bank, tokens, options),
            TypeRef::GenericType(e) => e.to_code(bank, tokens, options),
            TypeRef::OpaqueType(e) => {
                tokens.push(Token::Keyword(Keyword::Opaque));
                tokens.push(Token::Punct(Punct::LeftParen));
                tokens.push(Token::String(e.clone()));
                tokens.push(Token::Punct(Punct::RightParen));
            }
        }

        if has_parentheses {
            tokens.push(Token::Punct(Punct::RightParen));
        }
    }
}
