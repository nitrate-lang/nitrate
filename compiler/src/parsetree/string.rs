#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StringLit<'a> {
    value: &'a [u8],
}

impl<'a> StringLit<'a> {
    pub fn new(value: &'a [u8]) -> Self {
        StringLit { value }
    }

    pub fn get(&self) -> &'a [u8] {
        self.value
    }
}

impl<'a> std::ops::Deref for StringLit<'a> {
    type Target = &'a [u8];

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}
