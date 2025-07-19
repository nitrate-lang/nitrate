use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Punctuation, Token};
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct TupleType<'a> {
    elements: Vec<Rc<Type<'a>>>,
}

impl<'a> TupleType<'a> {
    fn new(elements: Vec<Rc<Type<'a>>>) -> Self {
        TupleType { elements }
    }

    pub fn into_inner(self) -> Vec<Rc<Type<'a>>> {
        self.elements
    }

    pub fn elements(&self) -> &[Rc<Type<'a>>] {
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
    elements: Vec<Rc<Type<'a>>>,
}

impl<'a> TupleTypeBuilder<'a> {
    pub fn with_element(mut self, element: Rc<Type<'a>>) -> Self {
        self.elements.push(element);
        self
    }

    pub fn with_elements(mut self, elements: Vec<Rc<Type<'a>>>) -> Self {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> TupleType<'a> {
        TupleType::new(self.elements)
    }
}
