use super::storage::{ExprRef, TypeRef};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone)]
pub struct Variable<'a> {
    kind: VariableKind,
    name: &'a str,
    var_type: Option<TypeRef<'a>>,
    value: Option<ExprRef<'a>>,
}

impl<'a> Variable<'a> {
    pub fn new(
        kind: VariableKind,
        name: &'a str,
        var_type: Option<TypeRef<'a>>,
        value: Option<ExprRef<'a>>,
    ) -> Self {
        Variable {
            kind,
            name,
            var_type,
            value,
        }
    }

    pub fn kind(&self) -> VariableKind {
        self.kind
    }

    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    pub fn get_type(&self) -> Option<TypeRef<'a>> {
        self.var_type
    }

    pub fn set_type(&mut self, var_type: Option<TypeRef<'a>>) {
        self.var_type = var_type;
    }

    pub fn value(&self) -> Option<ExprRef<'a>> {
        self.value
    }

    pub fn set_value(&mut self, value: Option<ExprRef<'a>>) {
        self.value = value;
    }
}
