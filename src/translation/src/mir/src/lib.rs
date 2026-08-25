//! # Nitrate Mid-Level Intermediate Representation (MIR)
//!
//! The MIR is a control-flow-graph-based representation that sits
//! between the HIR and LLVM IR. It lowers high-level constructs
//! (pattern matching, method calls, complex expressions) into a
//! simpler form with explicit basic blocks, temporary locals, and
//! three-address-code style statements.
//!
//! ## Architecture
//!
//! MIR represents function bodies as a control-flow graph (CFG) of
//! basic blocks, each containing a sequence of statements terminated
//! by a terminator. Values flow through SSA-like locals and are
//! manipulated via places, operands, and rvalues.
//!
//! ### Key Concepts
//!
//! - **Basic blocks**: Linear sequences of statements with a single
//!   entry point and a terminator that transfers control to one or
//!   more successor blocks.
//! - **Locals**: Stack slots for variables, temporaries, and
//!   compiler-introduced values. Either mutable or immutable.
//! - **Places**: Memory locations that can be read from or written to
//!   (locals, dereferences, field projections, index projections).
//! - **Operands**: Values used by statements — either a copy of a
//!   place or a constant.
//! - **Rvalues**: Expressions that produce values — binary/unary ops,
//!   casts, references, aggregate construction, length queries.
//! - **Statements**: Side-effecting operations — assignments, storage
//!   live/dead markers, nops.
//! - **Terminators**: Control flow transfers — return, goto, switch,
//!   unreachable, call with destination.
//!
//! ## Module Overview
//!
//! | Module | Description |
//! |--------|-------------|
//! | [`func`] | Function body representation with basic blocks |
//! | [`stmt`] | Statement and terminator types |
//! | [`place`] | Memory location abstraction |
//! | [`operand`] | Value operands (copy/constant) |
//! | [`rvalue`] | Right-hand-side expression values |
//! | [`ty`] | MIR-specific type wrappers |
//! | [`store`] | TLS-based storage for MIR data |
//! | [`builder`] | Builder API for constructing MIR bodies |
//! | [`graphviz`] | Graphviz DOT output for MIR debugging |
//!
//! ## Relationship to HIR
//!
//! MIR is produced from HIR by the `nitrate_mir_from_hir` crate and
//! consumed by `nitrate_llvm_from_mir` for LLVM code generation.
//! Optional MIR optimization passes live in `nitrate_mir_optimize`.

mod builder;
mod func;
mod graphviz;
mod operand;
mod place;
mod rvalue;
mod stmt;
mod store;
mod ty;

pub use builder::*;
pub use func::*;
pub use operand::*;
pub use place::*;
pub use rvalue::*;
pub use stmt::*;
pub use store::*;
pub use ty::*;

/// Convenience re-export of all MIR types.
pub mod prelude {
    pub use crate::builder::*;
    pub use crate::func::*;
    pub use crate::operand::*;
    pub use crate::place::*;
    pub use crate::rvalue::*;
    pub use crate::stmt::*;
    pub use crate::store::*;
    pub use crate::ty::*;
}
