use super::block::Block;
use super::expression::{Expr, Type};

pub type FunctionParameter<'a> = (&'a str, Option<Box<Type<'a>>>, Option<Box<Expr<'a>>>);

#[derive(Debug, Clone)]
pub struct Function<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Box<Type<'a>>>,
    attributes: Vec<Box<Expr<'a>>>,
    name: &'a str,
    definition: Option<Block<'a>>,
}

impl<'a> Function<'a> {
    pub fn new(
        name: &'a str,
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Option<Box<Type<'a>>>,
        attributes: Vec<Box<Expr<'a>>>,
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

    pub fn parameters(&self) -> &Vec<FunctionParameter<'a>> {
        &self.parameters
    }

    pub fn parameters_mut(&mut self) -> &mut Vec<FunctionParameter<'a>> {
        &mut self.parameters
    }

    pub fn return_type(&self) -> Option<&Box<Type<'a>>> {
        self.return_type.as_ref()
    }

    pub fn set_return_type(&mut self, ty: Option<Box<Type<'a>>>) {
        self.return_type = ty;
    }

    pub fn attributes(&self) -> &Vec<Box<Expr<'a>>> {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Box<Expr<'a>>> {
        &mut self.attributes
    }

    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn definition(&self) -> Option<&Block<'a>> {
        self.definition.as_ref()
    }

    pub fn definition_mut(&mut self) -> Option<&mut Block<'a>> {
        self.definition.as_mut()
    }
}
