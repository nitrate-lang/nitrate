use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone)]
pub struct RefinementType<'a> {
    principal: TypeKey<'a>,
    width: Option<ExprKey<'a>>,
    min: Option<ExprKey<'a>>,
    max: Option<ExprKey<'a>>,
}

impl<'a> RefinementType<'a> {
    pub fn new(
        principal: TypeKey<'a>,
        width: Option<ExprKey<'a>>,
        min: Option<ExprKey<'a>>,
        max: Option<ExprKey<'a>>,
    ) -> Self {
        RefinementType {
            principal,
            width,
            min,
            max,
        }
    }

    pub fn principal(&self) -> TypeKey<'a> {
        self.principal
    }

    pub fn width(&self) -> Option<ExprKey<'a>> {
        self.width
    }

    pub fn min(&self) -> Option<ExprKey<'a>> {
        self.min
    }

    pub fn max(&self) -> Option<ExprKey<'a>> {
        self.max
    }
}
