use super::expression::Type;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct TupleType<'a> {
    elements: Vec<Box<Type<'a>>>,
}

impl<'a> TupleType<'a> {
    pub fn new(elements: Vec<Box<Type<'a>>>) -> Self {
        TupleType { elements }
    }

    pub fn into_inner(self) -> Vec<Box<Type<'a>>> {
        self.elements
    }

    pub fn elements(&self) -> &[Box<Type<'a>>] {
        &self.elements
    }
}
