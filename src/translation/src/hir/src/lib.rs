#![warn(clippy::pedantic)]

mod expression;
mod item_store;
pub mod item_system;
mod node_digest;
mod type_alignment;
mod type_size;
mod type_store;
pub mod type_system;

pub use node_digest::NodeDigest;

pub use item_store::{ItemId, ItemStore};

pub use type_alignment::get_align_of;
pub use type_size::get_size_of;
pub use type_store::{TypeId, TypeStore};
