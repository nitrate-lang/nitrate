use super::expression::ExprLit;
use super::origin::OriginTag;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ListLit<'a> {
    value: Vec<ExprLit<'a>>,
    origin: OriginTag,
}

impl<'a> ListLit<'a> {
    pub fn new(value: Vec<ExprLit<'a>>, origin: OriginTag) -> Self {
        ListLit { value, origin }
    }

    pub fn into_inner(self) -> Vec<ExprLit<'a>> {
        self.value
    }

    pub fn origin(&self) -> OriginTag {
        self.origin
    }

    pub fn iter(&self) -> std::slice::Iter<ExprLit<'a>> {
        self.value.iter()
    }

    pub fn mut_iter(&mut self) -> std::slice::IterMut<ExprLit<'a>> {
        self.value.iter_mut()
    }
}
