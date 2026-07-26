#![forbid(unsafe_code)]

mod diagnosis;
mod monomorphize;
mod solver;
mod substitution;

pub use solver::{resolve_function, resolve_global};
