use super::storage::{ExprKey, TypeKey};
use std::collections::BTreeMap;

#[derive(Debug, Clone)]
pub struct StructType<'a> {
    name: Option<&'a str>,
    attributes: Vec<ExprKey<'a>>,
    fields: BTreeMap<&'a str, TypeKey<'a>>,
}

impl<'a> StructType<'a> {
    pub fn new(
        name: Option<&'a str>,
        attributes: Vec<ExprKey<'a>>,
        fields: BTreeMap<&'a str, TypeKey<'a>>,
    ) -> Self {
        StructType {
            name,
            attributes,
            fields,
        }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, TypeKey<'a>> {
        self.fields
    }

    pub fn name(&self) -> Option<&'a str> {
        self.name
    }

    pub fn attributes(&self) -> &[ExprKey<'a>] {
        &self.attributes
    }

    pub fn fields(&self) -> &BTreeMap<&'a str, TypeKey<'a>> {
        &self.fields
    }
}
