use super::expression::{Expr, Type};
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct ArrayType<'a> {
    element: Rc<Type<'a>>,
    count: Arc<Expr<'a>>,
}

impl<'a> ArrayType<'a> {
    #[must_use]
    pub(crate) fn new(element: Rc<Type<'a>>, count: Arc<Expr<'a>>) -> Self {
        ArrayType { element, count }
    }

    #[must_use]
    pub fn element(&self) -> Rc<Type<'a>> {
        self.element.clone()
    }

    #[must_use]
    pub fn count(&self) -> Arc<Expr<'a>> {
        self.count.clone()
    }
}
