use crate::operand::Operand;
use crate::place::Place;
use crate::rvalue::Rvalue;
use crate::store::{BasicBlockId, LocalId};
use serde::{Deserialize, Serialize};
use thin_vec::ThinVec;

// ─────────────────────────────────────────────────────────────
// Statements — within a basic block
// ─────────────────────────────────────────────────────────────

/// A Statement is a single operation within a basic block.
/// Statements execute sequentially. Only the Assign statement
/// has a side effect visible in the MIR; StorageLive/StorageDead
/// are markers for borrow checking and optimization.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Statement {
    /// Assign the result of an rvalue computation to a place.
    /// SSA requires that each Local is assigned exactly once.
    Assign(Place, Rvalue),

    /// Set the discriminant of an enum place to the given variant index.
    /// This is used to initialize a variant before its payload is written.
    SetDiscriminant { place: Place, variant_index: u32 },

    /// Mark a local variable as "live" — storage begins here.
    StorageLive(LocalId),

    /// Mark a local variable as "dead" — storage ends here.
    StorageDead(LocalId),
}

// ─────────────────────────────────────────────────────────────
// Terminators — control flow at end of basic block
// ─────────────────────────────────────────────────────────────

/// A Terminator is the final instruction of a basic block.
/// It transfers control to one or more target blocks.
/// Every basic block MUST end with exactly one terminator.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Terminator {
    /// Unconditional branch to a target block.
    Goto { target: BasicBlockId },

    /// Conditional branch on a boolean operand.
    /// If the operand is true, branch to `true_target`; otherwise `false_target`.
    If {
        condition: Operand,
        true_target: BasicBlockId,
        false_target: BasicBlockId,
    },

    /// Multi-way branch on an integer operand.
    /// Switches on the integer value, branching to the corresponding target.
    SwitchInt {
        discr: Operand,
        /// Mapping from integer values to target blocks
        targets: ThinVec<(u128, BasicBlockId)>,
        /// Default target if no arm matches
        otherwise: BasicBlockId,
    },

    /// Return from the function with an optional value.
    Return { value: Option<Operand> },

    /// Jump to an unwind block. Used for panic/exception unwinding path.
    Unwind { target: BasicBlockId },

    /// Unreachable — marks a code path that must never be taken.
    Unreachable,

    /// Call a function that never returns (diverges), e.g. `exit()` or `panic()`.
    /// No destination is needed because control never returns.
    Call { callee: Operand, args: ThinVec<Operand> },

    /// Call a function and continue to `target` with the return value
    /// placed in `destination`.
    CallReturn {
        callee: Operand,
        args: ThinVec<Operand>,
        destination: Place,
        target: BasicBlockId,
    },

    /// Resume unwinding after a caught panic (used with Unwind).
    Resume,

    /// Abort the program immediately.
    Abort,
}

// ─────────────────────────────────────────────────────────────
// Basic Blocks
// ─────────────────────────────────────────────────────────────

/// A Basic Block is a straight-line sequence of statements ending with
/// a terminator. Branches (ifs, loops, switches) are represented as
/// terminators that transfer to other blocks. Non-branching expressions
/// remain as DAG/tree-like Rvalues within Assign statements.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct BasicBlock {
    /// Statements executed in sequence within this block.
    pub statements: ThinVec<Statement>,

    /// The terminator (control flow transfer) at the end of this block.
    /// Always present — use Terminator::Unreachable for blocks that
    /// should never terminate normally.
    pub terminator: Terminator,
}

impl BasicBlock {
    #[must_use]
    pub fn new(statements: ThinVec<Statement>, terminator: Terminator) -> Self {
        Self { statements, terminator }
    }

    /// Returns the successor blocks of this block's terminator.
    #[must_use]
    pub fn successors(&self) -> ThinVec<BasicBlockId> {
        match &self.terminator {
            Terminator::Goto { target } => [target.clone()].into_iter().collect(),
            Terminator::If {
                true_target,
                false_target,
                ..
            } => [true_target.clone(), false_target.clone()].into_iter().collect(),
            Terminator::SwitchInt { targets, otherwise, .. } => {
                let mut succs: ThinVec<BasicBlockId> = targets.iter().map(|(_, bb)| bb.clone()).collect();
                succs.push(otherwise.clone());
                succs
            }
            Terminator::Return { .. } | Terminator::Unreachable | Terminator::Resume | Terminator::Abort => {
                ThinVec::new()
            }
            Terminator::Call { .. } => ThinVec::new(),
            Terminator::CallReturn { target, .. } => [target.clone()].into_iter().collect(),
            Terminator::Unwind { target } => [target.clone()].into_iter().collect(),
        }
    }
}
