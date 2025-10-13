#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod dump;
mod dump_expr;
mod dump_item;
mod dump_ty;
mod expr;
mod item;
mod literal_ops;
mod node_digest;
mod save;
mod store;
mod ty;
mod ty_alignment;
mod ty_size;
mod ty_stride;

pub use dump::{Dump, DumpContext};
pub use node_digest::NodeDigest;
pub use store::{
    BlockId, EnumTypeId, FunctionTypeId, ItemId, LiteralId, Store, StructTypeId, SymbolId, TypeId,
    ValueId,
};

pub mod hir {
    pub use super::expr::*;
    pub use super::item::*;
    pub use super::save::IntoStoreId;
    pub use super::ty::*;
    pub use super::ty_alignment::*;
    pub use super::ty_size::*;
    pub use super::ty_stride::*;
    pub use crate::literal_ops::*;
}

pub mod prelude {
    pub use super::hir::*;
    pub use super::{
        BlockId, Dump, DumpContext, EnumTypeId, FunctionTypeId, ItemId, LiteralId, Store,
        StructTypeId, SymbolId, TypeId, ValueId,
    };
}
