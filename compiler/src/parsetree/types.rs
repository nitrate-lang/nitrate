use super::array_type::ArrayType;
use super::expression::OriginTag;
use super::expression::{Expr, InnerExpr, Metadata};
use super::function_type::FunctionType;
use super::struct_type::StructType;
use super::tuple_type::TupleType;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub enum InnerType<'a> {
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
pub struct Type<'a> {
    expr: InnerType<'a>,
    has_parenthesis: bool,
}

impl<'a> Type<'a> {
    pub const fn new(expr: InnerType<'a>, has_parenthesis: bool) -> Self {
        Type {
            expr,
            has_parenthesis,
        }
    }

    pub fn into_inner(self) -> InnerType<'a> {
        self.expr
    }

    pub fn get(&self) -> &InnerType<'a> {
        &self.expr
    }

    pub fn has_parenthesis(&self) -> bool {
        self.has_parenthesis
    }

    pub fn into_expr(self) -> Expr<'a> {
        let has_parenthesis = self.has_parenthesis();

        let expr = match self.expr {
            InnerType::Bool => InnerExpr::Bool,
            InnerType::UInt8 => InnerExpr::UInt8,
            InnerType::UInt16 => InnerExpr::UInt16,
            InnerType::UInt32 => InnerExpr::UInt32,
            InnerType::UInt64 => InnerExpr::UInt64,
            InnerType::UInt128 => InnerExpr::UInt128,
            InnerType::Int8 => InnerExpr::Int8,
            InnerType::Int16 => InnerExpr::Int16,
            InnerType::Int32 => InnerExpr::Int32,
            InnerType::Int64 => InnerExpr::Int64,
            InnerType::Int128 => InnerExpr::Int128,
            InnerType::Float8 => InnerExpr::Float8,
            InnerType::Float16 => InnerExpr::Float16,
            InnerType::Float32 => InnerExpr::Float32,
            InnerType::Float64 => InnerExpr::Float64,
            InnerType::Float128 => InnerExpr::Float128,

            InnerType::InferType => InnerExpr::InferType,
            InnerType::TupleType(tuple) => InnerExpr::TupleType(tuple),
            InnerType::ArrayType(array) => InnerExpr::ArrayType(array),
            InnerType::StructType(struct_type) => InnerExpr::StructType(struct_type),
            InnerType::FunctionType(function) => InnerExpr::FunctionType(function),
        };

        Expr::new(expr, Metadata::new(OriginTag::default(), has_parenthesis))
    }

    pub fn is_lit(&self) -> bool {
        match &self.expr {
            InnerType::Bool => true,
            InnerType::UInt8 => true,
            InnerType::UInt16 => true,
            InnerType::UInt32 => true,
            InnerType::UInt64 => true,
            InnerType::UInt128 => true,
            InnerType::Int8 => true,
            InnerType::Int16 => true,
            InnerType::Int32 => true,
            InnerType::Int64 => true,
            InnerType::Int128 => true,
            InnerType::Float8 => true,
            InnerType::Float16 => true,
            InnerType::Float32 => true,
            InnerType::Float64 => true,
            InnerType::Float128 => true,

            InnerType::InferType => false,
            InnerType::TupleType(tuple) => tuple.elements().iter().all(|item| item.is_lit()),
            InnerType::ArrayType(array) => array.element_ty().is_lit() && array.count().is_lit(),
            InnerType::StructType(_struct) => _struct
                .fields()
                .iter()
                .all(|(_, field_ty)| field_ty.is_lit()),
            InnerType::FunctionType(function) => {
                function.parameters().iter().all(|(_, ty, default)| {
                    ty.as_ref().map_or(true, |f| f.is_lit())
                        && default.as_ref().map_or(true, |d| d.is_lit())
                }) && function.return_type().map_or(true, |ty| ty.is_lit())
                    && function.attributes().iter().all(|attr| attr.is_lit())
            }
        }
    }
}

impl<'a> std::ops::Deref for Type<'a> {
    type Target = InnerType<'a>;

    fn deref(&self) -> &Self::Target {
        &self.expr
    }
}
