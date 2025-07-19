use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
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

    pub fn expressions(&self) -> &[Expr<'a>] {
        &self.items
    }

    pub fn expressions_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.items
    }
}

impl<'a> ToCode<'a> for Block<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBrace));
        for expr in self.expressions() {
            expr.to_code(tokens, options);
        }
        tokens.push(Token::Punctuation(Punctuation::RightBrace));
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct BlockBuilder<'a> {
    items: Vec<Expr<'a>>,
}

impl<'a> BlockBuilder<'a> {
    pub fn with_expr(mut self, expr: Expr<'a>) -> Self {
        self.items.push(expr);
        self
    }

    pub fn build(self) -> Block<'a> {
        Block::new(self.items)
    }
}
