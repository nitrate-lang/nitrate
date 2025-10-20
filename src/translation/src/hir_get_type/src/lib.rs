#![forbid(unsafe_code)]

mod get_type;

pub use get_type::{TypeInferenceCtx, TypeInferenceError, get_type};
