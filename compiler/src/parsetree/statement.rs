use super::expression::ExprOwned;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct Statement<'a> {
    expr: Arc<ExprOwned<'a>>,
}

impl<'a> Statement<'a> {
    #[must_use]
    pub(crate) fn new(expr: Arc<ExprOwned<'a>>) -> Self {
        Statement { expr }
    }

    #[must_use]
    pub fn into_inner(self) -> Arc<ExprOwned<'a>> {
        self.expr
    }

    #[must_use]
    pub fn get(&self) -> Arc<ExprOwned<'a>> {
        self.expr.clone()
    }

    pub fn set(&mut self, expr: Arc<ExprOwned<'a>>) {
        self.expr = expr;
    }
}
