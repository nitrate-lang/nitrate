#![forbid(unsafe_code)]

mod context;
mod diagnosis;
mod into;
mod lower;

pub use context::Ast2HirCtx;
pub use into::{ast_expr2hir, ast_mod2hir, ast_type2hir};
