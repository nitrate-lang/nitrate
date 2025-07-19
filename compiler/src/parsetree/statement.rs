use super::expression::{CodeFormat, Expr, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Statement<'a> {
    expr: Box<Expr<'a>>,
}

impl<'a> Statement<'a> {
    fn new(expr: Box<Expr<'a>>) -> Self {
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

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct StatementBuilder<'a> {
    expr: Option<Box<Expr<'a>>>,
}

impl<'a> StatementBuilder<'a> {
    pub fn with_expr(mut self, expr: Box<Expr<'a>>) -> Self {
        self.expr = Some(expr);
        self
    }

    pub fn build(self) -> Statement<'a> {
        Statement::new(self.expr.expect("StatementBuilder must have an expression"))
    }
}
