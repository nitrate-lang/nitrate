#![forbid(unsafe_code)]

//! # HIR Type Utilities
//!
//! Utility functions for querying properties of HIR types:
//!
//! - **Type retrieval** ([`HirGetType`]): Trait for extracting the
//!   [`TypeId`] from HIR values and type-bearing constructs.
//! - **Size computation** ([`hir_type_size`]): Returns the allocation
//!   size of a type in bytes, accounting for platform pointer width.
//! - **Alignment** ([`hir_type_alignment`]): Returns the required
//!   memory alignment of a type.
//! - **Stride** ([`hir_type_stride`]): Returns the array stride
//!   (size rounded up to alignment) for a type.
//!
//! These functions are used throughout the compiler — by the solver
//! for type classification, by codegen for layout computation, and
//! by the evaluator for constant folding.

mod get_type;
mod ty_alignment;
mod ty_copy;
mod ty_size;
mod ty_stride;

pub use get_type::*;
pub use ty_alignment::*;
pub use ty_copy::*;
pub use ty_size::*;
pub use ty_stride::*;
