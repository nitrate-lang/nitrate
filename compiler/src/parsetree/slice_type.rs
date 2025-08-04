use super::storage::TypeKey;

#[derive(Debug, Clone)]
pub struct SliceType<'a> {
    element_ty: TypeKey<'a>,
}

impl<'a> SliceType<'a> {
    pub fn new(element_ty: TypeKey<'a>) -> Self {
        SliceType { element_ty }
    }

    pub fn element_ty(&self) -> TypeKey<'a> {
        self.element_ty
    }
}
