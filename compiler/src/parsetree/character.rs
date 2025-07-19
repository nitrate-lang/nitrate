use super::expression::{CodeFormat, ToCode};
use crate::lexer::Token;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct CharLit {
    value: char,
}

impl CharLit {
    fn new(value: char) -> Self {
        CharLit { value }
    }

    pub fn into_inner(self) -> char {
        self.value
    }

    pub fn get(&self) -> char {
        self.value
    }
}

impl<'a> ToCode<'a> for CharLit {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        tokens.push(Token::Char(self.get()));
    }
}

impl std::ops::Deref for CharLit {
    type Target = char;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct CharLitBuilder {
    value: Option<char>,
}

impl CharLitBuilder {
    pub fn with_char(mut self, value: char) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> CharLit {
        CharLit::new(self.value.expect("CharLitBuilder must have a value"))
    }
}
