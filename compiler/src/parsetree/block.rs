use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct Block<'a> {
    items: Vec<Expr<'a>>,
}

impl<'a> Block<'a> {
    pub fn new(items: Vec<Expr<'a>>) -> Self {
        Block { items }
    }

    pub fn into_inner(self) -> Vec<Expr<'a>> {
        self.items
    }

    pub fn items(&self) -> &[Expr<'a>] {
        &self.items
    }
}

impl<'a> ToCode<'a> for Block<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBrace));
        for expr in self.iter() {
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBrace));
    }
}

impl<'a> std::ops::Deref for Block<'a> {
    type Target = [Expr<'a>];

    fn deref(&self) -> &Self::Target {
        &self.items
    }
}

impl<'a> std::ops::DerefMut for Block<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.items
    }
}
