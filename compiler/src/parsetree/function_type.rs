use super::function::FunctionParameter;
use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: TypeKey<'a>,
    attributes: Vec<ExprKey<'a>>,
}

impl<'a> FunctionType<'a> {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: TypeKey<'a>,
        attributes: Vec<ExprKey<'a>>,
    ) -> Self {
        FunctionType {
            parameters,
            return_type,
            attributes,
        }
    }

    #[must_use]
    pub fn parameters(&self) -> &[FunctionParameter<'a>] {
        &self.parameters
    }

    #[must_use]
    pub fn return_type(&self) -> TypeKey<'a> {
        self.return_type
    }

    #[must_use]
    pub fn attributes(&self) -> &[ExprKey<'a>] {
        &self.attributes
    }
}
