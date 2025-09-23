#![forbid(unsafe_code)]

mod diagnosis;
mod expr;
mod item;
mod parse;
mod ty;

pub use parse::Parser;
