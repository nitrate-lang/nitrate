use super::expression::Expr;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Statement<'a> {
    expr: Expr<'a>,
}

impl<'a> Statement<'a> {
    pub fn new(expr: Expr<'a>) -> Self {
        Statement { expr }
    }

    pub fn into_inner(self) -> Expr<'a> {
        self.expr
    }

    pub fn get(&self) -> &Expr<'a> {
        &self.expr
    }

    pub fn get_mut(&mut self) -> &mut Expr<'a> {
        &mut self.expr
    }
}
