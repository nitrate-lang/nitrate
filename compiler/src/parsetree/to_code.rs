use super::array_type::ArrayType;
use super::binary_op::{BinaryOp, BinaryOperator};
use super::block::Block;
use super::character::CharLit;
use super::expression::{ExprRef, TypeRef};
use super::function::Function;
use super::function_type::FunctionType;
use super::generic_type::GenericType;
use super::list::ListLit;
use super::map_type::MapType;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::reference::{ManagedType, UnmanagedType};
use super::refinement_type::RefinementType;
use super::returns::Return;
use super::slice_type::SliceType;
use super::statement::Statement;
use super::storage::{ExprKey, Storage, TypeKey};
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::{UnaryOp, UnaryOperator};
use super::variable::{Variable, VariableKind};
use crate::lexer::{
    Float, Integer, Keyword, Name, Operator, Punct, StringLit as StringLitToken, Token,
};

#[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
pub struct CodeFormat {}

pub trait ToCode<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat);
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

impl<'a> ToCode<'a> for FloatLit {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let number = Float::new(self.get());
        tokens.push(Token::Float(number));
    }
}

impl<'a> ToCode<'a> for StringLit<'a> {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let string_lit = StringLitToken::from_ref(self);
        tokens.push(Token::String(string_lit));
    }
}

impl<'a> ToCode<'a> for CharLit {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        tokens.push(Token::Char(self.get()));
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

impl<'a> ToCode<'a> for UnaryOperator {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let operator = Token::Op(match self {
            UnaryOperator::Add => Operator::Add,
            UnaryOperator::Sub => Operator::Sub,
            UnaryOperator::Mul => Operator::Mul,
            UnaryOperator::BitAnd => Operator::BitAnd,
            UnaryOperator::BitNot => Operator::BitNot,
            UnaryOperator::LogicNot => Operator::LogicNot,
            UnaryOperator::Inc => Operator::Inc,
            UnaryOperator::Dec => Operator::Dec,
            UnaryOperator::Sizeof => Operator::Sizeof,
            UnaryOperator::Alignof => Operator::Alignof,
            UnaryOperator::Typeof => Operator::Typeof,
            UnaryOperator::Question => Operator::Question,
        });

        tokens.push(operator);
    }
}

impl<'a> ToCode<'a> for UnaryOp<'a> {
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

impl<'a> ToCode<'a> for BinaryOperator {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let operator = Token::Op(match self {
            BinaryOperator::Add => Operator::Add,
            BinaryOperator::Sub => Operator::Sub,
            BinaryOperator::Mul => Operator::Mul,
            BinaryOperator::Div => Operator::Div,
            BinaryOperator::Mod => Operator::Mod,
            BinaryOperator::BitAnd => Operator::BitAnd,
            BinaryOperator::BitOr => Operator::BitOr,
            BinaryOperator::BitXor => Operator::BitXor,
            BinaryOperator::BitShl => Operator::BitShl,
            BinaryOperator::BitShr => Operator::BitShr,
            BinaryOperator::BitRotl => Operator::BitRotl,
            BinaryOperator::BitRotr => Operator::BitRotr,
            BinaryOperator::LogicAnd => Operator::LogicAnd,
            BinaryOperator::LogicOr => Operator::LogicOr,
            BinaryOperator::LogicXor => Operator::LogicXor,
            BinaryOperator::LogicLt => Operator::LogicLt,
            BinaryOperator::LogicGt => Operator::LogicGt,
            BinaryOperator::LogicLe => Operator::LogicLe,
            BinaryOperator::LogicGe => Operator::LogicGe,
            BinaryOperator::LogicEq => Operator::LogicEq,
            BinaryOperator::LogicNe => Operator::LogicNe,
            BinaryOperator::Set => Operator::Set,
            BinaryOperator::SetPlus => Operator::SetPlus,
            BinaryOperator::SetMinus => Operator::SetMinus,
            BinaryOperator::SetTimes => Operator::SetTimes,
            BinaryOperator::SetSlash => Operator::SetSlash,
            BinaryOperator::SetPercent => Operator::SetPercent,
            BinaryOperator::SetBitAnd => Operator::SetBitAnd,
            BinaryOperator::SetBitOr => Operator::SetBitOr,
            BinaryOperator::SetBitXor => Operator::SetBitXor,
            BinaryOperator::SetBitShl => Operator::SetBitShl,
            BinaryOperator::SetBitShr => Operator::SetBitShr,
            BinaryOperator::SetBitRotl => Operator::SetBitRotl,
            BinaryOperator::SetBitRotr => Operator::SetBitRotr,
            BinaryOperator::SetLogicAnd => Operator::SetLogicAnd,
            BinaryOperator::SetLogicOr => Operator::SetLogicOr,
            BinaryOperator::SetLogicXor => Operator::SetLogicXor,
            BinaryOperator::As => Operator::As,
            BinaryOperator::Dot => Operator::Dot,
            BinaryOperator::Ellipsis => Operator::Ellipsis,
            BinaryOperator::Scope => Operator::Scope,
            BinaryOperator::Arrow => Operator::Arrow,
            BinaryOperator::BlockArrow => Operator::BlockArrow,
            BinaryOperator::Range => Operator::Range,
            BinaryOperator::Question => Operator::Question,
            BinaryOperator::Spaceship => Operator::Spaceship,
        });

        tokens.push(operator);
    }
}

impl<'a> ToCode<'a> for BinaryOp<'a> {
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
        for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));

            tokens.push(Token::Name(Name::new(name)));

            if let Some(ty) = ty {
                if !matches!(ty.get(bank), TypeRef::InferType) {
                    tokens.push(Token::Punct(Punct::Colon));
                    ty.to_code(bank, tokens, options);
                }
            }

            if let Some(default_expr) = default {
                tokens.push(Token::Op(Operator::Set));
                default_expr.to_code(bank, tokens, options);
            }
        }
        tokens.push(Token::Punct(Punct::RightParen));

        if let Some(return_type) = self.return_type() {
            if !matches!(return_type.get(bank), TypeRef::InferType) {
                tokens.push(Token::Punct(Punct::Colon));
                return_type.to_code(bank, tokens, options);
            }
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

        if let Some(var_type) = self.get_type() {
            tokens.push(Token::Punct(Punct::Colon));
            var_type.to_code(bank, tokens, options);
        }

        if let Some(value) = self.value() {
            tokens.push(Token::Op(Operator::Set));
            value.to_code(bank, tokens, options);
        }
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

impl<'a> ToCode<'a> for RefinementType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.principal().to_code(bank, tokens, options);

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
        tokens.push(Token::Op(Operator::Arrow));
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

impl<'a> ToCode<'a> for StructType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Struct));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punct(Punct::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i != 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
                attr.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punct(Punct::RightBracket));
        }

        if let Some(name) = self.name() {
            tokens.push(Token::Name(Name::new(name)));
        }

        tokens.push(Token::Punct(Punct::LeftBrace));
        for (field_name, field_ty) in self.fields() {
            tokens.push(Token::Name(Name::new(field_name)));
            tokens.push(Token::Punct(Punct::Colon));
            field_ty.to_code(bank, tokens, options);
            tokens.push(Token::Punct(Punct::Comma));
        }
        tokens.push(Token::Punct(Punct::RightBrace));
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
        for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));

            tokens.push(Token::Name(Name::new(name)));

            if let Some(ty) = ty {
                if !matches!(ty.get(bank), TypeRef::InferType) {
                    tokens.push(Token::Punct(Punct::Colon));
                    ty.to_code(bank, tokens, options);
                }
            }

            if let Some(default_expr) = default {
                tokens.push(Token::Op(Operator::Set));
                default_expr.to_code(bank, tokens, options);
            }
        }
        tokens.push(Token::Punct(Punct::RightParen));

        if let Some(return_type) = self.return_type() {
            if !matches!(return_type.get(bank), TypeRef::InferType) {
                tokens.push(Token::Punct(Punct::Colon));
                return_type.to_code(bank, tokens, options);
            }
        }
    }
}

impl<'a> ToCode<'a> for ManagedType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Op(Operator::BitAnd));
        if self.is_mutable() {
            tokens.push(Token::Keyword(Keyword::Mut));
        }

        self.target().to_code(bank, tokens, options);
    }
}

impl<'a> ToCode<'a> for UnmanagedType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Op(Operator::Mul));
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
        self.principal().to_code(bank, tokens, options);

        tokens.push(Token::Op(Operator::LogicLt));
        for (i, (name, value)) in self.arguments().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punct(Punct::Comma)));
            if !name.is_empty() {
                tokens.push(Token::Name(Name::new(name)));
                tokens.push(Token::Punct(Punct::Colon));
            }
            value.to_code(bank, tokens, options);
        }
        tokens.push(Token::Op(Operator::LogicGt));
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
            ExprRef::StructType(e) => e.to_code(bank, tokens, options),
            ExprRef::FunctionType(e) => e.to_code(bank, tokens, options),
            ExprRef::ManagedType(e) => e.to_code(bank, tokens, options),
            ExprRef::UnmanagedType(e) => e.to_code(bank, tokens, options),
            ExprRef::GenericType(e) => e.to_code(bank, tokens, options),

            ExprRef::Discard => {}

            ExprRef::IntegerLit(e) => e.to_code(bank, tokens, options),
            ExprRef::FloatLit(e) => e.to_code(bank, tokens, options),
            ExprRef::StringLit(e) => e.to_code(bank, tokens, options),
            ExprRef::CharLit(e) => e.to_code(bank, tokens, options),
            ExprRef::ListLit(e) => e.to_code(bank, tokens, options),
            ExprRef::ObjectLit(e) => e.to_code(bank, tokens, options),

            ExprRef::UnaryOp(e) => e.to_code(bank, tokens, options),
            ExprRef::BinaryOp(e) => e.to_code(bank, tokens, options),
            ExprRef::Statement(e) => e.to_code(bank, tokens, options),
            ExprRef::Block(e) => e.to_code(bank, tokens, options),

            ExprRef::Function(e) => e.to_code(bank, tokens, options),
            ExprRef::Variable(e) => e.to_code(bank, tokens, options),

            ExprRef::Return(e) => e.to_code(bank, tokens, options),
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
            TypeRef::StructType(e) => e.to_code(bank, tokens, options),
            TypeRef::FunctionType(e) => e.to_code(bank, tokens, options),
            TypeRef::ManagedType(e) => e.to_code(bank, tokens, options),
            TypeRef::UnmanagedType(e) => e.to_code(bank, tokens, options),
            TypeRef::GenericType(e) => e.to_code(bank, tokens, options),
        }

        if has_parentheses {
            tokens.push(Token::Punct(Punct::RightParen));
        }
    }
}
