#![forbid(unsafe_code)]

mod eval;
mod expr;

pub use eval::{HirEvalCtx, HirEvaluate, Unwind};
