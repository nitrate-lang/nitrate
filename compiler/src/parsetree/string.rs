use crate::lexer::StringData;

#[derive(Debug, Clone)]
pub struct StringLit<'a> {
    value: StringData<'a>,
}

impl<'a> StringLit<'a> {
    pub(crate) fn new(value: StringData<'a>) -> Self {
        StringLit { value }
    }

    pub fn into_inner(self) -> StringData<'a> {
        self.value
    }

    pub fn get(&self) -> &str {
        self.value.get()
    }
}
