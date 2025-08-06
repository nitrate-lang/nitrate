use super::function::FunctionParameter;
use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<TypeKey<'a>>,
    attributes: Vec<ExprKey<'a>>,
}

impl<'a> FunctionType<'a> {
    pub(crate) fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Option<TypeKey<'a>>,
        attributes: Vec<ExprKey<'a>>,
    ) -> Self {
        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    pub fn parameters(&self) -> &[FunctionParameter<'a>] {
        &self.parameters
    }

    pub fn return_type(&self) -> Option<TypeKey<'a>> {
        self.return_type
    }

    pub fn attributes(&self) -> &[ExprKey<'a>] {
        &self.attributes
    }
}
