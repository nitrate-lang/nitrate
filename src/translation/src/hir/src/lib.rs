#![warn(clippy::pedantic)]

mod cmp;
mod context;
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
    BlockId, EnumTypeId, FunctionId, FunctionTypeId, GlobalVariableId, LiteralId, LocalVariableId,
    ModuleId, ParameterId, Store, StructTypeId, SymbolId, TraitId, TypeId, ValueId,
};

pub mod hir {
    pub use super::cmp::*;
    pub use super::context::*;
    pub use super::expr::*;
    pub use super::item::*;
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
        BlockId, Dump, DumpContext, EnumTypeId, FunctionId, FunctionTypeId, GlobalVariableId,
        LiteralId, LocalVariableId, ModuleId, ParameterId, Store, StructTypeId, SymbolId, TraitId,
        TypeId, ValueId,
    };
}
