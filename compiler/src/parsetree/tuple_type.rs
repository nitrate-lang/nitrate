use super::storage::TypeKey;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct TupleType<'a> {
    elements: Vec<TypeKey<'a>>,
}

impl<'a> TupleType<'a> {
    pub(crate) fn new(elements: Vec<TypeKey<'a>>) -> Self {
        TupleType { elements }
    }

    pub fn into_inner(self) -> Vec<TypeKey<'a>> {
        self.elements
    }

    pub fn elements(&self) -> &[TypeKey<'a>] {
        &self.elements
    }
}
