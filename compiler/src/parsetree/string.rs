use super::origin::OriginTag;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct StringLit<'a> {
    value: &'a str,
    origin: OriginTag,
}

impl<'a> StringLit<'a> {
    pub fn new(value: &'a str, origin: OriginTag) -> Self {
        StringLit { value, origin }
    }

    pub fn origin(&self) -> OriginTag {
        self.origin
    }
}

impl<'a> std::ops::Deref for StringLit<'a> {
    type Target = &'a str;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}
