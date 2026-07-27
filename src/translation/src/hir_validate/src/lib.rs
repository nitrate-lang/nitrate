#![forbid(unsafe_code)]

mod diagnosis;
mod expr;
mod item;
mod ty;
mod validate_hir;

pub use validate_hir::*;
