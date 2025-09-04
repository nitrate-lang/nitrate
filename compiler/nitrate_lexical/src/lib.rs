#![warn(clippy::pedantic)]
#![allow(clippy::inline_always)]

mod lex;
mod token;

pub use lex::*;
pub use token::*;
