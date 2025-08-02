use super::storage::ExprRef;
use std::collections::BTreeMap;

#[derive(Debug, Clone)]
pub struct Object<'a> {
    fields: BTreeMap<&'a str, ExprRef<'a>>,
}

impl<'a> Object<'a> {
    pub fn new(fields: BTreeMap<&'a str, ExprRef<'a>>) -> Self {
        Object { fields }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, ExprRef<'a>> {
        self.fields
    }

    pub fn get(&self) -> &BTreeMap<&'a str, ExprRef<'a>> {
        &self.fields
    }

    pub fn get_mut(&mut self) -> &mut BTreeMap<&'a str, ExprRef<'a>> {
        &mut self.fields
    }
}
