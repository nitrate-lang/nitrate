use super::types::Type;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct TupleType<'a> {
    elements: Vec<Arc<Type<'a>>>,
}

impl<'a> TupleType<'a> {
    pub fn new(elements: Vec<Arc<Type<'a>>>) -> Self {
        TupleType { elements }
    }

    pub fn into_inner(self) -> Vec<Arc<Type<'a>>> {
        self.elements
    }

    pub fn elements(&self) -> &[Arc<Type<'a>>] {
        &self.elements
    }
}
