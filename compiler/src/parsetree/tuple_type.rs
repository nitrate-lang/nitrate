use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct TupleType<'a> {
    elements: Vec<Type<'a>>,
}

impl<'a> TupleType<'a> {
    pub fn new(elements: Vec<Type<'a>>) -> Self {
        TupleType { elements }
    }

    pub fn into_inner(self) -> Vec<Type<'a>> {
        self.elements
    }

    pub fn elements(&self) -> &[Type<'a>] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<Type<'a>> {
        &mut self.elements
    }
}

impl<'a> ToCode<'a> for TupleType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (i, ty) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
            ty.to_code(tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}
