#![forbid(unsafe_code)]
#![allow(clippy::too_many_lines)]
#![allow(clippy::inline_always)]

//! # Nitrate Token Definitions
//!
//! Defines the lexical token types produced by the lexer and consumed
//! by the parser. Each token carries source location information
//! (byte span) for error reporting.
//!
//! ## Token Categories
//!
//! - **Literals**: Integer, float, string, byte-string, character literals.
//! - **Keywords**: Language keywords (`fn`, `let`, `if`, `else`, `struct`, etc.).
//! - **Identifiers**: User-defined names for variables, functions, types.
//! - **Operators**: Arithmetic, comparison, logical, bitwise, assignment operators.
//! - **Delimiters**: Parentheses, braces, brackets, commas, semicolons.
//! - **Trivia**: Whitespace and comments (stored as trivia rather than tokens).
//!
//! ## Key Types
//!
//! - [`Token`]: The core token enum with variants for every lexical element.
//! - [`AnnotatedToken`]: A token paired with its source byte span.
//! - [`TokenInfo`]: Metadata about a token (used for pretty-printing and LSP).

mod token;

pub use token::*;
