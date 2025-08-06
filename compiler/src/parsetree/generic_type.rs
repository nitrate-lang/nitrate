use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct GenericType<'a> {
    base: TypeKey<'a>,
    args: Vec<(&'a str, ExprKey<'a>)>,
}

impl<'a> GenericType<'a> {
    pub(crate) fn new(base: TypeKey<'a>, args: Vec<(&'a str, ExprKey<'a>)>) -> Self {
        GenericType { base, args }
    }

    pub fn base(&self) -> TypeKey<'a> {
        self.base
    }

    pub fn arguments(&self) -> &[(&'a str, ExprKey<'a>)] {
        &self.args
    }
}
