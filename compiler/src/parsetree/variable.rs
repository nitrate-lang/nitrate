use super::expression::{CodeFormat, Expr, ToCode};
use super::types::Type;
use crate::lexer::{Identifier, Keyword, Operator, Punctuation, Token};
use std::sync::Arc;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Variable<'a> {
    kind: VariableKind,
    name: &'a str,
    var_type: Option<Arc<Type<'a>>>,
    value: Option<Box<Expr<'a>>>,
}

impl<'a> Variable<'a> {
    fn new(
        kind: VariableKind,
        name: &'a str,
        var_type: Option<Arc<Type<'a>>>,
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
        self.kind.clone()
    }

    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    pub fn get_type(&self) -> Option<&Arc<Type<'a>>> {
        self.var_type.as_ref()
    }

    pub fn get_type_mut(&mut self) -> Option<&mut Arc<Type<'a>>> {
        self.var_type.as_mut()
    }

    pub fn value(&self) -> Option<&Expr<'a>> {
        self.value.as_deref()
    }

    pub fn value_mut(&mut self) -> Option<&mut Expr<'a>> {
        self.value.as_deref_mut()
    }
}

impl<'a> ToCode<'a> for Variable<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        match self.kind() {
            VariableKind::Let => tokens.push(Token::Keyword(Keyword::Let)),
            VariableKind::Var => tokens.push(Token::Keyword(Keyword::Var)),
        }

        tokens.push(Token::Identifier(Identifier::new(self.name())));

        if let Some(var_type) = self.get_type() {
            tokens.push(Token::Punctuation(Punctuation::Colon));
            var_type.to_code(tokens, options);
        }

        if let Some(value) = self.value() {
            tokens.push(Token::Operator(Operator::Set));
            value.to_code(tokens, options);
        }
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct VariableBuilder<'a> {
    kind: Option<VariableKind>,
    name: &'a str,
    var_type: Option<Arc<Type<'a>>>,
    value: Option<Box<Expr<'a>>>,
}

impl<'a> VariableBuilder<'a> {
    pub fn with_kind(mut self, kind: VariableKind) -> Self {
        self.kind = Some(kind);
        self
    }

    pub fn with_name(mut self, name: &'a str) -> Self {
        self.name = name;
        self
    }

    pub fn with_type(mut self, var_type: Arc<Type<'a>>) -> Self {
        self.var_type = Some(var_type);
        self
    }

    pub fn with_value(mut self, value: Box<Expr<'a>>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Variable<'a> {
        let kind = self.kind.expect("VariableBuilder kind must be set");

        Variable::new(kind, self.name, self.var_type, self.value)
    }
}
