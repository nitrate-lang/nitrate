use super::storage::ExprKey;

#[derive(Debug, Clone)]
pub struct Block<'a> {
    elements: Vec<ExprKey<'a>>,
}

impl<'a> Block<'a> {
    pub fn new(items: Vec<ExprKey<'a>>) -> Self {
        Block { elements: items }
    }

    pub fn into_inner(self) -> Vec<ExprKey<'a>> {
        self.elements
    }

    pub fn elements(&self) -> &[ExprKey<'a>] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<ExprKey<'a>> {
        &mut self.elements
    }
}
