#![forbid(unsafe_code)]

mod codegen;
mod expr;
mod item;
mod ty;

pub use codegen::generate_code;
