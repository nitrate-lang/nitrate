#![forbid(unsafe_code)]
#![allow(clippy::wrong_self_convention)]

mod eval;
mod expr;

pub use eval::{HirEvalCtx, HirEvaluate, Unwind};
