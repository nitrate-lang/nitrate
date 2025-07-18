use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::Token;

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct FunctionType<'a> {
    parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
    return_type: Option<Box<Type<'a>>>,
    attributes: Vec<Expr<'a>>,
}

impl<'a> FunctionType<'a> {
    pub fn new(
        parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
        return_type: Option<Box<Type<'a>>>,
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

    pub fn parameters_mut(&mut self) -> &mut Vec<(&'a str, Type<'a>, Option<Expr<'a>>)> {
        &mut self.parameters
    }

    pub fn return_type(&self) -> Option<&Type<'a>> {
        self.return_type.as_deref()
    }

    pub fn return_type_mut(&mut self) -> Option<&mut Type<'a>> {
        self.return_type.as_deref_mut()
    }

    pub fn attributes(&self) -> &Vec<Expr<'a>> {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.attributes
    }
}

impl<'a> ToCode<'a> for FunctionType<'a> {
    fn to_code(&self, _tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
        // TODO: Implement function type code generation
    }
}
