//! # Place Representation and Utilities
//!
//! This module provides utilities for working with `Place` values in the borrow checker.
//! Places represent paths to memory locations and form a tree structure.
//!
//! ## Place Tree Properties
//!
//! - Places form a rooted tree: each place has at most one parent
//! - Two places overlap if one is a prefix of the other
//! - Borrowing a place conflicts with borrowing any overlapping place
//!
//! ## Future Work
//!
//! - Place projections through enums (discriminant-aware)
//! - Tracking of partial moves (moving one field while leaving others)
//! - Support for slice index ranges (not just single indices)
