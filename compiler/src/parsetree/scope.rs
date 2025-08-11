use super::storage::ExprKey;

#[derive(Debug, Clone)]
pub struct Scope<'a> {
    name: &'a str,
    attributes: Vec<ExprKey<'a>>,
    block: ExprKey<'a>,
}

impl<'a> Scope<'a> {
    #[must_use]
    pub fn new(name: &'a str, attributes: Vec<ExprKey<'a>>, block: ExprKey<'a>) -> Self {
        Scope {
            name,
            attributes,
            block,
        }
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
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
