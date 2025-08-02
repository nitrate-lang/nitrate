use super::storage::ExprRef;

#[derive(Debug, Clone)]
pub struct Return<'a> {
    value: Option<ExprRef<'a>>,
}

impl<'a> Return<'a> {
    pub fn new(value: Option<ExprRef<'a>>) -> Self {
        Return { value }
    }

    pub fn into_inner(self) -> Option<ExprRef<'a>> {
        self.value
    }

    pub fn value(&self) -> Option<ExprRef<'a>> {
        self.value
    }

    pub fn set_value(&mut self, value: Option<ExprRef<'a>>) {
        self.value = value;
    }
}
