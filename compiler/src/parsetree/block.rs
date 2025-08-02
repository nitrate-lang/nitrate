use super::expression::Expr;

#[derive(Debug, Clone)]
pub struct Block<'a> {
    elements: Vec<Box<Expr<'a>>>,
}

impl<'a> Block<'a> {
    pub fn new(items: Vec<Box<Expr<'a>>>) -> Self {
        Block { elements: items }
    }

    pub fn into_inner(self) -> Vec<Box<Expr<'a>>> {
        self.elements
    }

    pub fn elements(&self) -> &[Box<Expr<'a>>] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<Box<Expr<'a>>> {
        &mut self.elements
    }
}
