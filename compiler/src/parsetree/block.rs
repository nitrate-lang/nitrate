use super::expression::Expr;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct Block<'a> {
    elements: Vec<Arc<Expr<'a>>>,
}

impl<'a> Block<'a> {
    #[must_use]
    pub(crate) fn new(items: Vec<Arc<Expr<'a>>>) -> Self {
        Block { elements: items }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<Arc<Expr<'a>>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[Arc<Expr<'a>>] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.elements
    }
}
