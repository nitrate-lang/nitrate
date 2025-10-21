#![warn(clippy::pedantic)]

mod cmp;
mod dump;
mod dump_expr;
mod dump_item;
mod dump_ty;
mod expr;
mod item;
mod iter;
mod iter_ops;
mod literal_ops;
mod node_digest;
mod save;
mod store;
mod table;
mod try_for_each;
mod ty;
mod ty_alignment;
mod ty_size;
mod ty_stride;

pub use dump::{Dump, DumpContext};
pub use node_digest::NodeDigest;
pub use store::{
    BlockId, EnumDefId, EnumTypeId, FunctionId, FunctionTypeId, GlobalVariableId, LiteralId,
    LocalVariableId, ModuleId, ParameterId, Store, StructDefId, StructTypeId, TraitId,
    TypeAliasDefId, TypeId, ValueId,
};
pub use table::SymbolTab;

pub mod hir {
    pub use super::cmp::*;
    pub use super::expr::*;
    pub use super::item::*;
    pub use super::iter::*;
    pub use super::literal_ops::*;
    pub use super::save::IntoStoreId;
    pub use super::ty::*;
    pub use super::ty_alignment::*;
    pub use super::ty_size::*;
    pub use super::ty_stride::*;
}

pub mod prelude {
    pub use super::hir::*;
    pub use super::{
        BlockId, Dump, DumpContext, EnumDefId, EnumTypeId, FunctionId, FunctionTypeId,
        GlobalVariableId, LiteralId, LocalVariableId, ModuleId, ParameterId, Store, StructDefId,
        StructTypeId, TraitId, TypeAliasDefId, TypeId, ValueId,
    };
}
