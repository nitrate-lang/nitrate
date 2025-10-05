#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod dump;
mod dump_expr;
mod dump_item;
mod dump_ty;
mod expr;
mod expr_place;
mod item;
mod node_digest;
mod store;
mod ty;
mod type_alignment;
mod type_size;
mod type_stride;

pub use dump::{Dump, DumpContext};
pub use node_digest::NodeDigest;
pub use store::{BlockId, ExprId, ItemId, Store, TypeId};

pub use type_alignment::get_align_of;
pub use type_size::get_size_of;
pub use type_stride::get_stride_of;

pub mod hir {
    pub use super::expr::*;
    pub use super::item::*;
    pub use super::ty::*;
}

pub mod prelude {
    pub use super::hir;
    pub use super::{BlockId, Dump, DumpContext, ExprId, ItemId, TypeId};
}
