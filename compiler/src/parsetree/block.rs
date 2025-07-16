use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Block<'a> {
    code: Vec<Expr<'a>>,
}

impl<'a> Block<'a> {
    pub fn new(code: Vec<Expr<'a>>) -> Self {
        Block { code }
    }

    pub fn into_inner(self) -> Vec<Expr<'a>> {
        self.code
    }

    pub fn inner_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.code
    }

    pub fn items(&self) -> &[Expr<'a>] {
        &self.code
    }

    pub fn iter(&self) -> std::slice::Iter<Expr<'a>> {
        self.code.iter()
    }

    pub fn iter_mut(&mut self) -> std::slice::IterMut<Expr<'a>> {
        self.code.iter_mut()
    }
}

impl<'a> ToCode<'a> for Block<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBrace));
        for expr in self.items() {
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBrace));
    }
}
