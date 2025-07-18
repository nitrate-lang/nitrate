use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Identifier, Punctuation, Token};
use std::collections::BTreeMap;

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct StructType<'a> {
    items: BTreeMap<&'a str, Expr<'a>>,
}

impl<'a> StructType<'a> {
    pub fn new(items: BTreeMap<&'a str, Expr<'a>>) -> Self {
        StructType { items }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Expr<'a>> {
        self.items
    }

    pub fn items(&self) -> &BTreeMap<&'a str, Expr<'a>> {
        &self.items
    }
}

impl<'a> ToCode<'a> for StructType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (key, value) in self.items().iter() {
            tokens.push(Token::Identifier(Identifier::new(key)));
            tokens.push(Token::Punctuation(Punctuation::Colon));

            value.to_code(tokens, options);
            tokens.push(Token::Punctuation(Punctuation::Comma));
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

impl<'a> std::ops::Deref for StructType<'a> {
    type Target = BTreeMap<&'a str, Expr<'a>>;

    fn deref(&self) -> &Self::Target {
        &self.items
    }
}

impl<'a> std::ops::DerefMut for StructType<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.items
    }
}
