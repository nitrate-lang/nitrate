use super::expression::Expr;

#[derive(Debug, Clone)]
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
