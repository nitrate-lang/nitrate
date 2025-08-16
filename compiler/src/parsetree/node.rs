use ordered_float::NotNan;

use super::expression::{
    Assert, Await, BinExpr, Block, Break, Continue, DoWhileLoop, ForEach, Function, If, IntegerLit,
    List, Object, Return, Scope, Statement, Switch, UnaryExpr, Variable, WhileLoop,
};
use super::types::{
    ArrayType, FunctionType, GenericType, ManagedRefType, MapType, RefinementType, SliceType,
    TupleType, UnmanagedRefType,
};
use crate::lexical::{BStringData, StringData};
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Expr<'a> {
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float8,
    Float16,
    Float32,
    Float64,
    Float128,
    InferType,
    TypeName(&'a str),
    RefinementType(Rc<RefinementType<'a>>),
    TupleType(Rc<TupleType<'a>>),
    ArrayType(Rc<ArrayType<'a>>),
    MapType(Rc<MapType<'a>>),
    SliceType(Rc<SliceType<'a>>),
    FunctionType(Rc<FunctionType<'a>>),
    ManagedRefType(Rc<ManagedRefType<'a>>),
    UnmanagedRefType(Rc<UnmanagedRefType<'a>>),
    GenericType(Rc<GenericType<'a>>),
    OpaqueType(Rc<StringData<'a>>),
    HasParenthesesType(Rc<Type<'a>>),

    Discard,
    HasParentheses(Rc<Expr<'a>>),

    BooleanLit(bool),
    IntegerLit(Rc<IntegerLit>),
    FloatLit(NotNan<f64>),
    StringLit(Rc<StringData<'a>>),
    BStringLit(Rc<BStringData<'a>>),

    List(Rc<List<'a>>),
    Object(Rc<Object<'a>>),
    UnaryExpr(Rc<UnaryExpr<'a>>),
    BinExpr(Rc<BinExpr<'a>>),
    Statement(Rc<Statement<'a>>),
    Block(Rc<Block<'a>>),

    Function(Rc<Function<'a>>),
    Variable(Rc<Variable<'a>>),
    Identifier(Rc<&'a str>),
    Scope(Rc<Scope<'a>>),

    If(Rc<If<'a>>),
    WhileLoop(Rc<WhileLoop<'a>>),
    DoWhileLoop(Rc<DoWhileLoop<'a>>),
    Switch(Rc<Switch<'a>>),
    Break(Rc<Break<'a>>),
    Continue(Rc<Continue<'a>>),
    Return(Rc<Return<'a>>),
    ForEach(Rc<ForEach<'a>>),
    Await(Rc<Await<'a>>),
    Assert(Rc<Assert<'a>>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Type<'a> {
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float8,
    Float16,
    Float32,
    Float64,
    Float128,

    InferType,
    TypeName(&'a str),
    RefinementType(Rc<RefinementType<'a>>),
    TupleType(Rc<TupleType<'a>>),
    ArrayType(Rc<ArrayType<'a>>),
    MapType(Rc<MapType<'a>>),
    SliceType(Rc<SliceType<'a>>),
    FunctionType(Rc<FunctionType<'a>>),
    ManagedRefType(Rc<ManagedRefType<'a>>),
    UnmanagedRefType(Rc<UnmanagedRefType<'a>>),
    GenericType(Rc<GenericType<'a>>),
    OpaqueType(Rc<StringData<'a>>),

    HasParenthesesType(Rc<Type<'a>>),
}

impl<'a> TryInto<Type<'a>> for Expr<'a> {
    type Error = Self;

    fn try_into(self) -> Result<Type<'a>, Self::Error> {
        match self {
            Expr::Bool => Ok(Type::Bool),
            Expr::UInt8 => Ok(Type::UInt8),
            Expr::UInt16 => Ok(Type::UInt16),
            Expr::UInt32 => Ok(Type::UInt32),
            Expr::UInt64 => Ok(Type::UInt64),
            Expr::UInt128 => Ok(Type::UInt128),
            Expr::Int8 => Ok(Type::Int8),
            Expr::Int16 => Ok(Type::Int16),
            Expr::Int32 => Ok(Type::Int32),
            Expr::Int64 => Ok(Type::Int64),
            Expr::Int128 => Ok(Type::Int128),
            Expr::Float8 => Ok(Type::Float8),
            Expr::Float16 => Ok(Type::Float16),
            Expr::Float32 => Ok(Type::Float32),
            Expr::Float64 => Ok(Type::Float64),
            Expr::Float128 => Ok(Type::Float128),

            Expr::InferType => Ok(Type::InferType),
            Expr::TypeName(x) => Ok(Type::TypeName(x)),
            Expr::RefinementType(x) => Ok(Type::RefinementType(x)),
            Expr::TupleType(x) => Ok(Type::TupleType(x)),
            Expr::ArrayType(x) => Ok(Type::ArrayType(x)),
            Expr::MapType(x) => Ok(Type::MapType(x)),
            Expr::SliceType(x) => Ok(Type::SliceType(x)),
            Expr::FunctionType(x) => Ok(Type::FunctionType(x)),
            Expr::ManagedRefType(x) => Ok(Type::ManagedRefType(x)),
            Expr::UnmanagedRefType(x) => Ok(Type::UnmanagedRefType(x)),
            Expr::GenericType(x) => Ok(Type::GenericType(x)),
            Expr::OpaqueType(x) => Ok(Type::OpaqueType(x)),
            Expr::HasParenthesesType(x) => Ok(Type::HasParenthesesType(x)),

            Expr::Discard
            | Expr::HasParentheses(_)
            | Expr::BooleanLit(_)
            | Expr::IntegerLit(_)
            | Expr::FloatLit(_)
            | Expr::StringLit(_)
            | Expr::BStringLit(_)
            | Expr::List(_)
            | Expr::Object(_)
            | Expr::UnaryExpr(_)
            | Expr::BinExpr(_)
            | Expr::Statement(_)
            | Expr::Block(_)
            | Expr::Function(_)
            | Expr::Variable(_)
            | Expr::Identifier(_)
            | Expr::Scope(_)
            | Expr::If(_)
            | Expr::WhileLoop(_)
            | Expr::DoWhileLoop(_)
            | Expr::Switch(_)
            | Expr::Break(_)
            | Expr::Continue(_)
            | Expr::Return(_)
            | Expr::ForEach(_)
            | Expr::Await(_)
            | Expr::Assert(_) => Err(self),
        }
    }
}

impl<'a> From<Type<'a>> for Expr<'a> {
    fn from(val: Type<'a>) -> Expr<'a> {
        match val {
            Type::Bool => Expr::Bool,
            Type::UInt8 => Expr::UInt8,
            Type::UInt16 => Expr::UInt16,
            Type::UInt32 => Expr::UInt32,
            Type::UInt64 => Expr::UInt64,
            Type::UInt128 => Expr::UInt128,
            Type::Int8 => Expr::Int8,
            Type::Int16 => Expr::Int16,
            Type::Int32 => Expr::Int32,
            Type::Int64 => Expr::Int64,
            Type::Int128 => Expr::Int128,
            Type::Float8 => Expr::Float8,
            Type::Float16 => Expr::Float16,
            Type::Float32 => Expr::Float32,
            Type::Float64 => Expr::Float64,
            Type::Float128 => Expr::Float128,

            Type::InferType => Expr::InferType,
            Type::TypeName(x) => Expr::TypeName(x),
            Type::RefinementType(x) => Expr::RefinementType(x),
            Type::TupleType(x) => Expr::TupleType(x),
            Type::ArrayType(x) => Expr::ArrayType(x),
            Type::MapType(x) => Expr::MapType(x),
            Type::SliceType(x) => Expr::SliceType(x),
            Type::FunctionType(x) => Expr::FunctionType(x),
            Type::ManagedRefType(x) => Expr::ManagedRefType(x),
            Type::UnmanagedRefType(x) => Expr::UnmanagedRefType(x),
            Type::GenericType(x) => Expr::GenericType(x),
            Type::OpaqueType(x) => Expr::OpaqueType(x),
            Type::HasParenthesesType(x) => Expr::HasParenthesesType(x),
        }
    }
}

impl Expr<'_> {
    #[must_use]
    pub fn is_discard(&self) -> bool {
        matches!(self, Expr::Discard)
    }
}

impl Type<'_> {
    #[must_use]
    pub fn is_known(&self) -> bool {
        matches!(self, Type::InferType)
    }
}
