#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]
#![allow(clippy::too_many_lines)]
#![allow(clippy::inline_always)]

mod file_id;
mod lex;
mod token;

pub use file_id::*;
pub use lex::*;
pub use token::*;
