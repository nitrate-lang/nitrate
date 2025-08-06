use super::storage::TypeKey;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct SliceType<'a> {
    element: TypeKey<'a>,
}

impl<'a> SliceType<'a> {
    pub(crate) fn new(element: TypeKey<'a>) -> Self {
        SliceType { element }
    }

    pub fn element(&self) -> TypeKey<'a> {
        self.element
    }
}
