use super::storage::ExprRef;

#[derive(Debug, Clone)]
pub struct List<'a> {
    elements: Vec<ExprRef<'a>>,
}

impl<'a> List<'a> {
    pub fn new(elements: Vec<ExprRef<'a>>) -> Self {
        List { elements }
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
