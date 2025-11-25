#![forbid(unsafe_code)]

mod diagnosis;
mod hindley_milner;
mod resolve;

pub use resolve::TyCtx;
