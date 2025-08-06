use super::block::Block;
use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunctionParameter<'a> {
    name: &'a str,
    param_type: Option<TypeKey<'a>>,
    default_value: Option<ExprKey<'a>>,
}

impl<'a> FunctionParameter<'a> {
    pub fn new(
        name: &'a str,
        param_type: Option<TypeKey<'a>>,
        default_value: Option<ExprKey<'a>>,
    ) -> Self {
        FunctionParameter {
            name,
            param_type,
            default_value,
        }
    }

    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn param_type(&self) -> Option<TypeKey<'a>> {
        self.param_type
    }

    pub fn default_value(&self) -> Option<ExprKey<'a>> {
        self.default_value
    }
}

#[derive(Debug, Clone)]
pub struct Function<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<TypeKey<'a>>,
    attributes: Vec<ExprKey<'a>>,
    name: &'a str,
    definition: Option<Block<'a>>,
}

impl<'a> Function<'a> {
    pub(crate) fn new(
        name: &'a str,
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Option<TypeKey<'a>>,
        attributes: Vec<ExprKey<'a>>,
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

    pub fn parameters(&self) -> &[FunctionParameter<'a>] {
        &self.parameters
    }

    pub fn parameters_mut(&mut self) -> &mut Vec<FunctionParameter<'a>> {
        &mut self.parameters
    }

    pub fn return_type(&self) -> Option<TypeKey<'a>> {
        self.return_type
    }

    pub fn set_return_type(&mut self, ty: Option<TypeKey<'a>>) {
        self.return_type = ty;
    }

    pub fn attributes(&self) -> &[ExprKey<'a>] {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<ExprKey<'a>> {
        &mut self.attributes
    }

    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    pub fn definition(&self) -> Option<&Block<'a>> {
        self.definition.as_ref()
    }

    pub fn definition_mut(&mut self) -> Option<&mut Block<'a>> {
        self.definition.as_mut()
    }
}
