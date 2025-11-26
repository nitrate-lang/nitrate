use crate::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Expr {
    Trivia {
        trivia: Option<Trivia>,
    },

    Integer {
        source_offset: u32,
        trivia: Option<Trivia>,
        value: u128,
    },

    Boolean {
        source_offset: u32,
        trivia: Option<Trivia>,
        value: bool,
    },
}

impl From<Expr> for ExprId {
    fn from(expr: Expr) -> Self {
        get_storage(|store| store.store_expr(expr))
    }
}
