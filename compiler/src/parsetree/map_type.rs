use super::expression::Type;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq)]
pub struct MapType<'a> {
    key: Rc<Type<'a>>,
    value: Rc<Type<'a>>,
}

impl<'a> MapType<'a> {
    #[must_use]
    pub(crate) fn new(key: Rc<Type<'a>>, value: Rc<Type<'a>>) -> Self {
        MapType { key, value }
    }

    #[must_use]
    pub fn key(&self) -> Rc<Type<'a>> {
        self.key.clone()
    }

    #[must_use]
    pub fn value(&self) -> Rc<Type<'a>> {
        self.value.clone()
    }
}
