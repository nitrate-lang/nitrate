use super::expression::Expr;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct List<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> List<'a> {
    pub fn new(elements: Vec<Expr<'a>>) -> Self {
        List { elements }
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
