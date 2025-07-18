use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Identifier, Punctuation, Token};
use std::collections::BTreeMap;

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct Object<'a> {
    items: BTreeMap<&'a str, Expr<'a>>,
}

impl<'a> Object<'a> {
    pub fn new(items: BTreeMap<&'a str, Expr<'a>>) -> Self {
        Object { items }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Expr<'a>> {
        self.items
    }

    pub fn get(&self) -> &BTreeMap<&'a str, Expr<'a>> {
        &self.items
    }

    pub fn get_mut(&mut self) -> &mut BTreeMap<&'a str, Expr<'a>> {
        &mut self.items
    }
}

impl<'a> ToCode<'a> for Object<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (key, value) in self.get() {
            tokens.push(Token::Identifier(Identifier::new(key)));
            tokens.push(Token::Punctuation(Punctuation::Colon));

            value.to_code(tokens, options);
            tokens.push(Token::Punctuation(Punctuation::Comma));
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}
