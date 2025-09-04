use super::expression::FunctionParameter;
use super::node::{Expr, Type};

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
