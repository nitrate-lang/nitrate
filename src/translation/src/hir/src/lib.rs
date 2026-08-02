//! # Nitrate High-Level Intermediate Representation (HIR)
//!
//! The HIR is the central data structure of the Nitrate compiler. It
//! sits between name resolution and code generation, serving as the
//! representation on which type inference, validation, optimization,
//! and LLVM lowering operate.
//!
//! ## Architecture
//!
//! HIR data is stored in thread-local append-only vectors accessed
//! through lightweight handle types (`TypeId`, `ValueId`, `FunctionId`,
//! etc.). This design provides:
//!
//! - **Deduplication**: Identical types produce the same `TypeId`,
//!   enabling fast structural equality via pointer comparison.
//! - **Interior mutability**: Values are stored in `RefCell`-backed
//!   storage, allowing in-place mutation during solving.
//! - **Lifetime-free handles**: Handles dereference through TLS,
//!   avoiding lifetime parameters throughout the codebase.
//!
//! ## Module Overview
//!
//! | Module | Description |
//! |--------|-------------|
//! | [`ty`] | The `Type` enum and related types (`Lifetime`, `TypeBound`, `FunctionType`, etc.) |
//! | [`expr`] | The `Value` enum — all expression variants in the HIR |
//! | [`item`] | Top-level items: `Function`, `StructDef`, `EnumDef`, `GlobalVariable`, `Parameter`, `LocalVariable` |
//! | [`store`] | Handle types (`TypeId`, `ValueId`, `FunctionId`, etc.) and TLS-based storage |
//! | [`table`] | The `SymbolTab` — central registry of all defined symbols |
//! | [`pass`] | Pass infrastructure for running compiler passes over HIR modules |
//! | [`literal_ops`] | The `Lit` enum for compile-time literal values |
//! | [`node_digest`] | Content-based hashing for HIR deduplication |
//! | [`helper`] | Utility functions for HIR manipulation |
//!
//! ## Key Types
//!
//! - [`Type`]: The type system's central enum — primitives, compounds,
//!   references, generics, refinements, and unresolved variants.
//! - [`Value`]: Expressions — literals, operations, control flow,
//!   calls, blocks, and symbolic references.
//! - [`Function`]: A function definition with parameters, return type,
//!   generics, and body.
//! - [`StructDef`]: A struct definition with fields, generics, and
//!   memory layout.
//! - [`SymbolTab`]: The symbol table providing iteration over all
//!   defined symbols and method lookup.

mod expr;
mod helper;
mod item;
mod literal_ops;
mod node_digest;
mod pass;
mod store;
mod table;
mod ty;

pub use expr::*;
pub use item::*;
pub use literal_ops::*;
pub use node_digest::*;
pub use pass::*;
pub use store::*;
pub use table::*;
pub use ty::*;

/// Convenience re-export of all HIR types.
///
/// Import with `use nitrate_hir::prelude::*` to bring all HIR types
/// into scope in one line.
pub mod prelude {
    pub use super::*;
}
