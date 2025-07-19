use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct List<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> List<'a> {
    fn new(elements: Vec<Expr<'a>>) -> Self {
        List { elements }
    }

    pub fn into_inner(self) -> Vec<Expr<'a>> {
        self.elements
    }

    pub fn elements(&self) -> &[Expr<'a>] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.elements
    }
}

impl<'a> ToCode<'a> for List<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (i, expr) in self.elements().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct ListBuilder<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> ListBuilder<'a> {
    pub fn with_element(mut self, element: Expr<'a>) -> Self {
        self.elements.push(element);
        self
    }

    pub fn build(self) -> List<'a> {
        List::new(self.elements)
    }
}
