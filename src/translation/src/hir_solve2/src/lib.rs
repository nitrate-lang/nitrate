//! # Nitrate Constraint Solver v2 (hir_solve2)
//!
//! A suspended constraint-based type inference engine for the Nitrate compiler.
//!
//! ## Architecture
//!
//! Unlike the original `hir_solve` which eagerly resolves types during a
//! worklist-based fixed-point walk, `hir_solve2` separates constraint
//! **generation** from constraint **solving**:
//!
//! 1. **Walk** — traverse the expression tree, creating fresh type variables
//!    for `Inferred` types and collecting equality/subtype/trait constraints
//!    into a `ConstraintGraph`.
//! 2. **Unify** — solve equality constraints using union-find with occurs check
//!    and conflict detection.
//! 3. **Rewrite** — resolve `InferredInteger`/`InferredFloat` to concrete types
//!    (unconstrained integers → `i32`, unconstrained floats → `f64` per Rust),
//!    and desugar `Range` expressions into struct objects.
//! 4. **Monomorphize** — detect generic call sites and create specialized copies.
//! 5. **Repeat** until a fixed point is reached (no new monomorphizations).
//!
//! ## Key Improvements over hir_solve
//!
//! - **Clean separation**: constraint generation vs solving are distinct phases
//! - **Union-find unification**: proper occurs check prevents infinite types
//! - **Conflict detection**: if a variable is constrained to two different types,
//!   both constraints are reported
//! - **Rust-style defaults**: unconstrained integers → `i32`, floats → `f64`
//! - **Extensible**: `Subtype` and `HasTrait` constraints defined for future use
//! - **Fixed bugs**: dead `has_error` variable, missing `SliceRef`/`SlicePtr` in
//!   generic inference, incomplete struct type updates after monomorphization

#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]

mod bounds;
mod constraints;
mod diagnosis;
mod engine;
mod monomorphize;
mod range;
mod rewrite;
mod substitution;
mod walk;

pub use range::{ensure_range_structs, range_struct_name};

use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Function, GlobalVariable, SymbolTab};

/// Resolve type inference for a single function body.
///
/// This is the main entry point for the solver. It mutates the function
/// in place, resolving all `Inferred` types and monomorphizing generic
/// call sites. Errors are reported through the `CompilerLog`.
pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    engine::solve_function(function, m, log)
}

/// Resolve type inference for a global variable initializer.
pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    engine::solve_global(global, m, log)
}

#[cfg(test)]
mod tests;
