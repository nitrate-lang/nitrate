use super::expression::Expr;
use crate::syntax::QualifiedScope;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct Scope<'a> {
    scope: QualifiedScope<'a>,
    attributes: Vec<Arc<Expr<'a>>>,
    block: Arc<Expr<'a>>,
}

impl<'a> Scope<'a> {
    #[must_use]
    pub fn new(
        scope: QualifiedScope<'a>,
        attributes: Vec<Arc<Expr<'a>>>,
        block: Arc<Expr<'a>>,
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
    pub fn attributes(&self) -> &[Arc<Expr<'a>>] {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn block(&self) -> Arc<Expr<'a>> {
        self.block.clone()
    }

    pub fn set_block(&mut self, block: Arc<Expr<'a>>) {
        self.block = block;
    }
}
