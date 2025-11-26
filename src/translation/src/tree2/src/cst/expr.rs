use crate::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Expr {
    Placeholder,
}

impl From<Expr> for ExprId {
    fn from(expr: Expr) -> Self {
        get_storage(|store| store.store_expr(expr))
    }
}
