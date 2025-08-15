use super::expression::ExprOwned;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct Block<'a> {
    elements: Vec<Arc<ExprOwned<'a>>>,
}

impl<'a> Block<'a> {
    #[must_use]
    pub(crate) fn new(items: Vec<Arc<ExprOwned<'a>>>) -> Self {
        Block { elements: items }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<Arc<ExprOwned<'a>>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[Arc<ExprOwned<'a>>] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Arc<ExprOwned<'a>>> {
        &mut self.elements
    }
}
