#![forbid(unsafe_code)]

mod diagnosis;
mod expr;
mod item;
mod parse;
mod resolve_import;
mod resolve_path;
mod symbol_table;
mod ty;

pub use parse::{Parser, ResolveCtx};
