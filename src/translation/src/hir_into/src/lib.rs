#![forbid(unsafe_code)]

mod diagnosis;
mod into;
mod lower;

pub use into::{ast_expr2hir, ast_mod2hir, ast_type2hir};
