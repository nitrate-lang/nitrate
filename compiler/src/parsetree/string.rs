use super::expression::ToCode;
use crate::lexer::Token;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct StringLit<'a> {
    value: &'a str,
}

impl<'a> StringLit<'a> {
    pub fn new(value: &'a str) -> Self {
        StringLit { value }
    }
}

impl<'a> std::ops::Deref for StringLit<'a> {
    type Target = &'a str;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

impl<'a> ToCode<'a> for StringLit<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>) {
        // TODO: Convert string to code
    }
}
