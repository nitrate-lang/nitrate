use super::expression::{ExprOwned, TypeOwned};
use super::function::FunctionParameter;
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct FunctionType<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Rc<TypeOwned<'a>>,
    attributes: Vec<Arc<ExprOwned<'a>>>,
}

impl<'a> FunctionType<'a> {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Rc<TypeOwned<'a>>,
        attributes: Vec<Arc<ExprOwned<'a>>>,
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
    pub fn return_type(&self) -> Rc<TypeOwned<'a>> {
        self.return_type.clone()
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<ExprOwned<'a>>] {
        &self.attributes
    }
}
