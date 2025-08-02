use super::storage::ExprRef;

#[derive(Debug, Clone)]
pub struct Statement<'a> {
    expr: ExprRef<'a>,
}

impl<'a> Statement<'a> {
    pub fn new(expr: ExprRef<'a>) -> Self {
        Statement { expr }
    }

    pub fn into_inner(self) -> ExprRef<'a> {
        self.expr
    }

    pub fn get(&self) -> ExprRef<'a> {
        self.expr
    }

    pub fn set(&mut self, expr: ExprRef<'a>) {
        self.expr = expr;
    }
}
