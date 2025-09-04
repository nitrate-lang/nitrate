#![warn(clippy::pedantic)]
#![allow(clippy::too_many_lines)]
#![allow(clippy::inline_always)]

mod lex;
mod token;

pub use lex::*;
pub use token::*;
