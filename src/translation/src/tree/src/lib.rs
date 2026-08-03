#![forbid(unsafe_code)]

//! # Nitrate Parse Tree (Concrete Syntax Tree)
//!
//! The parse tree represents the syntactic structure of Nitrate source
//! code as a concrete tree with full source position information. It is
//! the output of the parser and the input to name resolution.
//!
//! ## Architecture
//!
//! The tree is a mutable arena-based tree where each node stores its
//! kind (expression, type, item), source span, and child links. The
//! tree supports both immutable and mutable traversal via iterator
//! types.
//!
//! ## Module Overview
//!
//! | Module | Description |
//! |--------|-------------|
//! | [`ast`] | AST node types: expressions, types, items, spans, trivia |
//! | [`span`] | `SrcPos` — compact source location type |
//! | [`trivia`] | Whitespace and comment handling |
//! | [`iter`] / [`iter_mut`] | Immutable / mutable tree traversals |
//! | [`pretty_print`] | Debug and display formatting of parse trees |
//!
//! ## Key Types
//!
//! - [`SrcPos`]: A compact source position storing file, line, column, and byte offset.
//! - [`Trivia`]: Whitespace and comment information attached to tokens.
//! - [`ParseTreeIter`] / [`ParseTreeIterMut`]: Iterator-based tree traversal.
//! - [`PrettyPrint`]: Trait for formatting tree nodes as source-like text.

mod convert;
mod expr;
mod expr_iter;
mod expr_iter_mut;
mod item;
mod item_iter;
mod item_iter_mut;
mod iter;
mod iter_mut;
mod pretty_print;
mod span;
mod trivia;
mod ty;
mod ty_iter;
mod ty_iter_mut;

pub use convert::{
    raw_to_srcpos, span_from_offsets, span_from_raw, tok_to_srcpos_end, tok_to_srcpos_start, tok_to_srcspan,
};
pub use span::{SrcPos, SrcSpan};
pub use trivia::Trivia;

/// Abstract Syntax Tree node types.
///
/// Contains expression nodes, type nodes, item nodes, and associated
/// iteration and span types used by the parser, resolver, and HIR
/// lowering stages.
pub mod ast {
    pub use super::expr::*;
    pub use super::span::*;
    pub use super::trivia::*;
    pub use super::ty::*;
    pub use crate::item::*;
}

pub use iter::{Order, ParseTreeIter, RefNode};
pub use iter_mut::{ParseTreeIterMut, RefNodeMut};
pub use pretty_print::{PrettyPrint, PrintContext};

/// Convenience re-export of all parse tree types.
pub mod prelude {
    pub use super::ast::*;
    pub use super::iter::{Order, ParseTreeIter, RefNode};
    pub use super::iter_mut::{ParseTreeIterMut, RefNodeMut};
    pub use super::pretty_print::{PrettyPrint, PrintContext};
}
