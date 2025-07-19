use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Identifier, Punctuation, Token};
use std::collections::BTreeMap;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Object<'a> {
    fields: BTreeMap<&'a str, Expr<'a>>,
}

impl<'a> Object<'a> {
    fn new(fields: BTreeMap<&'a str, Expr<'a>>) -> Self {
        Object { fields }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Expr<'a>> {
        self.fields
    }

    pub fn get(&self) -> &BTreeMap<&'a str, Expr<'a>> {
        &self.fields
    }

    pub fn get_mut(&mut self) -> &mut BTreeMap<&'a str, Expr<'a>> {
        &mut self.fields
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

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct ObjectBuilder<'a> {
    fields: BTreeMap<&'a str, Expr<'a>>,
}
impl<'a> ObjectBuilder<'a> {
    pub fn with_field(mut self, key: &'a str, value: Expr<'a>) -> Self {
        self.fields.insert(key, value);
        self
    }

    pub fn build(self) -> Object<'a> {
        Object::new(self.fields)
    }
}
