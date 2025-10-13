#![forbid(unsafe_code)]

mod diagnosis;
mod expr;
mod item;
mod lower;
mod prepare;
mod ty;

pub use lower::TryIntoHir;
pub use prepare::{from_nitrate_expression, from_nitrate_type};
