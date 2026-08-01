#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

mod bounds;
mod constraints;
mod diagnosis;
mod monomorphize;
mod range;
mod solver;
mod substitution;

pub use range::{ensure_range_structs, range_struct_name};
pub use solver::{resolve_function, resolve_global};

#[cfg(test)]
mod tests;
