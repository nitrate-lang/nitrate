use super::storage::TypeKey;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct TupleType<'a> {
    elements: Vec<TypeKey<'a>>,
}

impl<'a> TupleType<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<TypeKey<'a>>) -> Self {
        TupleType { elements }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<TypeKey<'a>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[TypeKey<'a>] {
        &self.elements
    }
}
