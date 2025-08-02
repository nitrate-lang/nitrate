use super::function::FunctionParameter;
use super::storage::{ExprRef, TypeRef};

#[derive(Debug, Clone)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<TypeRef<'a>>,
    attributes: Vec<ExprRef<'a>>,
}

impl<'a> FunctionType<'a> {
    pub fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Option<TypeRef<'a>>,
        attributes: Vec<ExprRef<'a>>,
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

    pub fn return_type(&self) -> Option<TypeRef<'a>> {
        self.return_type
    }

    pub fn attributes(&self) -> &[ExprRef<'a>] {
        &self.attributes
    }
}
