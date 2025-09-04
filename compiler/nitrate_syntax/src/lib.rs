#![warn(clippy::pedantic)]

mod expression;
mod parse;
mod preamble;
mod source_model;
mod symbol_table;
mod to_code;
mod type_system;

pub use parse::Parser;
pub use source_model::{CopyrightInfo, SourceModel};
pub use symbol_table::SymbolTable;
pub use to_code::*;
