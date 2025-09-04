#![warn(clippy::pedantic)]
#![allow(clippy::inline_always)]

pub mod lex;
pub mod token;

pub use lex::*;
pub use token::*;
