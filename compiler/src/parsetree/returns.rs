use super::expression::{CodeFormat, Expr, ToCode};
use crate::lexer::{Keyword, Token};

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Return<'a> {
    value: Option<Expr<'a>>,
}

impl<'a> Return<'a> {
    pub fn new(value: Option<Expr<'a>>) -> Self {
        Return { value }
    }

    pub fn into_inner(self) -> Option<Expr<'a>> {
        self.value
    }

    pub fn value(&self) -> Option<&Expr<'a>> {
        self.value.as_ref()
    }

    pub fn value_mut(&mut self) -> Option<&mut Expr<'a>> {
        self.value.as_mut()
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
