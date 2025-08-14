use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunctionParameter<'a> {
    name: &'a str,
    param_type: TypeKey<'a>,
    default_value: Option<ExprKey<'a>>,
}

impl<'a> FunctionParameter<'a> {
    #[must_use]
    pub fn new(name: &'a str, param_type: TypeKey<'a>, default_value: Option<ExprKey<'a>>) -> Self {
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
    pub fn param_type(&self) -> TypeKey<'a> {
        self.param_type
    }

    #[must_use]
    pub fn default_value(&self) -> Option<ExprKey<'a>> {
        self.default_value
    }
}

#[derive(Debug, Clone)]
pub struct Function<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: TypeKey<'a>,
    attributes: Vec<ExprKey<'a>>,
    name: &'a str,
    definition: Option<ExprKey<'a>>,
}

impl<'a> Function<'a> {
    #[must_use]
    pub(crate) fn new(
        name: &'a str,
        parameters: Vec<FunctionParameter<'a>>,
        return_type: TypeKey<'a>,
        attributes: Vec<ExprKey<'a>>,
        definition: Option<ExprKey<'a>>,
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
    pub fn return_type(&self) -> TypeKey<'a> {
        self.return_type
    }

    pub fn set_return_type(&mut self, ty: TypeKey<'a>) {
        self.return_type = ty;
    }

    #[must_use]
    pub fn attributes(&self) -> &[ExprKey<'a>] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<ExprKey<'a>> {
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
    pub fn definition(&self) -> Option<&ExprKey<'a>> {
        self.definition.as_ref()
    }

    pub fn set_definition(&mut self, definition: Option<ExprKey<'a>>) {
        self.definition = definition;
    }
}
