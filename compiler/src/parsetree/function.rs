use super::expression::{ExprOwned, TypeOwned};
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct FunctionParameter<'a> {
    name: &'a str,
    param_type: Rc<TypeOwned<'a>>,
    default_value: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> FunctionParameter<'a> {
    #[must_use]
    pub fn new(
        name: &'a str,
        param_type: Rc<TypeOwned<'a>>,
        default_value: Option<Arc<ExprOwned<'a>>>,
    ) -> Self {
        FunctionParameter {
            name,
            param_type,
            default_value,
        }
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    #[must_use]
    pub fn type_(&self) -> Rc<TypeOwned<'a>> {
        self.param_type.clone()
    }

    #[must_use]
    pub fn default(&self) -> Option<Arc<ExprOwned<'a>>> {
        self.default_value.clone()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Function<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Rc<TypeOwned<'a>>,
    attributes: Vec<Arc<ExprOwned<'a>>>,
    name: &'a str,
    definition: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> Function<'a> {
    #[must_use]
    pub(crate) fn new(
        name: &'a str,
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Rc<TypeOwned<'a>>,
        attributes: Vec<Arc<ExprOwned<'a>>>,
        definition: Option<Arc<ExprOwned<'a>>>,
    ) -> Self {
        Function {
            parameters,
            return_type,
            attributes,
            name,
            definition,
        }
    }

    #[must_use]
    pub fn parameters(&self) -> &[FunctionParameter<'a>] {
        &self.parameters
    }

    #[must_use]
    pub fn parameters_mut(&mut self) -> &mut Vec<FunctionParameter<'a>> {
        &mut self.parameters
    }

    #[must_use]
    pub fn return_type(&self) -> Rc<TypeOwned<'a>> {
        self.return_type.clone()
    }

    pub fn set_return_type(&mut self, ty: Rc<TypeOwned<'a>>) {
        self.return_type = ty;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<ExprOwned<'a>>] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Arc<ExprOwned<'a>>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    #[must_use]
    pub fn definition(&self) -> Option<&Arc<ExprOwned<'a>>> {
        self.definition.as_ref()
    }

    pub fn set_definition(&mut self, definition: Option<Arc<ExprOwned<'a>>>) {
        self.definition = definition;
    }
}
