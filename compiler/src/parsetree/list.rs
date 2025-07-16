use super::expression::Expr;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct List<'a> {
    value: Vec<Expr<'a>>,
}

impl<'a> List<'a> {
    pub fn new(value: Vec<Expr<'a>>) -> Self {
        List { value }
    }

    pub fn into_inner(self) -> Vec<Expr<'a>> {
        self.value
    }

    pub fn iter(&self) -> std::slice::Iter<Expr<'a>> {
        self.value.iter()
    }

    pub fn iter_mut(&mut self) -> std::slice::IterMut<Expr<'a>> {
        self.value.iter_mut()
    }
}
