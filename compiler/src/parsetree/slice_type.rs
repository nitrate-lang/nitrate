use super::expression::Type;

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
