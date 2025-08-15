use super::expression::{ExprOwned, TypeOwned};
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct RefinementType<'a> {
    base: Rc<TypeOwned<'a>>,
    width: Option<Arc<ExprOwned<'a>>>,
    min: Option<Arc<ExprOwned<'a>>>,
    max: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> RefinementType<'a> {
    #[must_use]
    pub(crate) fn new(
        base: Rc<TypeOwned<'a>>,
        width: Option<Arc<ExprOwned<'a>>>,
        min: Option<Arc<ExprOwned<'a>>>,
        max: Option<Arc<ExprOwned<'a>>>,
    ) -> Self {
        RefinementType {
            base,
            width,
            min,
            max,
        }
    }

    #[must_use]
    pub fn base(&self) -> Rc<TypeOwned<'a>> {
        self.base.clone()
    }

    #[must_use]
    pub fn width(&self) -> Option<Arc<ExprOwned<'a>>> {
        self.width.clone()
    }

    #[must_use]
    pub fn min(&self) -> Option<Arc<ExprOwned<'a>>> {
        self.min.clone()
    }

    #[must_use]
    pub fn max(&self) -> Option<Arc<ExprOwned<'a>>> {
        self.max.clone()
    }
}
