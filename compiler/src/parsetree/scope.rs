use super::expression::ExprOwned;
use crate::syntax::QualifiedScope;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct Scope<'a> {
    scope: QualifiedScope<'a>,
    attributes: Vec<Arc<ExprOwned<'a>>>,
    block: Arc<ExprOwned<'a>>,
}

impl<'a> Scope<'a> {
    #[must_use]
    pub fn new(
        scope: QualifiedScope<'a>,
        attributes: Vec<Arc<ExprOwned<'a>>>,
        block: Arc<ExprOwned<'a>>,
    ) -> Self {
        Scope {
            scope,
            attributes,
            block,
        }
    }

    #[must_use]
    pub fn scope(&self) -> &QualifiedScope<'a> {
        &self.scope
    }

    pub fn set_scope(&mut self, scope: QualifiedScope<'a>) {
        self.scope = scope;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<ExprOwned<'a>>] {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Arc<ExprOwned<'a>>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn block(&self) -> Arc<ExprOwned<'a>> {
        self.block.clone()
    }

    pub fn set_block(&mut self, block: Arc<ExprOwned<'a>>) {
        self.block = block;
    }
}
