use crate::syntax::QualifiedScope;

use super::storage::ExprKey;

#[derive(Debug, Clone)]
pub struct Scope<'a> {
    scope: QualifiedScope<'a>,
    attributes: Vec<ExprKey<'a>>,
    block: ExprKey<'a>,
}

impl<'a> Scope<'a> {
    #[must_use]
    pub fn new(
        scope: QualifiedScope<'a>,
        attributes: Vec<ExprKey<'a>>,
        block: ExprKey<'a>,
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
    pub fn attributes(&self) -> &[ExprKey<'a>] {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<ExprKey<'a>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn block(&self) -> ExprKey<'a> {
        self.block
    }

    pub fn set_block(&mut self, block: ExprKey<'a>) {
        self.block = block;
    }
}
