use super::expression::{CodeFormat, ToCode};
use crate::lexer::{StringLit as StringLitToken, Token};

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StringLit<'a> {
    value: &'a [u8],
}

impl<'a> StringLit<'a> {
    fn new(value: &'a [u8]) -> Self {
        StringLit { value }
    }

    pub fn get(&self) -> &'a [u8] {
        self.value
    }
}

impl<'a> ToCode<'a> for StringLit<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        let string_lit = StringLitToken::from_ref(self);
        tokens.push(Token::String(string_lit));
    }
}

impl<'a> std::ops::Deref for StringLit<'a> {
    type Target = &'a [u8];

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct StringLitBuilder<'a> {
    value: Option<&'a [u8]>,
}

impl<'a> StringLitBuilder<'a> {
    pub fn with_value(mut self, value: &'a [u8]) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> StringLit<'a> {
        StringLit::new(self.value.expect("StringLitBuilder must have a value"))
    }
}
