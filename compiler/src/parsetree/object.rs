use super::expression::Expr;
use std::collections::BTreeMap;

#[derive(Debug, Clone)]
pub struct Object<'a> {
    fields: BTreeMap<&'a str, Box<Expr<'a>>>,
}

impl<'a> Object<'a> {
    pub fn new(fields: BTreeMap<&'a str, Box<Expr<'a>>>) -> Self {
        Object { fields }
    }

    pub fn into_inner(self) -> BTreeMap<&'a str, Box<Expr<'a>>> {
        self.fields
    }

    pub fn get(&self) -> &BTreeMap<&'a str, Box<Expr<'a>>> {
        &self.fields
    }

    pub fn get_mut(&mut self) -> &mut BTreeMap<&'a str, Box<Expr<'a>>> {
        &mut self.fields
    }
}
