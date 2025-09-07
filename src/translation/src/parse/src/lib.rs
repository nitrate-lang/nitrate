// #![warn(clippy::pedantic)]

mod expression;
mod parse;
mod preamble;
mod symbol_table;
mod to_code;
mod type_system;

pub use parse::Parser;
pub use symbol_table::SymbolTable;
pub use to_code::*;
