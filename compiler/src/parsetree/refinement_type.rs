use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct RefinementType<'a> {
    base: TypeKey<'a>,
    width: Option<ExprKey<'a>>,
    min: Option<ExprKey<'a>>,
    max: Option<ExprKey<'a>>,
}

impl<'a> RefinementType<'a> {
    #[must_use]
    pub(crate) fn new(
        base: TypeKey<'a>,
        width: Option<ExprKey<'a>>,
        min: Option<ExprKey<'a>>,
        max: Option<ExprKey<'a>>,
    ) -> Self {
        RefinementType {
            base,
            width,
            min,
            max,
        }
    }

    #[must_use]
    pub fn base(&self) -> TypeKey<'a> {
        self.base
    }

    #[must_use]
    pub fn width(&self) -> Option<ExprKey<'a>> {
        self.width
    }

    #[must_use]
    pub fn min(&self) -> Option<ExprKey<'a>> {
        self.min
    }

    #[must_use]
    pub fn max(&self) -> Option<ExprKey<'a>> {
        self.max
    }
}
