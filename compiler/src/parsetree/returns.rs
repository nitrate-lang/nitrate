use super::expression::Expr;

#[derive(Debug, Clone)]
pub struct Return<'a> {
    value: Option<Box<Expr<'a>>>,
}

impl<'a> Return<'a> {
    pub fn new(value: Option<Box<Expr<'a>>>) -> Self {
        Return { value }
    }

    pub fn into_inner(self) -> Option<Box<Expr<'a>>> {
        self.value
    }

    pub fn value(&self) -> Option<&Expr<'a>> {
        self.value.as_deref()
    }

    pub fn value_mut(&mut self) -> Option<&mut Expr<'a>> {
        self.value.as_deref_mut()
    }
}
