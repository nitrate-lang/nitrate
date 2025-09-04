use crate::expression::{Expr, FunctionParameter, Identifier};
use nitrate_lexical::StringData;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct RefinementType<'a> {
    base: Type<'a>,
    width: Option<Expr<'a>>,
    min: Option<Expr<'a>>,
    max: Option<Expr<'a>>,
}

impl<'a> RefinementType<'a> {
    #[must_use]
    pub(crate) fn new(
        base: Type<'a>,
        width: Option<Expr<'a>>,
        min: Option<Expr<'a>>,
        max: Option<Expr<'a>>,
    ) -> Self {
        RefinementType {
            base,
            width,
            min,
            max,
        }
    }

    #[must_use]
    pub fn base(&self) -> &Type<'a> {
        &self.base
    }

    #[must_use]
    pub fn width(&self) -> Option<&Expr<'a>> {
        self.width.as_ref()
    }

    #[must_use]
    pub fn min(&self) -> Option<&Expr<'a>> {
        self.min.as_ref()
    }

    #[must_use]
    pub fn max(&self) -> Option<&Expr<'a>> {
        self.max.as_ref()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct TupleType<'a> {
    elements: Vec<Type<'a>>,
}

impl<'a> TupleType<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<Type<'a>>) -> Self {
        TupleType { elements }
    }

    #[must_use]
    pub fn elements(&self) -> &[Type<'a>] {
        &self.elements
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ArrayType<'a> {
    element: Type<'a>,
    count: Expr<'a>,
}

impl<'a> ArrayType<'a> {
    #[must_use]
    pub(crate) fn new(element: Type<'a>, count: Expr<'a>) -> Self {
        ArrayType { element, count }
    }

    #[must_use]
    pub fn element(&self) -> &Type<'a> {
        &self.element
    }

    #[must_use]
    pub fn count(&self) -> &Expr<'a> {
        &self.count
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct MapType<'a> {
    key: Type<'a>,
    value: Type<'a>,
}

impl<'a> MapType<'a> {
    #[must_use]
    pub(crate) fn new(key: Type<'a>, value: Type<'a>) -> Self {
        MapType { key, value }
    }

    #[must_use]
    pub fn key(&self) -> &Type<'a> {
        &self.key
    }

    #[must_use]
    pub fn value(&self) -> &Type<'a> {
        &self.value
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct SliceType<'a> {
    element: Type<'a>,
}

impl<'a> SliceType<'a> {
    #[must_use]
    pub(crate) fn new(element: Type<'a>) -> Self {
        SliceType { element }
    }

    #[must_use]
    pub fn element(&self) -> &Type<'a> {
        &self.element
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Type<'a>,
    attributes: Vec<Expr<'a>>,
}

impl<'a> FunctionType<'a> {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Type<'a>,
        attributes: Vec<Expr<'a>>,
    ) -> Self {
        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    #[must_use]
    pub fn attributes(&self) -> &[Expr<'a>] {
        &self.attributes
    }

    #[must_use]
    pub fn parameters(&self) -> &[FunctionParameter<'a>] {
        &self.parameters
    }

    #[must_use]
    pub fn return_type(&self) -> &Type<'a> {
        &self.return_type
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ManagedRefType<'a> {
    target: Type<'a>,
    is_mutable: bool,
}

impl<'a> ManagedRefType<'a> {
    #[must_use]
    pub(crate) fn new(target: Type<'a>, is_mutable: bool) -> Self {
        ManagedRefType { target, is_mutable }
    }

    #[must_use]
    pub fn target(&self) -> &Type<'a> {
        &self.target
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct UnmanagedRefType<'a> {
    target: Type<'a>,
    is_mutable: bool,
}

impl<'a> UnmanagedRefType<'a> {
    #[must_use]
    pub(crate) fn new(target: Type<'a>, is_mutable: bool) -> Self {
        UnmanagedRefType { target, is_mutable }
    }

    #[must_use]
    pub fn target(&self) -> &Type<'a> {
        &self.target
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct GenericType<'a> {
    base: Type<'a>,
    args: Vec<(&'a str, Expr<'a>)>,
}

impl<'a> GenericType<'a> {
    #[must_use]
    pub(crate) fn new(base: Type<'a>, args: Vec<(&'a str, Expr<'a>)>) -> Self {
        GenericType { base, args }
    }

    #[must_use]
    pub fn base(&self) -> &Type<'a> {
        &self.base
    }

    #[must_use]
    pub fn arguments(&self) -> &[(&'a str, Expr<'a>)] {
        &self.args
    }
}

pub type StructField<'a> = (&'a str, Type<'a>, Option<Expr<'a>>);

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct StructType<'a> {
    fields: Vec<StructField<'a>>,
}

impl<'a> StructType<'a> {
    #[must_use]
    pub(crate) fn new(fields: Vec<StructField<'a>>) -> Self {
        StructType { fields }
    }

    #[must_use]
    pub fn fields(&self) -> &[StructField<'a>] {
        &self.fields
    }
}

#[derive(Clone, PartialEq, Eq, Hash)]
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
    UnitType,
    InferType,
    TypeName(Rc<Identifier<'a>>),
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
    StructType(Rc<StructType<'a>>),
    LatentType(Rc<Expr<'a>>),
    HasParenthesesType(Rc<Type<'a>>),
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

impl Type<'_> {
    #[must_use]
    pub fn is_known(&self) -> bool {
        matches!(self, Type::InferType)
    }
}

impl<'a> std::fmt::Debug for Type<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let expr: Expr<'a> = self.to_owned().into();
        expr.fmt(f)
    }
}
