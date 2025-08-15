use super::expression::Type;

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
