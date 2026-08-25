#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

mod diagnosis;
mod expr;
mod item;
mod ty;
mod validate_hir;

pub use validate_hir::*;
