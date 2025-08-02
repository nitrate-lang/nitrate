use super::array_type::ArrayType;
use super::binary_op::BinaryExpr;
use super::block::Block;
use super::character::CharLit;
use super::function::Function;
use super::function_type::FunctionType;
use super::list::List;
use super::number::{FloatLit, IntegerLit};
use super::object::Object;
use super::returns::Return;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::UnaryExpr;
use super::variable::Variable;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub enum Expr<'a> {
    Discard,

    /* Primitive Expressions */
    Integer(IntegerLit),
    Float(FloatLit),
    String(StringLit<'a>),
    Char(CharLit),
    List(List<'a>),
    Object(Object<'a>),

    /* Compound Expressions */
    UnaryOp(UnaryExpr<'a>),
    BinaryOp(BinaryExpr<'a>),
    Statement(Statement<'a>),
    Block(Block<'a>),

    /* Definition */
    Function(Function<'a>),
    Variable(Variable<'a>),

    /* Control Flow */
    Return(Return<'a>),

    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    StructType(StructType<'a>),
    FunctionType(FunctionType<'a>),
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub enum RefExpr<'storage, 'a> {
    Discard,

    /* Primitive Expressions */
    Integer(&'storage IntegerLit),
    Float(&'storage FloatLit),
    String(&'storage StringLit<'a>),
    Char(&'storage CharLit),
    List(&'storage List<'a>),
    Object(&'storage Object<'a>),

    /* Compound Expressions */
    UnaryOp(&'storage UnaryExpr<'a>),
    BinaryOp(&'storage BinaryExpr<'a>),
    Statement(&'storage Statement<'a>),
    Block(&'storage Block<'a>),

    /* Definition */
    Function(&'storage Function<'a>),
    Variable(&'storage Variable<'a>),

    /* Control Flow */
    Return(&'storage Return<'a>),

    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    StructType(&'storage StructType<'a>),
    FunctionType(&'storage FunctionType<'a>),
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub enum Type<'a> {
    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    StructType(StructType<'a>),
    FunctionType(FunctionType<'a>),
}

impl<'a> TryInto<Type<'a>> for Expr<'a> {
    type Error = Self;

    fn try_into(self) -> Result<Type<'a>, Self::Error> {
        match self {
            Expr::Discard => Err(self),

            Expr::Integer(_) => Err(self),
            Expr::Float(_) => Err(self),
            Expr::String(_) => Err(self),
            Expr::Char(_) => Err(self),
            Expr::List(_) => Err(self),
            Expr::Object(_) => Err(self),

            Expr::UnaryOp(_) => Err(self),
            Expr::BinaryOp(_) => Err(self),
            Expr::Statement(_) => Err(self),
            Expr::Block(_) => Err(self),

            Expr::Function(_) => Err(self),
            Expr::Variable(_) => Err(self),

            Expr::Return(_) => Err(self),

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
            Expr::TupleType(tuple) => Ok(Type::TupleType(tuple)),
            Expr::ArrayType(array) => Ok(Type::ArrayType(array)),
            Expr::StructType(struct_type) => Ok(Type::StructType(struct_type)),
            Expr::FunctionType(function) => Ok(Type::FunctionType(function)),
        }
    }
}

impl<'a> Into<Expr<'a>> for Type<'a> {
    fn into(self) -> Expr<'a> {
        match self {
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
            Type::TupleType(tuple) => Expr::TupleType(tuple),
            Type::ArrayType(array) => Expr::ArrayType(array),
            Type::StructType(struct_type) => Expr::StructType(struct_type),
            Type::FunctionType(function) => Expr::FunctionType(function),
        }
    }
}
