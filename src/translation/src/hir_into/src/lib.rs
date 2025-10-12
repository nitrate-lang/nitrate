#![forbid(unsafe_code)]

mod diagnosis;
mod expr;
mod item;
mod lower;
mod ty;

pub use lower::{HirCtx, TryIntoHir};
