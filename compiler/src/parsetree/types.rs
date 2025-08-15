use super::function::FunctionParameter;
use super::node::{Expr, Type};
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct RefinementType<'a> {
    base: Type<'a>,
    width: Option<Arc<Expr<'a>>>,
    min: Option<Arc<Expr<'a>>>,
    max: Option<Arc<Expr<'a>>>,
}

impl<'a> RefinementType<'a> {
    #[must_use]
    pub(crate) fn new(
        base: Type<'a>,
        width: Option<Arc<Expr<'a>>>,
        min: Option<Arc<Expr<'a>>>,
        max: Option<Arc<Expr<'a>>>,
    ) -> Self {
        RefinementType {
            base,
            width,
            min,
            max,
        }
    }

    #[must_use]
    pub fn base(&self) -> Type<'a> {
        self.base.clone()
    }

    #[must_use]
    pub fn width(&self) -> Option<Arc<Expr<'a>>> {
        self.width.clone()
    }

    #[must_use]
    pub fn min(&self) -> Option<Arc<Expr<'a>>> {
        self.min.clone()
    }

    #[must_use]
    pub fn max(&self) -> Option<Arc<Expr<'a>>> {
        self.max.clone()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct TupleType<'a> {
    elements: Vec<Type<'a>>,
}

impl<'a> TupleType<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<Type<'a>>) -> Self {
        TupleType { elements }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<Type<'a>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[Type<'a>] {
        &self.elements
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct ArrayType<'a> {
    element: Type<'a>,
    count: Arc<Expr<'a>>,
}

impl<'a> ArrayType<'a> {
    #[must_use]
    pub(crate) fn new(element: Type<'a>, count: Arc<Expr<'a>>) -> Self {
        ArrayType { element, count }
    }

    #[must_use]
    pub fn element(&self) -> Type<'a> {
        self.element.clone()
    }

    #[must_use]
    pub fn count(&self) -> Arc<Expr<'a>> {
        self.count.clone()
    }
}

#[derive(Debug, Clone, PartialEq)]
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
    pub fn key(&self) -> Type<'a> {
        self.key.clone()
    }

    #[must_use]
    pub fn value(&self) -> Type<'a> {
        self.value.clone()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct SliceType<'a> {
    element: Type<'a>,
}

impl<'a> SliceType<'a> {
    #[must_use]
    pub(crate) fn new(element: Type<'a>) -> Self {
        SliceType { element }
    }

    #[must_use]
    pub fn element(&self) -> Type<'a> {
        self.element.clone()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Type<'a>,
    attributes: Vec<Arc<Expr<'a>>>,
}

impl<'a> FunctionType<'a> {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Type<'a>,
        attributes: Vec<Arc<Expr<'a>>>,
    ) -> Self {
        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    #[must_use]
    pub fn parameters(&self) -> &[FunctionParameter<'a>] {
        &self.parameters
    }

    #[must_use]
    pub fn return_type(&self) -> Type<'a> {
        self.return_type.clone()
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<Expr<'a>>] {
        &self.attributes
    }
}

#[derive(Debug, Clone, PartialEq)]
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
    pub fn target(&self) -> Type<'a> {
        self.target.clone()
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, PartialEq)]
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
    pub fn target(&self) -> Type<'a> {
        self.target.clone()
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct GenericType<'a> {
    base: Type<'a>,
    args: Vec<(&'a str, Arc<Expr<'a>>)>,
}

impl<'a> GenericType<'a> {
    #[must_use]
    pub(crate) fn new(base: Type<'a>, args: Vec<(&'a str, Arc<Expr<'a>>)>) -> Self {
        GenericType { base, args }
    }

    #[must_use]
    pub fn base(&self) -> Type<'a> {
        self.base.clone()
    }

    #[must_use]
    pub fn arguments(&self) -> &[(&'a str, Arc<Expr<'a>>)] {
        &self.args
    }
}
