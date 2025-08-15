use super::expression::{Expr, Type};
use super::function::FunctionParameter;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Type<'a>,
    attributes: Vec<Arc<Expr<'a>>>,
}

impl<'a> FunctionType<'a> {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Type<'a>,
        attributes: Vec<Arc<Expr<'a>>>,
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
    pub fn return_type(&self) -> Type<'a> {
        self.return_type.clone()
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<Expr<'a>>] {
        &self.attributes
    }
}
