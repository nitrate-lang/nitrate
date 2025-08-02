use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone)]
pub struct ArrayType<'a> {
    element_ty: TypeKey<'a>,
    count: ExprKey<'a>,
}

impl<'a> ArrayType<'a> {
    pub fn new(element_ty: TypeKey<'a>, count: ExprKey<'a>) -> Self {
        ArrayType { element_ty, count }
    }

    pub fn element_ty(&self) -> TypeKey<'a> {
        self.element_ty
    }

    pub fn count(&self) -> ExprKey<'a> {
        self.count
    }
}
