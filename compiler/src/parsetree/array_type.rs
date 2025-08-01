use super::expression::Expr;
use super::types::Type;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct ArrayType<'a> {
    element_ty: Arc<Type<'a>>,
    count: Box<Expr<'a>>,
}

impl<'a> ArrayType<'a> {
    pub fn new(element_ty: Arc<Type<'a>>, count: Box<Expr<'a>>) -> Self {
        ArrayType { element_ty, count }
    }

    pub fn element_ty(&self) -> &Type<'a> {
        &self.element_ty
    }

    pub fn count(&self) -> &Expr<'a> {
        &self.count
    }
}
