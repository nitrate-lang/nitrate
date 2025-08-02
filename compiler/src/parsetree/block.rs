use super::storage::ExprRef;

#[derive(Debug, Clone)]
pub struct Block<'a> {
    elements: Vec<ExprRef<'a>>,
}

impl<'a> Block<'a> {
    pub fn new(items: Vec<ExprRef<'a>>) -> Self {
        Block { elements: items }
    }

    pub fn into_inner(self) -> Vec<ExprRef<'a>> {
        self.elements
    }

    pub fn elements(&self) -> &[ExprRef<'a>] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<ExprRef<'a>> {
        &mut self.elements
    }
}
