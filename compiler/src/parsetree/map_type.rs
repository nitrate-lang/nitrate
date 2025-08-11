use super::storage::TypeKey;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct MapType<'a> {
    key: TypeKey<'a>,
    value: TypeKey<'a>,
}

impl<'a> MapType<'a> {
    #[must_use]
    pub(crate) fn new(key: TypeKey<'a>, value: TypeKey<'a>) -> Self {
        MapType { key, value }
    }

    #[must_use]
    pub fn key(&self) -> TypeKey<'a> {
        self.key
    }

    #[must_use]
    pub fn value(&self) -> TypeKey<'a> {
        self.value
    }
}
