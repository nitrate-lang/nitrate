#![forbid(unsafe_code)]
#![warn(clippy::pedantic)]

mod dump;
mod dump_expr_place;
mod dump_expr_value;
mod dump_item;
mod dump_ty;
mod expr_place;
mod expr_value;
mod item;
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
    BlockId, EnumAttributesId, EnumVariantsId, FuncAttributesId, ItemId, PlaceId, Store,
    StructAttributesId, StructFieldsId, SymbolId, TypeId, TypeListId, ValueId,
};

pub mod hir {
    pub use super::expr_place::*;
    pub use super::expr_value::*;
    pub use super::item::*;
    pub use super::save::IntoStoreId;
    pub use super::ty::*;
    pub use super::ty_alignment::*;
    pub use super::ty_size::*;
    pub use super::ty_stride::*;
}

pub mod prelude {
    pub use super::hir::*;
    pub use super::{
        BlockId, Dump, DumpContext, EnumAttributesId, EnumVariantsId, FuncAttributesId, ItemId,
        PlaceId, Store, StructAttributesId, StructFieldsId, SymbolId, TypeId, TypeListId, ValueId,
    };
}
