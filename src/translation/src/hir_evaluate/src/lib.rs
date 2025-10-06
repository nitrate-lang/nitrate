#![forbid(unsafe_code)]

mod eval;
mod expr;

pub use eval::{EvalFail, HirEvalCtx, HirEvaluate};
