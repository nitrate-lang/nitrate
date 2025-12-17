use crate::prelude::*;
use nitrate_nstring::NString;
use ordered_float::OrderedFloat;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Expr {
    Trivia {
        trivia: Option<Trivia>,
    },

    Unit {
        source_offset: u32,
        trivia: [Option<Trivia>; 2],
    },

    Boolean {
        source_offset: u32,
        trivia: Option<Trivia>,
        value: bool,
    },

    Integer {
        source_offset: u32,
        trivia: Option<Trivia>,
        value: u128,
    },

    Float {
        source_offset: u32,
        trivia: Option<Trivia>,
        value: OrderedFloat<f64>,
        raw_value_str: String,
    },

    String {
        source_offset: u32,
        trivia: Option<Trivia>,
        value: NString,
        raw_value_str: String,
    },
}

impl From<Expr> for ExprId {
    fn from(expr: Expr) -> Self {
        get_storage(|store| store.store_expr(expr))
    }
}
