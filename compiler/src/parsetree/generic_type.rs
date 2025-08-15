use super::expression::{ExprOwned, TypeOwned};
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct GenericType<'a> {
    base: Rc<TypeOwned<'a>>,
    args: Vec<(&'a str, Arc<ExprOwned<'a>>)>,
}

impl<'a> GenericType<'a> {
    #[must_use]
    pub(crate) fn new(base: Rc<TypeOwned<'a>>, args: Vec<(&'a str, Arc<ExprOwned<'a>>)>) -> Self {
        GenericType { base, args }
    }

    #[must_use]
    pub fn base(&self) -> Rc<TypeOwned<'a>> {
        self.base.clone()
    }

    #[must_use]
    pub fn arguments(&self) -> &[(&'a str, Arc<ExprOwned<'a>>)] {
        &self.args
    }
}
