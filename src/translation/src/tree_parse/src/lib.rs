#![forbid(unsafe_code)]

mod diagnosis;
mod expr;
mod helper;
mod item;
mod parse;
mod ty;

#[cfg(test)]
mod tests;

pub use parse::{Parser, ResolveCtx};
