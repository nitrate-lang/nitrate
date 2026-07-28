#![forbid(unsafe_code)]
#![allow(clippy::result_unit_err)]
//! # Nitrate Borrow Checker
//!
//! This crate implements a comprehensive Non-Lexical Lifetimes (NLL) borrow checker
//! for the Nitrate compiler. It performs dataflow analysis on HIR to enforce memory
//! safety guarantees in safe code.
//!
//! ## Design Philosophy
//!
//! Unlike Rust's borrow checker (which has known soundness holes), this borrow checker
//! is designed to be provably sound with no loopholes. Key differences:
//!
//! - **No two-phase borrow loopholes**: Two-phase borrows are strictly limited to
//!   reborrowing for function arguments, not general mutable references
//! - **No lifetime coercion unsoundness**: All lifetime relationships must be explicitly
//!   justified through the constraint graph
//! - **Comprehensive place tracking**: Borrows are tracked through all field and index
//!   accesses, not just top-level variables
//! - **Conservative region inference**: When in doubt, the checker conservatively
//!   rejects programs rather than risking unsoundness
//!
//! ## Algorithm Overview
//!
//! The borrow checker operates in four passes:
//!
//! 1. **Place Analysis**: Decomposes all expressions into `Place` values that represent
//!    paths to memory locations. Every memory access is classified as a read, write,
//!    or borrow (with its kind).
//!
//! 2. **Lifetime/Region Creation**: Assigns abstract region variables to each reference
//!    type and each borrow expression. Creates a `Region` for each program point scope.
//!
//! 3. **Constraint Collection**: Walks the function body and collects:
//!    - Outlives constraints: 'a: 'b (region 'a must outlive region 'b)
//!    - Borrow creation sites: where each borrow starts
//!    - Borrow kill sites: where each borrow ends (last use)
//!    - Conflicting access points: reads/writes that might conflict with active borrows
//!
//! 4. **Constraint Solving + Conflict Detection**: Solves the region constraints
//!    using fixed-point iteration, then checks for conflicts between active borrows
//!    and memory accesses.

mod check;
mod constraints;
mod place;
mod region;

pub use check::check_function_borrows;
pub use check::check_global_borrows;

use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use std::collections::{HashMap, HashSet};
use std::matches;
use std::write;

/// Diagnostics emitted by the borrow checker.
pub mod diagnosis {
    use std::format;

    use nitrate_diagnosis::{DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup};

    /// All possible borrow-checking errors.
    #[derive(Debug, Clone, PartialEq, Eq, Hash)]
    pub enum BorrowError {
        /// A borrow of a local variable outlives the variable itself
        /// (e.g., returning a reference to a local).
        BorrowOfLocal { place: String, reason: String },

        /// A borrow conflicts with an existing active borrow
        /// (e.g., mutable borrow while shared borrow is active).
        BorrowConflict {
            place: String,
            borrow_kind: String,
            conflicting_kind: String,
            reason: String,
        },

        /// A use of a moved value.
        UseAfterMove { place: String, reason: String },

        /// A value is used before it is initialized.
        UseBeforeInit { place: String, reason: String },

        /// A mutable borrow target is not mutable.
        MutableBorrowOfImmutable { place: String, reason: String },

        /// Invalidates a borrow (e.g., assigning to a field while it's borrowed).
        InvalidatesBorrow { place: String, reason: String },

        /// A borrow created inside a loop body escapes to the next iteration.
        BorrowEscapesLoop { place: String, reason: String },
    }

    impl FormattableDiagnosticGroup for BorrowError {
        fn group_id(&self) -> DiagnosticGroupId {
            DiagnosticGroupId::Hir
        }

        fn variant_id(&self) -> u16 {
            match self {
                BorrowError::BorrowOfLocal { .. } => 0x100,
                BorrowError::BorrowConflict { .. } => 0x101,
                BorrowError::UseAfterMove { .. } => 0x102,
                BorrowError::UseBeforeInit { .. } => 0x103,
                BorrowError::MutableBorrowOfImmutable { .. } => 0x104,
                BorrowError::InvalidatesBorrow { .. } => 0x105,
                BorrowError::BorrowEscapesLoop { .. } => 0x106,
            }
        }

        fn format(&self) -> DiagnosticInfo {
            let message = match self {
                BorrowError::BorrowOfLocal { place, reason } => {
                    format!(
                        "cannot return reference to local variable `{}`\n  = note: {}",
                        place, reason
                    )
                }
                BorrowError::BorrowConflict {
                    place,
                    borrow_kind,
                    conflicting_kind,
                    reason,
                } => {
                    format!(
                        "cannot {} borrow `{}` because it is already {} borrowed\n  = note: {}",
                        conflicting_kind, place, borrow_kind, reason
                    )
                }
                BorrowError::UseAfterMove { place, reason } => {
                    format!("use of moved value `{}`\n  = note: {}", place, reason)
                }
                BorrowError::UseBeforeInit { place, reason } => {
                    format!(
                        "use of potentially uninitialized variable `{}`\n  = note: {}",
                        place, reason
                    )
                }
                BorrowError::MutableBorrowOfImmutable { place, reason } => {
                    format!(
                        "cannot borrow `{}` as mutable, as it is not declared as mutable\n  = note: {}",
                        place, reason
                    )
                }
                BorrowError::InvalidatesBorrow { place, reason } => {
                    format!(
                        "cannot assign to `{}` because it is borrowed\n  = note: {}",
                        place, reason
                    )
                }
                BorrowError::BorrowEscapesLoop { place, reason } => {
                    format!("borrow of `{}` escapes loop body\n  = note: {}", place, reason)
                }
            };

            DiagnosticInfo {
                origin: nitrate_diagnosis::Origin::None,
                message,
            }
        }
    }
}

use diagnosis::BorrowError;
use region::RegionInferenceCtx;

// ============================================================================
// Core Types
// ============================================================================

/// A unique identifier for a region (abstract set of program points)
/// within a single function.
pub type RegionId = u32;

/// A unique identifier for a place (memory location path) within a function body.
pub type PlaceId = u32;

/// The kind of a borrow, determining what operations conflict with it.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum BorrowKind {
    /// Shared reference `&T` — allows reads but conflicts with writes and mutable borrows.
    Shared,
    /// Exclusive immutable reference `&uniq T` — conflicts with any other borrow.
    ExclusiveImmutable,
    /// Mutable reference `&mut T` — conflicts with any other borrow or read.
    Mutable,
}

impl BorrowKind {
    /// Returns true if this borrow conflicts with a write to the borrowed place.
    pub fn conflicts_with_write(&self) -> bool {
        matches!(
            self,
            BorrowKind::Shared | BorrowKind::ExclusiveImmutable | BorrowKind::Mutable
        )
    }

    /// Returns true if this borrow conflicts with a read of the borrowed place.
    pub fn conflicts_with_read(&self) -> bool {
        matches!(self, BorrowKind::ExclusiveImmutable | BorrowKind::Mutable)
    }
}

/// Represents a borrow that is active at some set of program points.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BorrowRecord {
    /// The place being borrowed.
    pub place: PlaceId,
    /// The kind of borrow.
    pub kind: BorrowKind,
    /// The region where this borrow was created.
    pub region: RegionId,
    /// The reason for this borrow (for diagnostic messages).
    pub reason: String,
}

/// A path segment in a `Place` representation.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum PlaceElem {
    /// A named field access: `.field_name`
    Field(NString),
    /// An indexed access: `[index]`
    Index(PlaceId),
    /// A dereference of a pointer/reference: `*place`
    Deref,
}

impl std::fmt::Display for PlaceElem {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            PlaceElem::Field(name) => write!(f, ".{}", name),
            PlaceElem::Index(id) => write!(f, "[{:?}]", id),
            PlaceElem::Deref => write!(f, ".*"),
        }
    }
}

/// A `Place` is a complete path to a memory location.
/// It provides a structured representation of all the ways memory can be addressed:
///
/// Places form a tree rooted at either a local variable, a static variable, or
/// a temporary expression result. The tree structure means that borrowing a parent
/// place conflicts with borrowing any descendant, and vice versa.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Place {
    /// A local variable binding.
    Local(LocalVariableId),
    /// A static/global variable.
    Static(GlobalVariableId),
    /// A function parameter.
    Param(ParameterId),
    /// A projection from a parent place.
    Projection {
        /// The parent place.
        base: Box<Place>,
        /// The projection element (field, index, deref).
        elem: PlaceElem,
    },
    /// A temporary value created during expression evaluation.
    /// These can't be borrowed because they don't have a stable memory address.
    Temporary,
}

impl std::fmt::Display for Place {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Place::Local(id) => write!(f, "{}", id.borrow().name),
            Place::Static(id) => write!(f, "static {}", id.borrow().name),
            Place::Param(id) => write!(f, "param {}", id.borrow().name),
            Place::Temporary => write!(f, "<temporary>"),
            Place::Projection { base, elem } => {
                write!(f, "{}{}", base, elem)
            }
        }
    }
}

impl Place {
    /// Returns true if this place represents the same root memory location as `other`,
    /// ignoring projection differences. Used for detecting conflicting borrows.
    pub fn same_root(&self, other: &Place) -> bool {
        match (self, other) {
            (Place::Local(a), Place::Local(b)) => a == b,
            (Place::Static(a), Place::Static(b)) => a == b,
            (Place::Param(a), Place::Param(b)) => a == b,
            (Place::Projection { base: b1, .. }, Place::Projection { base: b2, .. }) => b1.same_root(b2),
            (Place::Projection { base, .. }, other) | (other, Place::Projection { base, .. }) => base.same_root(other),
            _ => false,
        }
    }

    /// Returns true if this place is a strict prefix of `other`.
    /// e.g., `x.f` is a prefix of `x.f.g`, `x` is a prefix of `x.f`.
    /// This matters for borrow conflicts: borrowing `x.f` doesn't conflict with
    /// borrowing `x.g`, but borrowing `x` DOES conflict with both.
    pub fn is_prefix_of(&self, other: &Place) -> bool {
        if self == other {
            return true;
        }
        match (self, other) {
            (Place::Local(a), Place::Local(b)) => a == b,
            (Place::Static(a), Place::Static(b)) => a == b,
            (Place::Param(a), Place::Param(b)) => a == b,
            // If other is a projection, check if self equals or is prefix of its base
            (base, Place::Projection { base: other_base, .. }) => {
                base == other_base.as_ref() || base.is_prefix_of(other_base)
            }
            _ => false,
        }
    }

    /// Returns true if self and other are the same place or overlapping.
    /// Overlap means one is a prefix of the other.
    pub fn overlaps_with(&self, other: &Place) -> bool {
        self.is_prefix_of(other) || other.is_prefix_of(self)
    }
}

// ============================================================================
// Borrow Check Context
// ============================================================================

/// The main borrow checking context for a single function.
///
/// This struct holds all the state needed to perform borrow checking on one function body.
/// It is designed to be created fresh for each function, used, and then discarded.
pub(crate) struct BorrowCheckCtx<'a> {
    /// The function being checked (for diagnostics).
    #[allow(dead_code)]
    function_name: NString,
    /// The function's return type.
    return_type: TypeId,
    /// Symbol table for type resolution.
    tab: &'a SymbolTab,
    /// Diagnostic log.
    log: &'a CompilerLog,

    // --- Place tracking ---
    /// Map from place to its place ID.
    place_to_id: HashMap<Place, PlaceId>,
    /// Map from place ID to the place.
    pub(crate) id_to_place: Vec<Place>,
    /// Counter for generating new place IDs.
    next_place_id: PlaceId,

    // --- Region tracking ---
    /// Counter for generating new region IDs.
    next_region_id: RegionId,

    // --- Borrow tracking ---
    /// All active borrows, keyed by the place they borrow.
    pub(crate) active_borrows: Vec<BorrowRecord>,
    /// The current program point (represented as the region index).
    pub(crate) current_region: RegionId,

    // --- NLL tracking ---
    /// For each active borrow, the set of regions where it is used.
    borrow_use_regions: Vec<Vec<RegionId>>,
    /// Stack of borrow counts at branch points for tracking borrows created inside branches.
    borrow_count_stack: Vec<usize>,
    /// Region inference engine for NLL constraint solving.
    /// This tracks outlives constraints between regions and solves them
    /// to determine the minimal lifetime for each borrow.
    region_inference: RegionInferenceCtx,

    // --- Move/init state ---
    /// Set of place IDs that have been moved from.
    pub(crate) moved_from: HashSet<PlaceId>,
    /// Set of place IDs that have been initialized.
    pub(crate) initialized: HashSet<PlaceId>,

    // --- Error accumulation ---
    errors: Vec<BorrowError>,
}

impl<'a> BorrowCheckCtx<'a> {
    pub(crate) fn new(function_name: NString, return_type: TypeId, tab: &'a SymbolTab, log: &'a CompilerLog) -> Self {
        Self {
            function_name,
            return_type,
            tab,
            log,
            place_to_id: HashMap::new(),
            id_to_place: Vec::new(),
            next_place_id: 0,
            next_region_id: 0,
            active_borrows: Vec::new(),
            current_region: 0,
            borrow_use_regions: Vec::new(),
            borrow_count_stack: Vec::new(),
            region_inference: RegionInferenceCtx::new(),
            moved_from: HashSet::new(),
            initialized: HashSet::new(),
            errors: Vec::new(),
        }
    }

    /// Allocate a new place ID for a given place, or return the existing one.
    pub(crate) fn place_id(&mut self, place: Place) -> PlaceId {
        if let Some(&id) = self.place_to_id.get(&place) {
            return id;
        }
        let id = self.next_place_id;
        self.next_place_id += 1;
        self.place_to_id.insert(place.clone(), id);
        self.id_to_place.push(place);
        id
    }

    /// Create a new region scope.
    pub(crate) fn new_region(&mut self) -> RegionId {
        let id = self.next_region_id;
        self.next_region_id += 1;
        id
    }

    /// Advance to a new region (program point).
    pub(crate) fn advance_region(&mut self) -> RegionId {
        let id = self.new_region();
        self.current_region = id;
        id
    }

    /// Record that a borrow is used at the current region.
    pub(crate) fn record_borrow_use(&mut self, borrow_idx: usize) {
        let current = self.current_region;
        // Extend the borrow_use_regions for this borrow if needed
        while self.borrow_use_regions.len() <= borrow_idx {
            self.borrow_use_regions.push(Vec::new());
        }
        let uses = &mut self.borrow_use_regions[borrow_idx];
        if !uses.contains(&current) {
            uses.push(current);
        }
    }

    /// Get the set of regions where a borrow is used.
    pub(crate) fn borrow_use_regions_at(&self, borrow_idx: usize) -> &[RegionId] {
        if borrow_idx < self.borrow_use_regions.len() {
            &self.borrow_use_regions[borrow_idx]
        } else {
            &[]
        }
    }

    /// Push the current borrow count onto the stack (for branch handling).
    pub(crate) fn push_borrow_count(&mut self) {
        self.borrow_count_stack.push(self.active_borrows.len());
    }

    /// Pop the borrow count stack and return borrows created since the matching push.
    pub(crate) fn pop_new_borrows_since_push(&mut self) -> Vec<BorrowRecord> {
        let saved_count = self.borrow_count_stack.pop().unwrap_or(0);
        if saved_count < self.active_borrows.len() {
            self.active_borrows.drain(saved_count..).collect()
        } else {
            Vec::new()
        }
    }

    /// Get a mutable reference to the region inference engine for adding constraints.
    pub(crate) fn region_inference(&mut self) -> &mut RegionInferenceCtx {
        &mut self.region_inference
    }

    /// Report a borrow checking error.
    pub(crate) fn report(&mut self, error: BorrowError) {
        self.errors.push(error);
    }

    /// Check all tracked errors and report them to the compiler log.
    pub(crate) fn flush_errors(&self) -> Result<(), ()> {
        for error in &self.errors {
            self.log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }

    /// Solve all collected region constraints.
    /// This must be called after all constraint collection is complete.
    pub(crate) fn solve_regions(&mut self) {
        self.region_inference.solve();
    }
}
