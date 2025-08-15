use super::expression::{Expr, Type};
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct RefinementType<'a> {
    base: Rc<Type<'a>>,
    width: Option<Arc<Expr<'a>>>,
    min: Option<Arc<Expr<'a>>>,
    max: Option<Arc<Expr<'a>>>,
}

impl<'a> RefinementType<'a> {
    #[must_use]
    pub(crate) fn new(
        base: Rc<Type<'a>>,
        width: Option<Arc<Expr<'a>>>,
        min: Option<Arc<Expr<'a>>>,
        max: Option<Arc<Expr<'a>>>,
    ) -> Self {
        RefinementType {
            base,
            width,
            min,
            max,
        }
    }

    #[must_use]
    pub fn base(&self) -> Rc<Type<'a>> {
        self.base.clone()
    }

    #[must_use]
    pub fn width(&self) -> Option<Arc<Expr<'a>>> {
        self.width.clone()
    }

    #[must_use]
    pub fn min(&self) -> Option<Arc<Expr<'a>>> {
        self.min.clone()
    }

    #[must_use]
    pub fn max(&self) -> Option<Arc<Expr<'a>>> {
        self.max.clone()
    }
}
