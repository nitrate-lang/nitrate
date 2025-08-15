use super::node::Expr;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct ListLit<'a> {
    elements: Vec<Arc<Expr<'a>>>,
}

impl<'a> ListLit<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<Arc<Expr<'a>>>) -> Self {
        ListLit { elements }
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
