//! # Borrow-Checking Diagnostics
//!
//! Structured errors emitted by the MIR borrow checker. All variants live in
//! the `DiagnosticGroupId::BorrowCheck` diagnostic group (prefix `[B...]` /
//! group id 7) with stable variant ids starting at `0x100`.

use std::fmt;

use nitrate_diagnosis::{DiagnosticExplanation, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup, Origin};

/// All possible MIR borrow-checking errors.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum BorrowError {
    /// `&mut` of a place that is not mutable (immutable local/static).
    MutableBorrowOfImmutable { place: String, reason: String },
    /// A mutable borrow overlaps an existing borrow (shared or mutable).
    MutableBorrowConflict { place: String, borrow_kind: String, reason: String },
    /// A shared borrow overlaps an existing *activated* mutable borrow.
    SharedBorrowConflict { place: String, borrow_kind: String, reason: String },
    /// A write to a place while it is borrowed (shared or mutable).
    WriteWhileBorrowed { place: String, borrow_kind: String, reason: String },
    /// A read of a place while it is mutably borrowed.
    ReadWhileMutablyBorrowed { place: String, borrow_kind: String, reason: String },
    /// A reference to a local variable escapes the function (returned).
    BorrowOfLocalEscape { place: String, reason: String },
    /// A move out of a place while it is borrowed.
    MoveWhileBorrowed { place: String, borrow_kind: String, reason: String },
    /// A read of a value that was moved from.
    UseAfterMove { place: String, reason: String },
    /// A read of a value before it was initialized.
    UseBeforeInit { place: String, reason: String },
    /// A borrow of a value that was moved from.
    BorrowOfMoved { place: String, reason: String },
    /// A borrow of a value before it was initialized.
    BorrowOfUninit { place: String, reason: String },
    /// A write to a place whose parent was moved from.
    AssignToMoved { place: String, reason: String },
    /// A mutable borrow was activated while another borrow of the same place
    /// was still active (two-phase activation conflict).
    TwoPhaseActivationConflict { place: String, borrow_kind: String, reason: String },
}

impl BorrowError {
    /// The stable diagnostic variant id (within the `BorrowCheck` group).
    #[must_use]
    pub const fn variant_id(&self) -> u16 {
        match self {
            BorrowError::MutableBorrowOfImmutable { .. } => 0x100,
            BorrowError::MutableBorrowConflict { .. } => 0x101,
            BorrowError::SharedBorrowConflict { .. } => 0x102,
            BorrowError::WriteWhileBorrowed { .. } => 0x103,
            BorrowError::ReadWhileMutablyBorrowed { .. } => 0x104,
            BorrowError::BorrowOfLocalEscape { .. } => 0x105,
            BorrowError::MoveWhileBorrowed { .. } => 0x106,
            BorrowError::UseAfterMove { .. } => 0x107,
            BorrowError::UseBeforeInit { .. } => 0x108,
            BorrowError::BorrowOfMoved { .. } => 0x109,
            BorrowError::BorrowOfUninit { .. } => 0x10A,
            BorrowError::AssignToMoved { .. } => 0x10B,
            BorrowError::TwoPhaseActivationConflict { .. } => 0x10C,
        }
    }
}

impl fmt::Display for BorrowError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            BorrowError::MutableBorrowOfImmutable { place, reason } => {
                write!(f, "cannot borrow {place} as mutable — {reason}")
            }
            BorrowError::MutableBorrowConflict { place, borrow_kind, reason } => {
                write!(f, "cannot borrow {place} as mutable because it is already borrowed ({borrow_kind}) — {reason}")
            }
            BorrowError::SharedBorrowConflict { place, borrow_kind, reason } => {
                write!(f, "cannot borrow {place} as shared because it is already mutably borrowed ({borrow_kind}) — {reason}")
            }
            BorrowError::WriteWhileBorrowed { place, borrow_kind, reason } => {
                write!(f, "cannot assign to {place} because it is borrowed ({borrow_kind}) — {reason}")
            }
            BorrowError::ReadWhileMutablyBorrowed { place, borrow_kind, reason } => {
                write!(f, "cannot read {place} because it is mutably borrowed ({borrow_kind}) — {reason}")
            }
            BorrowError::BorrowOfLocalEscape { place, reason } => {
                write!(f, "cannot return a reference to local variable {place} — {reason}")
            }
            BorrowError::MoveWhileBorrowed { place, borrow_kind, reason } => {
                write!(f, "cannot move out of {place} because it is borrowed ({borrow_kind}) — {reason}")
            }
            BorrowError::UseAfterMove { place, reason } => {
                write!(f, "use of moved value {place} — {reason}")
            }
            BorrowError::UseBeforeInit { place, reason } => {
                write!(f, "use of possibly-uninitialized value {place} — {reason}")
            }
            BorrowError::BorrowOfMoved { place, reason } => {
                write!(f, "cannot borrow {place} because it was moved — {reason}")
            }
            BorrowError::BorrowOfUninit { place, reason } => {
                write!(f, "cannot borrow {place} because it is possibly-uninitialized — {reason}")
            }
            BorrowError::AssignToMoved { place, reason } => {
                write!(f, "cannot assign to {place} because its parent was moved — {reason}")
            }
            BorrowError::TwoPhaseActivationConflict { place, borrow_kind, reason } => {
                write!(f, "mutable borrow of {place} was activated while a {borrow_kind} borrow was still active — {reason}")
            }
        }
    }
}

impl FormattableDiagnosticGroup for BorrowError {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::BorrowCheck
    }

    fn variant_id(&self) -> u16 {
        BorrowError::variant_id(self)
    }

    fn format(&self) -> DiagnosticInfo {
        DiagnosticInfo {
            origin: Origin::None,
            message: self.to_string(),
        }
    }
}


/// Static explanations for every borrow-checking error code, used by `no3 --explain`.
pub fn explanations() -> &'static [DiagnosticExplanation] {
    &[
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x100,
            explanation: "A mutable borrow was taken of a place that is not mutable (e.g. an immutable `let` binding). \
                           Declare the binding with `var` to allow mutation.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x101,
            explanation: "A mutable borrow overlaps an existing active borrow of the same place. \
                           Nitrate enforces the aliasing rule: either multiple shared borrows or a single mutable borrow, \
                           never both at once. Restructure the code so the conflicting borrows do not overlap.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x102,
            explanation: "A shared borrow was created while an activated mutable borrow was still in use. \
                           Wait until the mutable borrow ends before taking a shared borrow of the same place.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x103,
            explanation: "A write (assignment) to a place occurred while it was borrowed. \
                           You cannot mutate a value that is currently borrowed. End the borrow before assigning.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x104,
            explanation: "A read of a place occurred while it was mutably borrowed. \
                           A mutable borrow grants exclusive access; end it before reading the value.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x105,
            explanation: "A reference to a local variable escapes the function (for example, by being returned). \
                           The local would be destroyed when the function returns, leaving a dangling reference. \
                           Return an owned value instead.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x106,
            explanation: "A value was moved out of a place while it was borrowed. \
                           Moving out invalidates the borrowed memory. End the borrow before moving the value.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x107,
            explanation: "A value was used after it was moved from. \
                           Once a non-`Copy` value is moved, the original binding can no longer be read. \
                           Use the value before the move or move it only at the end of its use.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x108,
            explanation: "A possibly-uninitialized value was read. \
                           Initialize the variable before reading it, or make the initialization unconditional.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x109,
            explanation: "A borrow was taken of a value that was moved from. \
                           The moved value is gone; borrow it before the move or reinitialize first.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x10A,
            explanation: "A borrow was taken of a possibly-uninitialized value. \
                           Initialize the value before borrowing it.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x10B,
            explanation: "An assignment targeted a place whose parent was moved from. \
                           Reinitialize the parent (or its owner) before assigning into it.",
        },
        DiagnosticExplanation {
            group_id: DiagnosticGroupId::BorrowCheck,
            variant_id: 0x10C,
            explanation: "A two-phase mutable borrow was activated while another borrow of the same place was still active. \
                           Two-phase borrows are reserved at the call site and activated during the call; \
                           the activation must not conflict with existing borrows.",
        },
    ]
}

