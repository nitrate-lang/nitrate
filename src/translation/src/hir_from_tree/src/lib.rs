#![forbid(unsafe_code)]
#![allow(unused)]

mod context;
mod diagnosis;
mod expr;
mod helpers;
mod item;
mod lower;
mod ty;

#[cfg(test)]
mod tests;

pub use context::Ast2HirCtx;
pub use lower::convert_ast_to_hir;
