#![forbid(unsafe_code)]

mod resolve;
mod symbol_table;

pub use resolve::resolve;
pub use symbol_table::{Symbol, SymbolTable, build_symbol_table};
