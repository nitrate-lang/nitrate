use super::expression::Type;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq)]
pub struct TupleType<'a> {
    elements: Vec<Rc<Type<'a>>>,
}

impl<'a> TupleType<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<Rc<Type<'a>>>) -> Self {
        TupleType { elements }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<Rc<Type<'a>>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[Rc<Type<'a>>] {
        &self.elements
    }
}
