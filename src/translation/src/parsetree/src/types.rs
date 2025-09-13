use crate::kind::{Block, Expr, FunctionParameter, StructField};
use interned_string::IString;
use serde::{Deserialize, Serialize};

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
    args: Vec<(IString, Type)>,
}

impl GenericType {
    #[must_use]
    pub(crate) fn new(base: Type, args: Vec<(IString, Type)>) -> Self {
        GenericType { base, args }
    }

    #[must_use]
    pub fn base(&self) -> &Type {
        &self.base
    }

    #[must_use]
    pub fn arguments(&self) -> &[(IString, Type)] {
        &self.args
    }
}

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
    TypeName(IString),
    RefinementType(Box<RefinementType>),
    TupleType(Box<TupleType>),
    ArrayType(Box<ArrayType>),
    MapType(Box<MapType>),
    SliceType(Box<SliceType>),
    FunctionType(Box<FunctionType>),
    ManagedRefType(Box<ManagedRefType>),
    UnmanagedRefType(Box<UnmanagedRefType>),
    GenericType(Box<GenericType>),
    OpaqueType(IString),
    StructType(Box<StructType>),
    LatentType(Box<Block>),
    HasParenthesesType(Box<Type>),
}

impl Type {
    #[must_use]
    pub fn is_known(&self) -> bool {
        matches!(self, Type::InferType)
    }
}

impl std::fmt::Debug for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Type::Bool => write!(f, "bool"),
            Type::UInt8 => write!(f, "u8"),
            Type::UInt16 => write!(f, "u16"),
            Type::UInt32 => write!(f, "u32"),
            Type::UInt64 => write!(f, "u64"),
            Type::UInt128 => write!(f, "u128"),
            Type::Int8 => write!(f, "i8"),
            Type::Int16 => write!(f, "i16"),
            Type::Int32 => write!(f, "i32"),
            Type::Int64 => write!(f, "i64"),
            Type::Int128 => write!(f, "i128"),
            Type::Float8 => write!(f, "f8"),
            Type::Float16 => write!(f, "f16"),
            Type::Float32 => write!(f, "f32"),
            Type::Float64 => write!(f, "f64"),
            Type::Float128 => write!(f, "f128"),
            Type::UnitType => write!(f, "()"),
            Type::InferType => write!(f, "_"),
            Type::TypeName(e) => f.debug_struct("TypeName").field("name", &e).finish(),
            Type::RefinementType(e) => e.fmt(f),
            Type::TupleType(e) => e.fmt(f),
            Type::ArrayType(e) => e.fmt(f),
            Type::MapType(e) => e.fmt(f),
            Type::SliceType(e) => e.fmt(f),
            Type::FunctionType(e) => e.fmt(f),
            Type::ManagedRefType(e) => e.fmt(f),
            Type::UnmanagedRefType(e) => e.fmt(f),
            Type::GenericType(e) => e.fmt(f),
            Type::OpaqueType(e) => f.debug_struct("OpaqueType").field("name", e).finish(),
            Type::StructType(e) => e.fmt(f),
            Type::LatentType(e) => f.debug_struct("LatentType").field("type", e).finish(),
            Type::HasParenthesesType(e) => f.debug_struct("Parentheses").field("type", e).finish(),
        }
    }
}
