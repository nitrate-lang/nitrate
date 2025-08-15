use super::expression::Expr;
use std::collections::BTreeMap;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct ObjectLit<'a> {
    fields: BTreeMap<&'a str, Arc<Expr<'a>>>,
}

impl<'a> ObjectLit<'a> {
    #[must_use]
    pub(crate) fn new(fields: BTreeMap<&'a str, Arc<Expr<'a>>>) -> Self {
        ObjectLit { fields }
    }

    #[must_use]
    pub fn into_inner(self) -> BTreeMap<&'a str, Arc<Expr<'a>>> {
        self.fields
    }

    #[must_use]
    pub fn get(&self) -> &BTreeMap<&'a str, Arc<Expr<'a>>> {
        &self.fields
    }

    #[must_use]
    pub fn get_mut(&mut self) -> &mut BTreeMap<&'a str, Arc<Expr<'a>>> {
        &mut self.fields
    }
}
