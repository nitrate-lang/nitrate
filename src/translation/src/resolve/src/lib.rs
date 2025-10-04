#![forbid(unsafe_code)]

mod diagnosis;
mod import;
mod name;
mod symbol_table;

pub use diagnosis::ResolveIssue;
pub use import::{ImportContext, resolve_imports};
pub use name::resolve_names;
pub use symbol_table::{Symbol, SymbolName, SymbolTable, build_symbol_table};
