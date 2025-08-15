use super::expression::Type;

#[derive(Debug, Clone, PartialEq)]
pub struct MapType<'a> {
    key: Type<'a>,
    value: Type<'a>,
}

impl<'a> MapType<'a> {
    #[must_use]
    pub(crate) fn new(key: Type<'a>, value: Type<'a>) -> Self {
        MapType { key, value }
    }

    #[must_use]
    pub fn key(&self) -> Type<'a> {
        self.key.clone()
    }

    #[must_use]
    pub fn value(&self) -> Type<'a> {
        self.value.clone()
    }
}
