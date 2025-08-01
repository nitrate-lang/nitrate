use super::expression::Expr;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct List<'a> {
    elements: Vec<Box<Expr<'a>>>,
}

impl<'a> List<'a> {
    pub fn new(elements: Vec<Box<Expr<'a>>>) -> Self {
        List { elements }
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
