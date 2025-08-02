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

impl<'a> Type<'a> {
    pub fn into_expr(self) -> Expr<'a> {
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

    pub fn is_lit(&self) -> bool {
        match self {
            Type::Bool => true,
            Type::UInt8 => true,
            Type::UInt16 => true,
            Type::UInt32 => true,
            Type::UInt64 => true,
            Type::UInt128 => true,
            Type::Int8 => true,
            Type::Int16 => true,
            Type::Int32 => true,
            Type::Int64 => true,
            Type::Int128 => true,
            Type::Float8 => true,
            Type::Float16 => true,
            Type::Float32 => true,
            Type::Float64 => true,
            Type::Float128 => true,

            Type::InferType => false,
            Type::TupleType(tuple) => tuple.elements().iter().all(|item| item.is_lit()),
            Type::ArrayType(array) => array.element_ty().is_lit() && array.count().is_lit(),
            Type::StructType(_struct) => _struct
                .fields()
                .iter()
                .all(|(_, field_ty)| field_ty.is_lit()),
            Type::FunctionType(function) => {
                function.parameters().iter().all(|(_, ty, default)| {
                    ty.as_ref().map_or(true, |f| f.is_lit())
                        && default.as_ref().map_or(true, |d| d.is_lit())
                }) && function.return_type().map_or(true, |ty| ty.is_lit())
                    && function.attributes().iter().all(|attr| attr.is_lit())
            }
        }
    }
}
