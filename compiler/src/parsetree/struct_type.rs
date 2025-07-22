use super::expression::Expr;
use super::types::Type;
use std::collections::BTreeMap;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StructType<'a> {
    name: Option<&'a str>,
    attributes: Vec<Expr<'a>>,
    fields: BTreeMap<&'a str, Arc<Type<'a>>>,
}

impl<'a> StructType<'a> {
    pub fn new(
        name: Option<&'a str>,
        attributes: Vec<Expr<'a>>,
        fields: BTreeMap<&'a str, Arc<Type<'a>>>,
    ) -> Self {
        StructType {
            name,
            attributes,
            fields,
        }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Arc<Type<'a>>> {
        self.fields
    }

    pub fn name(&self) -> Option<&'a str> {
        self.name
    }

    pub fn attributes(&self) -> &[Expr<'a>] {
        &self.attributes
    }

    pub fn fields(&self) -> &BTreeMap<&'a str, Arc<Type<'a>>> {
        &self.fields
    }
}
