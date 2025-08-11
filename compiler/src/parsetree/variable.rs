use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone)]
pub struct Variable<'a> {
    kind: VariableKind,
    name: &'a str,
    var_type: Option<TypeKey<'a>>,
    value: Option<ExprKey<'a>>,
}

impl<'a> Variable<'a> {
    #[must_use]
    pub(crate) fn new(
        kind: VariableKind,
        name: &'a str,
        var_type: Option<TypeKey<'a>>,
        value: Option<ExprKey<'a>>,
    ) -> Self {
        Variable {
            kind,
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
    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    #[must_use]
    pub fn get_type(&self) -> Option<TypeKey<'a>> {
        self.var_type
    }

    pub fn set_type(&mut self, var_type: Option<TypeKey<'a>>) {
        self.var_type = var_type;
    }

    #[must_use]
    pub fn value(&self) -> Option<ExprKey<'a>> {
        self.value
    }

    pub fn set_value(&mut self, value: Option<ExprKey<'a>>) {
        self.value = value;
    }
}
