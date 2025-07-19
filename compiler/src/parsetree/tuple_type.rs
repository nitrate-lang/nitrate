use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Punctuation, Token};
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct TupleType<'a> {
    elements: Vec<Arc<Type<'a>>>,
}

impl<'a> TupleType<'a> {
    fn new(elements: Vec<Arc<Type<'a>>>) -> Self {
        TupleType { elements }
    }

    pub fn into_inner(self) -> Vec<Arc<Type<'a>>> {
        self.elements
    }

    pub fn elements(&self) -> &[Arc<Type<'a>>] {
        &self.elements
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

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct TupleTypeBuilder<'a> {
    elements: Vec<Arc<Type<'a>>>,
}

impl<'a> TupleTypeBuilder<'a> {
    pub fn with_element(mut self, element: Arc<Type<'a>>) -> Self {
        self.elements.push(element);
        self
    }

    pub fn with_elements(mut self, elements: Vec<Arc<Type<'a>>>) -> Self {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> TupleType<'a> {
        TupleType::new(self.elements)
    }
}
