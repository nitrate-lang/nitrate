use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Identifier, Keyword, Operator, Punctuation, Token};
use crate::parsetree::InnerType;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct FunctionType<'a> {
    parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
    return_type: Option<&'a Type<'a>>,
    attributes: Vec<Expr<'a>>,
}

impl<'a> FunctionType<'a> {
    pub fn new(
        parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
        return_type: Option<&'a Type<'a>>,
        attributes: Vec<Expr<'a>>,
    ) -> Self {
        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    pub fn parameters(&self) -> &Vec<(&'a str, Type<'a>, Option<Expr<'a>>)> {
        &self.parameters
    }

    pub fn return_type(&self) -> Option<&Type<'a>> {
        self.return_type.as_deref()
    }

    pub fn attributes(&self) -> &Vec<Expr<'a>> {
        &self.attributes
    }
}

impl<'a> ToCode<'a> for FunctionType<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punctuation(Punctuation::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
                attr.to_code(tokens, options);
            }
            tokens.push(Token::Punctuation(Punctuation::RightBracket));
        }

        tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));

            tokens.push(Token::Identifier(Identifier::new(name)));

            if !matches!(**ty, InnerType::InferType) {
                tokens.push(Token::Punctuation(Punctuation::Colon));
                ty.to_code(tokens, options);
            }

            if let Some(default_expr) = default {
                tokens.push(Token::Operator(Operator::Set));
                default_expr.to_code(tokens, options);
            }
        }
        tokens.push(Token::Punctuation(Punctuation::RightParenthesis));

        if let Some(return_type) = self.return_type() {
            if !matches!(**return_type, InnerType::InferType) {
                tokens.push(Token::Punctuation(Punctuation::Colon));
                return_type.to_code(tokens, options);
            }
        }
    }
}
