use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct TupleType<'a> {
    items: Vec<Type<'a>>,
}

impl<'a> TupleType<'a> {
    pub fn new(items: Vec<Type<'a>>) -> Self {
        TupleType { items }
    }

    pub fn into_inner(self) -> Vec<Type<'a>> {
        self.items
    }

    pub fn elements(&self) -> &[Type<'a>] {
        &self.items
    }

    pub fn elements_mut(&mut self) -> &mut Vec<Type<'a>> {
        &mut self.items
    }
}

impl<'a> ToCode<'a> for TupleType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (i, ty) in self.elements().iter().enumerate() {
            if i > 0 {
                tokens.push(Token::Punctuation(Punctuation::Comma));
            }
            ty.to_code(tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}
