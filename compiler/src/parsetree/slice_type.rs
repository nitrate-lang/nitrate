use super::expression::TypeOwned;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq)]
pub struct SliceType<'a> {
    element: Rc<TypeOwned<'a>>,
}

impl<'a> SliceType<'a> {
    #[must_use]
    pub(crate) fn new(element: Rc<TypeOwned<'a>>) -> Self {
        SliceType { element }
    }

    #[must_use]
    pub fn element(&self) -> Rc<TypeOwned<'a>> {
        self.element.clone()
    }
}
