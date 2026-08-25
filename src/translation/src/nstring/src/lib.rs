//! # Nitrate Interned Strings (`NString`)
//!
//! The `NString` type provides fast, memory-efficient interned strings
//! for identifiers, keywords, symbols, and other frequently duplicated
//! string data throughout the compiler.
//!
//! ## Design
//!
//! Strings are interned into a global concurrent map and reference-counted.
//! Two `NString` values compare equal if they point to the same interned
//! string data. Cloning is cheap (atomic reference count increment), and
//! hashing uses the precomputed hash of the interned data.
//!
//! ## Key Functions
//!
//! - [`NString::from`]: Interns a string, returning an existing entry if
//!   one already exists.
//! - [`intern_nstring`]: Same as `NString::from` but as a free function.
//! - [`nstring_forget_all`]: Clears the interner (for use in tests that
//!   need to reset state between runs).
//!
//! ## Implementation
//!
//! The interner uses a `DashMap` behind an `Arc` for concurrent read
//! access with minimal contention. This is safe because the compiler
//! is single-threaded per compilation session, but the design allows
//! for future parallel compilation.

mod nstring;

pub use nstring::{NString, intern_nstring, nstring_forget_all};
