#![forbid(unsafe_code)]

mod diagnosis;
mod expr;
mod item;
mod parse;
mod pat;
mod stmt;
mod ty;

pub use parse::Parser;
