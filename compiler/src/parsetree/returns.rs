use super::expression::{CodeFormat, Expr, ToCode};
use crate::lexer::{Keyword, Token};

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Return<'a> {
    value: Option<Box<Expr<'a>>>,
}

impl<'a> Return<'a> {
    fn new(value: Option<Box<Expr<'a>>>) -> Self {
        Return { value }
    }

    pub fn into_inner(self) -> Option<Box<Expr<'a>>> {
        self.value
    }

    pub fn value(&self) -> Option<&Expr<'a>> {
        self.value.as_deref()
    }

    pub fn value_mut(&mut self) -> Option<&mut Expr<'a>> {
        self.value.as_deref_mut()
    }
}

impl<'a> ToCode<'a> for Return<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Ret));
        if let Some(value) = self.value() {
            value.to_code(tokens, options);
        }
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct ReturnBuilder<'a> {
    value: Option<Box<Expr<'a>>>,
}

impl<'a> ReturnBuilder<'a> {
    pub fn with_value(mut self, value: Box<Expr<'a>>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Return<'a> {
        Return::new(self.value)
    }
}
