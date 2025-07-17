use super::expression::{CodeFormat, ToCode};
use crate::lexer::{StringLit as StringLitToken, Token};

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
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
