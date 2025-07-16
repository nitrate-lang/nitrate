use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct List<'a> {
    value: Vec<Expr<'a>>,
}

impl<'a> List<'a> {
    pub fn new(value: Vec<Expr<'a>>) -> Self {
        List { value }
    }

    pub fn into_inner(self) -> Vec<Expr<'a>> {
        self.value
    }

    pub fn inner_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.value
    }

    pub fn items(&self) -> &[Expr<'a>] {
        &self.value
    }

    pub fn iter(&self) -> std::slice::Iter<Expr<'a>> {
        self.value.iter()
    }

    pub fn iter_mut(&mut self) -> std::slice::IterMut<Expr<'a>> {
        self.value.iter_mut()
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
