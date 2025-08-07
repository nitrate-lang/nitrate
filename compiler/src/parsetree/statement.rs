use super::storage::ExprKey;

#[derive(Debug, Clone)]
pub struct Statement<'a> {
    expr: ExprKey<'a>,
}

impl<'a> Statement<'a> {
    pub(crate) fn new(expr: ExprKey<'a>) -> Self {
        Statement { expr }
    }

    pub fn into_inner(self) -> ExprKey<'a> {
        self.expr
    }

    pub fn get(&self) -> ExprKey<'a> {
        self.expr
    }

    pub fn set(&mut self, expr: ExprKey<'a>) {
        self.expr = expr;
    }
}
