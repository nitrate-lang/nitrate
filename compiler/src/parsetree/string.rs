#[derive(Debug, Clone)]
pub struct StringLit<'a> {
    value: &'a str,
}

impl<'a> StringLit<'a> {
    pub fn new(value: &'a str) -> Self {
        StringLit { value }
    }

    pub fn get(&self) -> &'a str {
        self.value
    }
}

impl<'a> std::ops::Deref for StringLit<'a> {
    type Target = &'a str;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}
