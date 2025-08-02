use super::expression::{Expr, Type};
use std::collections::BTreeMap;

#[derive(Debug, Clone)]
pub struct StructType<'a> {
    name: Option<&'a str>,
    attributes: Vec<Box<Expr<'a>>>,
    fields: BTreeMap<&'a str, Box<Type<'a>>>,
}

impl<'a> StructType<'a> {
    pub fn new(
        name: Option<&'a str>,
        attributes: Vec<Box<Expr<'a>>>,
        fields: BTreeMap<&'a str, Box<Type<'a>>>,
    ) -> Self {
        StructType {
            name,
            attributes,
            fields,
        }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Box<Type<'a>>> {
        self.fields
    }

    pub fn name(&self) -> Option<&'a str> {
        self.name
    }

    pub fn attributes(&self) -> &[Box<Expr<'a>>] {
        &self.attributes
    }

    pub fn fields(&self) -> &BTreeMap<&'a str, Box<Type<'a>>> {
        &self.fields
    }
}
