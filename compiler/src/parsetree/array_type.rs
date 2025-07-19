use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Punctuation, Token};
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct ArrayType<'a> {
    element_ty: Arc<Type<'a>>,
    count: Box<Expr<'a>>,
}

impl<'a> ArrayType<'a> {
    pub fn new(element_ty: Arc<Type<'a>>, count: Box<Expr<'a>>) -> Self {
        ArrayType { element_ty, count }
    }

    pub fn element_ty(&self) -> &Type<'a> {
        &self.element_ty
    }

    pub fn count(&self) -> &Expr<'a> {
        &self.count
    }
}

impl<'a> ToCode<'a> for ArrayType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Punctuation(Punctuation::LeftBracket));
        self.element_ty.to_code(tokens, options);
        tokens.push(Token::Punctuation(Punctuation::Semicolon));
        self.count.to_code(tokens, options);
        tokens.push(Token::Punctuation(Punctuation::RightBracket));
    }
}

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct ArrayTypeBuilder<'a> {
    element_ty: Option<Arc<Type<'a>>>,
    count: Option<Box<Expr<'a>>>,
}

impl<'a> ArrayTypeBuilder<'a> {
    pub fn with_element_ty(mut self, element_ty: Arc<Type<'a>>) -> Self {
        self.element_ty = Some(element_ty);
        self
    }

    pub fn with_count(mut self, count: Box<Expr<'a>>) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> ArrayType<'a> {
        let element_ty = self
            .element_ty
            .expect("ArrayTypeBuilder element type must be set");
        let count = self.count.expect("ArrayTypeBuilder count must be set");

        ArrayType::new(element_ty, count)
    }
}
