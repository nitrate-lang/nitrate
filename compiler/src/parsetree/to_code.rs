use super::array_type::ArrayType;
use super::binary_op::{BinaryOp, BinaryOperator};
use super::block::Block;
use super::character::CharLit;
use super::expression::{ExprRef, TypeRef};
use super::function::Function;
use super::function_type::FunctionType;
use super::list::ListLit;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::returns::Return;
use super::statement::Statement;
use super::storage::{ExprKey, Storage, TypeKey};
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::{UnaryOp, UnaryOperator};
use super::variable::{Variable, VariableKind};
use crate::lexer::{
    Float, Identifier, Integer, Keyword, Operator, Punctuation, StringLit as StringLitToken, Token,
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
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (i, expr) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
            expr.to_code(bank, tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

impl<'a> ToCode<'a> for ObjectLit<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (key, value) in self.get() {
            tokens.push(Token::Identifier(Identifier::new(key)));
            tokens.push(Token::Punctuation(Punctuation::Colon));

            value.to_code(bank, tokens, options);
            tokens.push(Token::Punctuation(Punctuation::Comma));
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

impl<'a> ToCode<'a> for UnaryOperator {
    fn to_code(&self, _bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let operator = Token::Operator(match self {
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
        let operator = Token::Operator(match self {
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
        tokens.push(Token::Punctuation(Punctuation::Semicolon));
    }
}

impl<'a> ToCode<'a> for Block<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBrace));
        for expr in self.elements() {
            expr.to_code(bank, tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBrace));
    }
}

impl<'a> ToCode<'a> for Function<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punctuation(Punctuation::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
                attr.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punctuation(Punctuation::RightBracket));
        }

        if !self.name().is_empty() {
            tokens.push(Token::Identifier(Identifier::new(self.name())));
        }

        tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));

            tokens.push(Token::Identifier(Identifier::new(name)));

            if let Some(ty) = ty {
                if !matches!(ty.get(bank), TypeRef::InferType) {
                    tokens.push(Token::Punctuation(Punctuation::Colon));
                    ty.to_code(bank, tokens, options);
                }
            }

            if let Some(default_expr) = default {
                tokens.push(Token::Operator(Operator::Set));
                default_expr.to_code(bank, tokens, options);
            }
        }
        tokens.push(Token::Punctuation(Punctuation::RightParenthesis));

        if let Some(return_type) = self.return_type() {
            if !matches!(return_type.get(bank), TypeRef::InferType) {
                tokens.push(Token::Punctuation(Punctuation::Colon));
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

        tokens.push(Token::Identifier(Identifier::new(self.name())));

        if let Some(var_type) = self.get_type() {
            tokens.push(Token::Punctuation(Punctuation::Colon));
            var_type.to_code(bank, tokens, options);
        }

        if let Some(value) = self.value() {
            tokens.push(Token::Operator(Operator::Set));
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

impl<'a> ToCode<'a> for TupleType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (i, ty) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
            ty.to_code(bank, tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

impl<'a> ToCode<'a> for ArrayType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        self.element_ty().to_code(bank, tokens, options);
        tokens.push(Token::Punctuation(Punctuation::Semicolon));
        self.count().to_code(bank, tokens, options);
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

impl<'a> ToCode<'a> for StructType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Struct));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punctuation(Punctuation::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i != 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
                attr.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punctuation(Punctuation::RightBracket));
        }

        if let Some(name) = self.name() {
            tokens.push(Token::Identifier(Identifier::new(name)));
        }

        tokens.push(Token::Punctuation(Punctuation::LeftBrace));
        for (field_name, field_ty) in self.fields() {
            tokens.push(Token::Identifier(Identifier::new(field_name)));
            tokens.push(Token::Punctuation(Punctuation::Colon));
            field_ty.to_code(bank, tokens, options);
            tokens.push(Token::Punctuation(Punctuation::Comma));
        }
        tokens.push(Token::Punctuation(Punctuation::RightBrace));
    }
}

impl<'a> ToCode<'a> for FunctionType<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punctuation(Punctuation::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
                attr.to_code(bank, tokens, options);
            }
            tokens.push(Token::Punctuation(Punctuation::RightBracket));
        }

        tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));

            tokens.push(Token::Identifier(Identifier::new(name)));

            if let Some(ty) = ty {
                if !matches!(ty.get(bank), TypeRef::InferType) {
                    tokens.push(Token::Punctuation(Punctuation::Colon));
                    ty.to_code(bank, tokens, options);
                }
            }

            if let Some(default_expr) = default {
                tokens.push(Token::Operator(Operator::Set));
                default_expr.to_code(bank, tokens, options);
            }
        }
        tokens.push(Token::Punctuation(Punctuation::RightParenthesis));

        if let Some(return_type) = self.return_type() {
            if !matches!(return_type.get(bank), TypeRef::InferType) {
                tokens.push(Token::Punctuation(Punctuation::Colon));
                return_type.to_code(bank, tokens, options);
            }
        }
    }
}

impl<'a> ToCode<'a> for ExprKey<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        let expr = self.get(bank);

        if expr.is_discard() {
            return;
        }

        // if self.has_parenthesis() {
        //     tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        // }
        // FIXME: Put parenthesis handling back if needed

        match expr {
            ExprRef::Bool => tokens.push(Token::Identifier(Identifier::new("bool"))),
            ExprRef::UInt8 => tokens.push(Token::Identifier(Identifier::new("u8"))),
            ExprRef::UInt16 => tokens.push(Token::Identifier(Identifier::new("u16"))),
            ExprRef::UInt32 => tokens.push(Token::Identifier(Identifier::new("u32"))),
            ExprRef::UInt64 => tokens.push(Token::Identifier(Identifier::new("u64"))),
            ExprRef::UInt128 => tokens.push(Token::Identifier(Identifier::new("u128"))),
            ExprRef::Int8 => tokens.push(Token::Identifier(Identifier::new("i8"))),
            ExprRef::Int16 => tokens.push(Token::Identifier(Identifier::new("i16"))),
            ExprRef::Int32 => tokens.push(Token::Identifier(Identifier::new("i32"))),
            ExprRef::Int64 => tokens.push(Token::Identifier(Identifier::new("i64"))),
            ExprRef::Int128 => tokens.push(Token::Identifier(Identifier::new("i128"))),
            ExprRef::Float8 => tokens.push(Token::Identifier(Identifier::new("f8"))),
            ExprRef::Float16 => tokens.push(Token::Identifier(Identifier::new("f16"))),
            ExprRef::Float32 => tokens.push(Token::Identifier(Identifier::new("f32"))),
            ExprRef::Float64 => tokens.push(Token::Identifier(Identifier::new("f64"))),
            ExprRef::Float128 => tokens.push(Token::Identifier(Identifier::new("f128"))),

            ExprRef::InferType => tokens.push(Token::Identifier(Identifier::new("_"))),
            ExprRef::TupleType(e) => e.to_code(bank, tokens, options),
            ExprRef::ArrayType(e) => e.to_code(bank, tokens, options),
            ExprRef::StructType(e) => e.to_code(bank, tokens, options),
            ExprRef::FunctionType(e) => e.to_code(bank, tokens, options),

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

        // if self.has_parenthesis() {
        //     tokens.push(Token::Punctuation(Punctuation::RightParenthesis));
        // }
    }
}

impl<'a> ToCode<'a> for TypeKey<'a> {
    fn to_code(&self, bank: &Storage<'a>, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        // if self.has_parenthesis() {
        //     tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        // }
        // FIXME: Put parenthesis handling back if needed

        match self.get(bank) {
            TypeRef::Bool => tokens.push(Token::Identifier(Identifier::new("bool"))),
            TypeRef::UInt8 => tokens.push(Token::Identifier(Identifier::new("u8"))),
            TypeRef::UInt16 => tokens.push(Token::Identifier(Identifier::new("u16"))),
            TypeRef::UInt32 => tokens.push(Token::Identifier(Identifier::new("u32"))),
            TypeRef::UInt64 => tokens.push(Token::Identifier(Identifier::new("u64"))),
            TypeRef::UInt128 => tokens.push(Token::Identifier(Identifier::new("u128"))),
            TypeRef::Int8 => tokens.push(Token::Identifier(Identifier::new("i8"))),
            TypeRef::Int16 => tokens.push(Token::Identifier(Identifier::new("i16"))),
            TypeRef::Int32 => tokens.push(Token::Identifier(Identifier::new("i32"))),
            TypeRef::Int64 => tokens.push(Token::Identifier(Identifier::new("i64"))),
            TypeRef::Int128 => tokens.push(Token::Identifier(Identifier::new("i128"))),
            TypeRef::Float8 => tokens.push(Token::Identifier(Identifier::new("f8"))),
            TypeRef::Float16 => tokens.push(Token::Identifier(Identifier::new("f16"))),
            TypeRef::Float32 => tokens.push(Token::Identifier(Identifier::new("f32"))),
            TypeRef::Float64 => tokens.push(Token::Identifier(Identifier::new("f64"))),
            TypeRef::Float128 => tokens.push(Token::Identifier(Identifier::new("f128"))),

            TypeRef::InferType => tokens.push(Token::Identifier(Identifier::new("_"))),
            TypeRef::TupleType(e) => e.to_code(bank, tokens, options),
            TypeRef::ArrayType(e) => e.to_code(bank, tokens, options),
            TypeRef::StructType(e) => e.to_code(bank, tokens, options),
            TypeRef::FunctionType(e) => e.to_code(bank, tokens, options),
        }

        // if self.has_parenthesis() {
        //     tokens.push(Token::Punctuation(Punctuation::RightParenthesis));
        // }
    }
}
