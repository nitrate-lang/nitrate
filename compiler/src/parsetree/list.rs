use super::storage::ExprKey;

#[derive(Debug, Clone)]
pub struct ListLit<'a> {
    elements: Vec<ExprKey<'a>>,
}

impl<'a> ListLit<'a> {
    pub fn new(elements: Vec<ExprKey<'a>>) -> Self {
        ListLit { elements }
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
