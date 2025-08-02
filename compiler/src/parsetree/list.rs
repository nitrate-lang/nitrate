use super::storage::ExprRef;

#[derive(Debug, Clone)]
pub struct ListLit<'a> {
    elements: Vec<ExprRef<'a>>,
}

impl<'a> ListLit<'a> {
    pub fn new(elements: Vec<ExprRef<'a>>) -> Self {
        ListLit { elements }
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
