use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Identifier, Punctuation, Token};
use std::collections::BTreeMap;

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct StructType<'a> {
    fields: BTreeMap<&'a str, Type<'a>>,
}

impl<'a> StructType<'a> {
    pub fn new(fields: BTreeMap<&'a str, Type<'a>>) -> Self {
        StructType { fields }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Type<'a>> {
        self.fields
    }

    pub fn fields(&self) -> &BTreeMap<&'a str, Type<'a>> {
        &self.fields
    }
}

impl<'a> ToCode<'a> for StructType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        for (field_name, field_ty) in self.iter() {
            tokens.push(Token::Identifier(Identifier::new(field_name)));
            tokens.push(Token::Punctuation(Punctuation::Colon));

            field_ty.to_code(tokens, options);
            tokens.push(Token::Punctuation(Punctuation::Comma));
        }
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

impl<'a> std::ops::Deref for StructType<'a> {
    type Target = BTreeMap<&'a str, Type<'a>>;

    fn deref(&self) -> &Self::Target {
        &self.fields
    }
}

impl<'a> std::ops::DerefMut for StructType<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.fields
    }
}
