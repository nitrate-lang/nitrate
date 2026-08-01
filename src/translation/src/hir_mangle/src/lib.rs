#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

mod mangle;
mod pass;

pub mod string;
pub mod ty;

pub use mangle::*;
pub use pass::*;
