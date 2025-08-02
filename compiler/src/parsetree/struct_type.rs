use super::storage::{ExprRef, TypeRef};
use std::collections::BTreeMap;

#[derive(Debug, Clone)]
pub struct StructType<'a> {
    name: Option<&'a str>,
    attributes: Vec<ExprRef<'a>>,
    fields: BTreeMap<&'a str, TypeRef<'a>>,
}

impl<'a> StructType<'a> {
    pub fn new(
        name: Option<&'a str>,
        attributes: Vec<ExprRef<'a>>,
        fields: BTreeMap<&'a str, TypeRef<'a>>,
    ) -> Self {
        StructType {
            name,
            attributes,
            fields,
        }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, TypeRef<'a>> {
        self.fields
    }

    pub fn name(&self) -> Option<&'a str> {
        self.name
    }

    pub fn attributes(&self) -> &[ExprRef<'a>] {
        &self.attributes
    }

    pub fn fields(&self) -> &BTreeMap<&'a str, TypeRef<'a>> {
        &self.fields
    }
}
