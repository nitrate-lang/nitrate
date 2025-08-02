use super::storage::TypeRef;

#[derive(Debug, Clone)]
pub struct TupleType<'a> {
    elements: Vec<TypeRef<'a>>,
}

impl<'a> TupleType<'a> {
    pub fn new(elements: Vec<TypeRef<'a>>) -> Self {
        TupleType { elements }
    }

    pub fn into_inner(self) -> Vec<TypeRef<'a>> {
        self.elements
    }

    pub fn elements(&self) -> &[TypeRef<'a>] {
        &self.elements
    }
}
