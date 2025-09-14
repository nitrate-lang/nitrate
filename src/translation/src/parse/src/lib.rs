// #![warn(clippy::pedantic)]

mod expr;
mod item;
mod parse;
mod symbol_table;
mod ty;

pub use parse::Parser;
pub use symbol_table::SymbolTable;
