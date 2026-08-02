#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

mod bounds;
mod constraints;
mod diagnosis;
mod monomorphize;
mod range;
mod solve;
mod substitution;

pub use solve::{resolve_function, resolve_global};
