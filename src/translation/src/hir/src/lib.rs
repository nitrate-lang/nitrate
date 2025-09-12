#![warn(clippy::pedantic)]

mod expression;
mod type_store;

pub mod type_system;
pub use type_store::{TypeId, TypeStore};

pub use expression::*;
