use crate::expression::{Expr, FunctionParameter, Identifier};
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::sync::Arc;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RefinementType {
    base: Type,
    width: Option<Expr>,
    min: Option<Expr>,
    max: Option<Expr>,
}

impl RefinementType {
    #[must_use]
    pub(crate) fn new(
        base: Type,
        width: Option<Expr>,
        min: Option<Expr>,
        max: Option<Expr>,
    ) -> Self {
        RefinementType {
            base,
            width,
            min,
            max,
        }
    }

    #[must_use]
    pub fn base(&self) -> &Type {
        &self.base
    }

    #[must_use]
    pub fn width(&self) -> Option<&Expr> {
        self.width.as_ref()
    }

    #[must_use]
    pub fn min(&self) -> Option<&Expr> {
        self.min.as_ref()
    }

    #[must_use]
    pub fn max(&self) -> Option<&Expr> {
        self.max.as_ref()
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TupleType {
    elements: Vec<Type>,
}

impl TupleType {
    #[must_use]
    pub(crate) fn new(elements: Vec<Type>) -> Self {
        TupleType { elements }
    }

    #[must_use]
    pub fn elements(&self) -> &[Type] {
        &self.elements
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ArrayType {
    element: Type,
    count: Expr,
}

impl ArrayType {
    #[must_use]
    pub(crate) fn new(element: Type, count: Expr) -> Self {
        ArrayType { element, count }
    }

    #[must_use]
    pub fn element(&self) -> &Type {
        &self.element
    }

    #[must_use]
    pub fn count(&self) -> &Expr {
        &self.count
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MapType {
    key: Type,
    value: Type,
}

impl MapType {
    #[must_use]
    pub(crate) fn new(key: Type, value: Type) -> Self {
        MapType { key, value }
    }

    #[must_use]
    pub fn key(&self) -> &Type {
        &self.key
    }

    #[must_use]
    pub fn value(&self) -> &Type {
        &self.value
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceType {
    element: Type,
}

impl SliceType {
    #[must_use]
    pub(crate) fn new(element: Type) -> Self {
        SliceType { element }
    }

    #[must_use]
    pub fn element(&self) -> &Type {
        &self.element
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FunctionType {
    parameters: Vec<FunctionParameter>,
    return_type: Type,
    attributes: Vec<Expr>,
}

impl FunctionType {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter>,
        return_type: Type,
        attributes: Vec<Expr>,
    ) -> Self {
        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    #[must_use]
    pub fn attributes(&self) -> &[Expr] {
        &self.attributes
    }

    #[must_use]
    pub fn parameters(&self) -> &[FunctionParameter] {
        &self.parameters
    }

    #[must_use]
    pub fn return_type(&self) -> &Type {
        &self.return_type
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ManagedRefType {
    target: Type,
    is_mutable: bool,
}

impl ManagedRefType {
    #[must_use]
    pub(crate) fn new(target: Type, is_mutable: bool) -> Self {
        ManagedRefType { target, is_mutable }
    }

    #[must_use]
    pub fn target(&self) -> &Type {
        &self.target
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnmanagedRefType {
    target: Type,
    is_mutable: bool,
}

impl UnmanagedRefType {
    #[must_use]
    pub(crate) fn new(target: Type, is_mutable: bool) -> Self {
        UnmanagedRefType { target, is_mutable }
    }

    #[must_use]
    pub fn target(&self) -> &Type {
        &self.target
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GenericType {
    base: Type,
    args: Vec<(IString, Expr)>,
}

impl GenericType {
    #[must_use]
    pub(crate) fn new(base: Type, args: Vec<(IString, Expr)>) -> Self {
        GenericType { base, args }
    }

    #[must_use]
    pub fn base(&self) -> &Type {
        &self.base
    }

    #[must_use]
    pub fn arguments(&self) -> &[(IString, Expr)] {
        &self.args
    }
}

pub type StructField = (IString, Type, Option<Expr>);

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StructType {
    fields: Vec<StructField>,
}

impl StructType {
    #[must_use]
    pub(crate) fn new(fields: Vec<StructField>) -> Self {
        StructType { fields }
    }

    #[must_use]
    pub fn fields(&self) -> &[StructField] {
        &self.fields
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Type {
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
    UnitType,
    InferType,
    TypeName(Arc<Identifier>),
    RefinementType(Arc<RefinementType>),
    TupleType(Arc<TupleType>),
    ArrayType(Arc<ArrayType>),
    MapType(Arc<MapType>),
    SliceType(Arc<SliceType>),
    FunctionType(Arc<FunctionType>),
    ManagedRefType(Arc<ManagedRefType>),
    UnmanagedRefType(Arc<UnmanagedRefType>),
    GenericType(Arc<GenericType>),
    OpaqueType(IString),
    StructType(Arc<StructType>),
    LatentType(Arc<Expr>),
    HasParenthesesType(Arc<Type>),
}

impl From<Type> for Expr {
    fn from(val: Type) -> Expr {
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
            Type::UnitType => Expr::UnitType,

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
            Type::StructType(x) => Expr::StructType(x),
            Type::LatentType(x) => Expr::LatentType(x),
            Type::HasParenthesesType(x) => Expr::HasParenthesesType(x),
        }
    }
}

impl Type {
    #[must_use]
    pub fn is_known(&self) -> bool {
        matches!(self, Type::InferType)
    }
}

impl std::fmt::Debug for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let expr: Expr = self.to_owned().into();
        expr.fmt(f)
    }
}
