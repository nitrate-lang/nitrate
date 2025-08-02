use super::storage::ExprKey;

#[derive(Debug, Clone)]
pub struct Return<'a> {
    value: Option<ExprKey<'a>>,
}

impl<'a> Return<'a> {
    pub fn new(value: Option<ExprKey<'a>>) -> Self {
        Return { value }
    }

    pub fn into_inner(self) -> Option<ExprKey<'a>> {
        self.value
    }

    pub fn value(&self) -> Option<ExprKey<'a>> {
        self.value
    }

    pub fn set_value(&mut self, value: Option<ExprKey<'a>>) {
        self.value = value;
    }
}
