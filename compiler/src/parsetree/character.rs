#[derive(Debug, Clone)]
pub struct CharLit {
    value: char,
}

impl CharLit {
    pub fn new(value: char) -> Self {
        CharLit { value }
    }

    pub fn into_inner(self) -> char {
        self.value
    }

    pub fn get(&self) -> char {
        self.value
    }
}

impl std::ops::Deref for CharLit {
    type Target = char;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}
