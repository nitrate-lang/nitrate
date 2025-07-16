use super::origin::OriginTag;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct CharLit {
    value: char,
    origin: OriginTag,
}

impl CharLit {
    pub fn new(value: char, origin: OriginTag) -> Self {
        CharLit { value, origin }
    }

    pub fn origin(&self) -> OriginTag {
        self.origin
    }
}

impl std::ops::Deref for CharLit {
    type Target = char;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}
