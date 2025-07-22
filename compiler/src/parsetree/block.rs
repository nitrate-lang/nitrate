use super::expression::Expr;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Block<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> Block<'a> {
    pub fn new(items: Vec<Expr<'a>>) -> Self {
        Block { elements: items }
    }

    pub fn into_inner(self) -> Vec<Expr<'a>> {
        self.elements
    }

    pub fn elements(&self) -> &[Expr<'a>] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.elements
    }
}
