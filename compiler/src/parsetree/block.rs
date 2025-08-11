use super::storage::ExprKey;

#[derive(Debug, Clone)]
pub struct Block<'a> {
    elements: Vec<ExprKey<'a>>,
}

impl<'a> Block<'a> {
    #[must_use]
    pub(crate) fn new(items: Vec<ExprKey<'a>>) -> Self {
        Block { elements: items }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<ExprKey<'a>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[ExprKey<'a>] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<ExprKey<'a>> {
        &mut self.elements
    }
}
