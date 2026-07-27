#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

mod diagnosis;
mod monomorphize;
mod solver;
mod substitution;

pub use solver::{resolve_function, resolve_global};
