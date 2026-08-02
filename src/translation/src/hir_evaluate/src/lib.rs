#![forbid(unsafe_code)]

mod builtins;
mod error;
mod evaluator;
mod memory;
mod value;

pub use builtins::DEFAULT_BUILTIN_FUNCTIONS;
pub use error::EvalError;
pub use evaluator::{BuiltinFn, Evaluator};
pub use memory::Memory;
