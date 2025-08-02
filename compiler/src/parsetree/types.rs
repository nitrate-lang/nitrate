use super::array_type::ArrayType;
use super::expression::Expr;
use super::function_type::FunctionType;
use super::struct_type::StructType;
use super::tuple_type::TupleType;

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
