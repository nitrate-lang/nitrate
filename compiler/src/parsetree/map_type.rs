use super::expression::TypeOwned;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq)]
pub struct MapType<'a> {
    key: Rc<TypeOwned<'a>>,
    value: Rc<TypeOwned<'a>>,
}

impl<'a> MapType<'a> {
    #[must_use]
    pub(crate) fn new(key: Rc<TypeOwned<'a>>, value: Rc<TypeOwned<'a>>) -> Self {
        MapType { key, value }
    }

    #[must_use]
    pub fn key(&self) -> Rc<TypeOwned<'a>> {
        self.key.clone()
    }

    #[must_use]
    pub fn value(&self) -> Rc<TypeOwned<'a>> {
        self.value.clone()
    }
}
