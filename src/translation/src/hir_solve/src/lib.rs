#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

mod bounds;
mod constraints;
mod diagnosis;
mod monomorphize;
mod solver;
mod substitution;

pub use solver::{resolve_function, resolve_global};

#[cfg(test)]
mod tests;
