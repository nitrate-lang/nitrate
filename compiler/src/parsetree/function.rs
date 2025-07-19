use super::block::Block;
use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use super::types::InnerType;
use super::types::Type;
use crate::lexer::{Identifier, Keyword, Operator, Punctuation, Token};
use std::sync::Arc;

pub type FunctionParameter<'a> = (&'a str, Arc<Type<'a>>, Option<Box<Expr<'a>>>);

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Function<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Arc<Type<'a>>>,
    attributes: Vec<Expr<'a>>,
    name: &'a str,
    definition: Option<Block<'a>>,
}

impl<'a> Function<'a> {
    pub fn new(
        name: &'a str,
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Option<Arc<Type<'a>>>,
        attributes: Vec<Expr<'a>>,
        definition: Option<Block<'a>>,
    ) -> Self {
        Function {
            name,
            parameters,
            return_type,
            attributes,
            definition: definition,
        }
    }

    pub fn parameters(&self) -> &Vec<FunctionParameter<'a>> {
        &self.parameters
    }

    pub fn parameters_mut(&mut self) -> &mut Vec<FunctionParameter<'a>> {
        &mut self.parameters
    }

    pub fn return_type(&self) -> Option<&Arc<Type<'a>>> {
        self.return_type.as_ref()
    }

    pub fn set_return_type(&mut self, ty: Option<Arc<Type<'a>>>) {
        self.return_type = ty;
    }

    pub fn attributes(&self) -> &Vec<Expr<'a>> {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.attributes
    }

    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn definition(&self) -> Option<&Block<'a>> {
        self.definition.as_ref()
    }

    pub fn definition_mut(&mut self) -> Option<&mut Block<'a>> {
        self.definition.as_mut()
    }
}

impl<'a> ToCode<'a> for Function<'a> {
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

        if !self.name().is_empty() {
            tokens.push(Token::Identifier(Identifier::new(self.name())));
        }

        tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));

            tokens.push(Token::Identifier(Identifier::new(name)));

            if !matches!(***ty, InnerType::InferType) {
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
            if !matches!(***return_type, InnerType::InferType) {
                tokens.push(Token::Punctuation(Punctuation::Colon));
                return_type.to_code(tokens, options);
            }
        }

        if let Some(definition) = self.definition() {
            definition.to_code(tokens, options);
        }
    }
}
