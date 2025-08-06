use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone)]
pub struct GenericType<'a> {
    principal: TypeKey<'a>,
    args: Vec<(&'a str, ExprKey<'a>)>,
}

impl<'a> GenericType<'a> {
    pub fn new(principal: TypeKey<'a>, args: Vec<(&'a str, ExprKey<'a>)>) -> Self {
        GenericType { principal, args }
    }

    pub fn principal(&self) -> TypeKey<'a> {
        self.principal
    }

    pub fn arguments(&self) -> &[(&'a str, ExprKey<'a>)] {
        &self.args
    }
}
