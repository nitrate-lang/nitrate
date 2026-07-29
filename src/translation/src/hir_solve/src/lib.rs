#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

//! # Nitrate HIR Solver
//!
//! The solver crate is the core type inference engine for the Nitrate compiler.
//! It transforms unresolved HIR (with `Inferred` type variables and `GenericParam`
//! references) into fully resolved, concrete type assignments ready for validation
//! and code generation.
//!
//! ## Architecture
//!
//! The solver is organized into several modules, each with a clear responsibility:
//!
//! | Module | Purpose |
//! |--------|---------|
//! | [`solver`] | Core `Solver` struct, fixed-point iteration, literal resolution |
//! | [`visit`] | Per-variant visitor handlers for all Value expression nodes |
//! | [`bounds`] | Value range analysis for integer/refinement type checking |
//! | [`constraints`] | Constraint types (`TypeConstraint`, `NodeAction`) and propagation helpers |
//! | [`substitution`] | Type substitution for generic instantiation (monomorphization) |
//! | [`monomorphize`] | Generic function/struct instantiation with caching |
//! | [`diagnosis`] | Comprehensive error types with source locations |
//!
//! ## Key Design Patterns
//!
//! - **Fixed-point iteration**: The solver repeatedly visits all block elements
//!   until constraint accumulation stabilizes, enabling transitive propagation
//!   and incremental generic resolution.
//! - **Bidirectional inference**: Constraints flow both forward (from child values
//!   to parent expressions) and backward (from parent context to children), maximizing
//!   type inference power.
//! - **Deduplicated errors**: All errors are collected in a `HashSet<TypeErr>` to
//!   prevent duplicate reports of the same type mismatch.
//! - **Monomorphization caching**: Both function and struct monomorphization results
//!   are cached keyed by (original_id, sorted_type_args) to avoid redundant work.

mod bounds;
mod constraints;
mod diagnosis;
mod monomorphize;
mod solver;
mod substitution;

pub use solver::{resolve_function, resolve_global};

#[cfg(test)]
mod tests;
