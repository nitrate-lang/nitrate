#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod dump;
mod dump_expr;
mod dump_item;
mod dump_ty;
mod expr_place;
mod expr_value;
mod item;
mod node_digest;
mod store;
mod ty;
mod type_alignment;
mod type_size;
mod type_stride;

pub use dump::{Dump, DumpContext};
pub use node_digest::NodeDigest;
pub use store::{BlockId, ItemId, Store, TypeId, ValueId};

pub use type_alignment::get_align_of;
pub use type_size::get_size_of;
pub use type_stride::get_stride_of;

pub mod hir {
    pub use super::expr_value::*;
    pub use super::item::*;
    pub use super::ty::*;
}

pub mod prelude {
    pub use super::hir;
    pub use super::{BlockId, Dump, DumpContext, ItemId, TypeId, ValueId};
}
