use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct List<'a> {
    items: Vec<Expr<'a>>,
}

impl<'a> List<'a> {
    pub fn new(items: Vec<Expr<'a>>) -> Self {
        List { items }
    }

    pub fn into_inner(self) -> Vec<Expr<'a>> {
        self.items
    }

    pub fn items(&self) -> &[Expr<'a>] {
        &self.items
    }
}

impl<'a> ToCode<'a> for List<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (i, expr) in self.iter().enumerate() {
            if i > 0 {
                tokens.push(Token::Punctuation(Punctuation::Comma));
            }
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

impl<'a> std::ops::Deref for List<'a> {
    type Target = [Expr<'a>];

    fn deref(&self) -> &Self::Target {
        &self.items
    }
}

impl<'a> std::ops::DerefMut for List<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.items
    }
}
