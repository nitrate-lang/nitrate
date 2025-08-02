use super::storage::{ExprRef, TypeRef};

#[derive(Debug, Clone)]
pub struct ArrayType<'a> {
    element_ty: TypeRef<'a>,
    count: ExprRef<'a>,
}

impl<'a> ArrayType<'a> {
    pub fn new(element_ty: TypeRef<'a>, count: ExprRef<'a>) -> Self {
        ArrayType { element_ty, count }
    }

    pub fn element_ty(&self) -> TypeRef<'a> {
        self.element_ty
    }

    pub fn count(&self) -> ExprRef<'a> {
        self.count
    }
}
