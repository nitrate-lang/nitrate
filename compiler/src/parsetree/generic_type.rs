use super::expression::{Expr, Type};
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct GenericType<'a> {
    base: Type<'a>,
    args: Vec<(&'a str, Arc<Expr<'a>>)>,
}

impl<'a> GenericType<'a> {
    #[must_use]
    pub(crate) fn new(base: Type<'a>, args: Vec<(&'a str, Arc<Expr<'a>>)>) -> Self {
        GenericType { base, args }
    }

    #[must_use]
    pub fn base(&self) -> Type<'a> {
        self.base.clone()
    }

    #[must_use]
    pub fn arguments(&self) -> &[(&'a str, Arc<Expr<'a>>)] {
        &self.args
    }
}
