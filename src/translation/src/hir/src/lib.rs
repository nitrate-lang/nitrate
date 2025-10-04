#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod expr;
pub mod item;
mod item_store;
mod node_digest;
pub mod ty;
mod type_alignment;
mod type_size;
mod type_store;

pub use node_digest::NodeDigest;

pub use item_store::{ItemId, ItemStore};

pub use type_alignment::get_align_of;
pub use type_size::get_size_of;
pub use type_store::{TypeId, TypeStore};
