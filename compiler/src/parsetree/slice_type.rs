use super::storage::TypeKey;

#[derive(Debug, Clone)]
pub struct SliceType<'a> {
    element: TypeKey<'a>,
}

impl<'a> SliceType<'a> {
    pub fn new(element: TypeKey<'a>) -> Self {
        SliceType { element }
    }

    pub fn element(&self) -> TypeKey<'a> {
        self.element
    }
}
