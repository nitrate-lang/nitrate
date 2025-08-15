use super::expression::{ExprOwned, TypeOwned};
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct ArrayType<'a> {
    element: Rc<TypeOwned<'a>>,
    count: Arc<ExprOwned<'a>>,
}

impl<'a> ArrayType<'a> {
    #[must_use]
    pub(crate) fn new(element: Rc<TypeOwned<'a>>, count: Arc<ExprOwned<'a>>) -> Self {
        ArrayType { element, count }
    }

    #[must_use]
    pub fn element(&self) -> Rc<TypeOwned<'a>> {
        self.element.clone()
    }

    #[must_use]
    pub fn count(&self) -> Arc<ExprOwned<'a>> {
        self.count.clone()
    }
}
