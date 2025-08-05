use super::storage::TypeKey;

#[derive(Debug, Clone)]
pub struct MapType<'a> {
    key: TypeKey<'a>,
    value: TypeKey<'a>,
}

impl<'a> MapType<'a> {
    pub fn new(key: TypeKey<'a>, value: TypeKey<'a>) -> Self {
        MapType { key, value }
    }

    pub fn key(&self) -> TypeKey<'a> {
        self.key
    }

    pub fn value(&self) -> TypeKey<'a> {
        self.value
    }
}
