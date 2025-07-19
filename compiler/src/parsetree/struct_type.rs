use super::expression::{CodeFormat, Expr, ToCode};
use super::types::Type;
use crate::lexer::{Identifier, Keyword, Punctuation, Token};
use std::collections::BTreeMap;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StructType<'a> {
    name: Option<&'a str>,
    attributes: Vec<Expr<'a>>,
    fields: BTreeMap<&'a str, Arc<Type<'a>>>,
}

impl<'a> StructType<'a> {
    pub fn new(
        name: Option<&'a str>,
        attributes: Vec<Expr<'a>>,
        fields: BTreeMap<&'a str, Arc<Type<'a>>>,
    ) -> Self {
        StructType {
            name,
            attributes,
            fields,
        }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Arc<Type<'a>>> {
        self.fields
    }

    pub fn name(&self) -> Option<&'a str> {
        self.name
    }

    pub fn attributes(&self) -> &[Expr<'a>] {
        &self.attributes
    }

    pub fn fields(&self) -> &BTreeMap<&'a str, Arc<Type<'a>>> {
        &self.fields
    }
}

impl<'a> ToCode<'a> for StructType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Struct));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punctuation(Punctuation::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i != 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
                attr.to_code(tokens, options);
            }
            tokens.push(Token::Punctuation(Punctuation::RightBracket));
        }

        if let Some(name) = self.name() {
            tokens.push(Token::Identifier(Identifier::new(name)));
        }

        tokens.push(Token::Punctuation(Punctuation::LeftBrace));
        for (field_name, field_ty) in self.fields() {
            tokens.push(Token::Identifier(Identifier::new(field_name)));
            tokens.push(Token::Punctuation(Punctuation::Colon));
            field_ty.to_code(tokens, options);
            tokens.push(Token::Punctuation(Punctuation::Comma));
        }
        tokens.push(Token::Punctuation(Punctuation::RightBrace));
    }
}
