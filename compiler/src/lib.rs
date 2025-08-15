#![warn(clippy::pedantic)]
#![allow(clippy::inline_always)]

pub mod evaluate;
pub mod lexer;
pub mod parser;
pub mod parsetree;
pub mod symbol_resolution;
pub mod type_inference;
