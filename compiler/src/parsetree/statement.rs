use super::expression::{CodeFormat, Expr, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Statement<'a> {
    expr: Box<Expr<'a>>,
}

impl<'a> Statement<'a> {
    pub fn new(expr: Box<Expr<'a>>) -> Self {
        Statement { expr }
    }

    pub fn into_inner(self) -> Box<Expr<'a>> {
        self.expr
    }

    pub fn get(&self) -> &Expr<'a> {
        &self.expr
    }

    pub fn get_mut(&mut self) -> &mut Expr<'a> {
        &mut self.expr
    }
}

impl<'a> ToCode<'a> for Statement<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        self.expr.to_code(tokens, options);
        tokens.push(Token::Punctuation(Punctuation::Semicolon));
    }
}

impl<'a> std::ops::Deref for Statement<'a> {
    type Target = Expr<'a>;

    fn deref(&self) -> &Self::Target {
        &self.expr
    }
}

impl<'a> std::ops::DerefMut for Statement<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.expr
    }
}
