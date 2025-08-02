use super::expression::Expr;
use super::function::FunctionParameter;
use super::types::Type;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Box<Type<'a>>>,
    attributes: Vec<Box<Expr<'a>>>,
}

impl<'a> FunctionType<'a> {
    pub fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Option<Box<Type<'a>>>,
        attributes: Vec<Box<Expr<'a>>>,
    ) -> Self {
        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    pub fn parameters(&self) -> &Vec<FunctionParameter<'a>> {
        &self.parameters
    }

    pub fn return_type(&self) -> Option<&Box<Type<'a>>> {
        self.return_type.as_ref()
    }

    pub fn attributes(&self) -> &Vec<Box<Expr<'a>>> {
        &self.attributes
    }
}
