use super::node::Expr;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct Statement<'a> {
    expr: Arc<Expr<'a>>,
}

impl<'a> Statement<'a> {
    #[must_use]
    pub(crate) fn new(expr: Arc<Expr<'a>>) -> Self {
        Statement { expr }
    }

    #[must_use]
    pub fn into_inner(self) -> Arc<Expr<'a>> {
        self.expr
    }

    #[must_use]
    pub fn get(&self) -> Arc<Expr<'a>> {
        self.expr.clone()
    }

    pub fn set(&mut self, expr: Arc<Expr<'a>>) {
        self.expr = expr;
    }
}
