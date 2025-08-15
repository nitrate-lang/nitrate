use super::expression::{Expr, Type};
use std::sync::Arc;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Variable<'a> {
    kind: VariableKind,
    is_mutable: bool,
    attributes: Vec<Arc<Expr<'a>>>,
    name: &'a str,
    var_type: Type<'a>,
    value: Option<Arc<Expr<'a>>>,
}

impl<'a> Variable<'a> {
    #[must_use]
    pub(crate) fn new(
        kind: VariableKind,
        is_mutable: bool,
        attributes: Vec<Arc<Expr<'a>>>,
        name: &'a str,
        var_type: Type<'a>,
        value: Option<Arc<Expr<'a>>>,
    ) -> Self {
        Variable {
            kind,
            is_mutable,
            attributes,
            name,
            var_type,
            value,
        }
    }

    #[must_use]
    pub fn kind(&self) -> VariableKind {
        self.kind
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }

    pub fn set_mutable(&mut self, is_mutable: bool) {
        self.is_mutable = is_mutable;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<Expr<'a>>] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    #[must_use]
    pub fn get_type(&self) -> Type<'a> {
        self.var_type.clone()
    }

    pub fn set_type(&mut self, var_type: Type<'a>) {
        self.var_type = var_type;
    }

    #[must_use]
    pub fn value(&self) -> Option<Arc<Expr<'a>>> {
        self.value.clone()
    }

    pub fn set_value(&mut self, value: Option<Arc<Expr<'a>>>) {
        self.value = value;
    }
}
