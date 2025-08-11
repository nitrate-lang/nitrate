use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ArrayType<'a> {
    element: TypeKey<'a>,
    count: ExprKey<'a>,
}

impl<'a> ArrayType<'a> {
    pub(crate) fn new(element: TypeKey<'a>, count: ExprKey<'a>) -> Self {
        ArrayType { element, count }
    }

    #[must_use]
    pub fn element(&self) -> TypeKey<'a> {
        self.element
    }

    #[must_use]
    pub fn count(&self) -> ExprKey<'a> {
        self.count
    }
}
