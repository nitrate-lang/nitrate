use super::storage::ExprKey;
use std::collections::BTreeMap;

#[derive(Debug, Clone)]
pub struct ObjectLit<'a> {
    fields: BTreeMap<&'a str, ExprKey<'a>>,
}

impl<'a> ObjectLit<'a> {
    pub(crate) fn new(fields: BTreeMap<&'a str, ExprKey<'a>>) -> Self {
        ObjectLit { fields }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, ExprKey<'a>> {
        self.fields
    }

    pub fn get(&self) -> &BTreeMap<&'a str, ExprKey<'a>> {
        &self.fields
    }

    pub fn get_mut(&mut self) -> &mut BTreeMap<&'a str, ExprKey<'a>> {
        &mut self.fields
    }
}
