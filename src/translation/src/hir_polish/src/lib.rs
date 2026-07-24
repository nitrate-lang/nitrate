#![forbid(unsafe_code)]

mod diagnosis;
mod hindley_milner;

pub use hindley_milner::{resolve_function, resolve_global};
