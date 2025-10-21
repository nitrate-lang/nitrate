#![forbid(unsafe_code)]

mod context;
mod diagnosis;
mod into;
mod lower;
mod put_defaults;

pub use context::Ast2HirCtx;
pub use into::ast_mod2hir;
