#![warn(clippy::pedantic)]

mod expression;
mod node_digest;
mod type_store;

pub use node_digest::NodeDigest;

pub mod type_system;
pub use type_store::{TypeId, TypeStore};
