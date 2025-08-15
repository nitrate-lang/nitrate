use super::expression::TypeOwned;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq)]
pub struct TupleType<'a> {
    elements: Vec<Rc<TypeOwned<'a>>>,
}

impl<'a> TupleType<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<Rc<TypeOwned<'a>>>) -> Self {
        TupleType { elements }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<Rc<TypeOwned<'a>>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[Rc<TypeOwned<'a>>] {
        &self.elements
    }
}
