use super::expression::{Expr, Type};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Variable<'a> {
    kind: VariableKind,
    name: &'a str,
    var_type: Option<Box<Type<'a>>>,
    value: Option<Box<Expr<'a>>>,
}

impl<'a> Variable<'a> {
    pub fn new(
        kind: VariableKind,
        name: &'a str,
        var_type: Option<Box<Type<'a>>>,
        value: Option<Box<Expr<'a>>>,
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

    pub fn get_type(&self) -> Option<&Box<Type<'a>>> {
        self.var_type.as_ref()
    }

    pub fn get_type_mut(&mut self) -> Option<&mut Box<Type<'a>>> {
        self.var_type.as_mut()
    }

    pub fn value(&self) -> Option<&Expr<'a>> {
        self.value.as_deref()
    }

    pub fn value_mut(&mut self) -> Option<&mut Expr<'a>> {
        self.value.as_deref_mut()
    }
}
