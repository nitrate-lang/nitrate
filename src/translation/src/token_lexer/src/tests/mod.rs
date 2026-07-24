//! Nitrate Lexer unit tests — organized by category.
//!
//! Each category of tests lives in its own file under `src/tests/`.
//! Shared test helpers are in `helpers.rs`.
mod helpers;
pub(crate) use helpers::*;

mod comments;
mod errors;
mod fileid;
mod float_literals;
mod identifiers;
mod integer_literals;
mod iterator;
mod keywords;
mod misc;
mod navigation;
mod programs;
mod punctuation;
mod source_size;
mod string_literals;
mod trivia;
