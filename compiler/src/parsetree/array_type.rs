use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use crate::lexer::{Punctuation, Token};

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct ArrayType<'a> {
    element_ty: Box<Expr<'a>>,
    count: Box<Expr<'a>>,
}

impl<'a> ArrayType<'a> {
    pub fn new(element_ty: Box<Expr<'a>>, count: Box<Expr<'a>>) -> Option<Self> {
        element_ty
            .is_type()
            .then(|| ArrayType { element_ty, count })
    }

    pub fn element_ty(&self) -> &Expr<'a> {
        &self.element_ty
    }

    pub fn element_ty_mut(&mut self) -> &mut Expr<'a> {
        &mut self.element_ty
    }

    pub fn count(&self) -> &Expr<'a> {
        &self.count
    }

    pub fn count_mut(&mut self) -> &mut Expr<'a> {
        &mut self.count
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
