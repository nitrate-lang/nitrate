use super::expression::{CodeFormat, ToCode};
use crate::lexer::Token;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
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

impl<'a> ToCode<'a> for CharLit {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        tokens.push(Token::Char(self.get()));
    }
}
