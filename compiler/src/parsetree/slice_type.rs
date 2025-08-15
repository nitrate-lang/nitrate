use super::expression::Type;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq)]
pub struct SliceType<'a> {
    element: Rc<Type<'a>>,
}

impl<'a> SliceType<'a> {
    #[must_use]
    pub(crate) fn new(element: Rc<Type<'a>>) -> Self {
        SliceType { element }
    }

    #[must_use]
    pub fn element(&self) -> Rc<Type<'a>> {
        self.element.clone()
    }
}
