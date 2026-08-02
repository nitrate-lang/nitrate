use crate::operand::Operand;
use crate::place::Place;
use crate::rvalue::Rvalue;
use crate::store::{BasicBlockId, LocalId, MirTypeId};
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
///
/// # Block Arguments on Edges
///
/// Every branch-like terminator carries operand lists for each successor edge.
/// These operands are the "block arguments" passed to the target block's formal
/// parameters (see `BasicBlock::args`). The number and types of operands per
/// edge must match the `args` list of the corresponding target block.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Terminator {
    /// Unconditional branch to a target block, passing the given operands
    /// as block arguments to the target.
    Goto {
        target: BasicBlockId,
        args: ThinVec<Operand>,
    },

    /// Conditional branch on a boolean operand.
    /// If the operand is true, branch to `true_target`; otherwise `false_target`.
    /// Each branch carries its own block argument list.
    If {
        condition: Operand,
        true_target: BasicBlockId,
        true_args: ThinVec<Operand>,
        false_target: BasicBlockId,
        false_args: ThinVec<Operand>,
    },

    /// Multi-way branch on an integer operand.
    /// Switches on the integer value, branching to the corresponding target.
    /// Each arm carries block arguments for its target.
    SwitchInt {
        discr: Operand,
        /// Mapping from integer values to (target block, block arguments)
        targets: ThinVec<(u128, BasicBlockId, ThinVec<Operand>)>,
        /// Default target if no arm matches, with its block arguments
        otherwise: BasicBlockId,
        otherwise_args: ThinVec<Operand>,
    },

    /// Return from the function with an optional value.
    Return { value: Option<Operand> },

    /// Unreachable — marks a code path that must never be taken.
    Unreachable,

    /// Call a function.
    ///
    /// * If `destination` is `Some` and `target` is `Some`: the call returns,
    ///   the return value is stored in `destination`, and control transfers to
    ///   `target` with the given `target_args` block arguments.
    /// * If `destination` is `None` and `target` is `None`: the call diverges
    ///   (never returns). Control does not continue past this terminator.
    Call {
        callee: Operand,
        args: ThinVec<Operand>,
        destination: Option<Place>,
        target: Option<BasicBlockId>,
        target_args: ThinVec<Operand>,
    },
}

// ─────────────────────────────────────────────────────────────
// Basic Blocks
// ─────────────────────────────────────────────────────────────

/// A Basic Block is a straight-line sequence of statements ending with
/// a terminator. Branches (ifs, loops, switches) are represented as
/// terminators that transfer to other blocks. Non-branching expressions
/// remain as DAG/tree-like Rvalues within Assign statements.
///
/// # Block Arguments
///
/// Basic blocks can have formal parameters ("block arguments"). When a
/// terminator branches to a target block, it must supply operand values
/// for each of that block's arguments. This replaces traditional phi nodes:
/// instead of a phi instruction at block entry that selects values based on
/// the incoming edge, the predecessor directly passes the appropriate value.
///
/// This design follows MLIR/Cranelift-style block arguments, which simplify
/// SSA construction and make data flow explicit in the CFG.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct BasicBlock {
    /// Statements executed in sequence within this block.
    pub statements: ThinVec<Statement>,

    /// The terminator (control flow transfer) at the end of this block.
    /// Always present — use Terminator::Unreachable for blocks that
    /// should never terminate normally.
    pub terminator: Terminator,

    /// Block argument types. Each predecessor that branches to this block
    /// must supply operand values matching these types. The arguments are
    /// conceptually bound to fresh SSA values at the start of the block
    /// and can be referenced by subsequent statements.
    pub args: ThinVec<MirTypeId>,
}

impl BasicBlock {
    #[must_use]
    pub fn new(statements: ThinVec<Statement>, terminator: Terminator) -> Self {
        Self {
            statements,
            terminator,
            args: ThinVec::new(),
        }
    }

    /// Create a basic block with block arguments.
    #[must_use]
    pub fn new_with_args(statements: ThinVec<Statement>, terminator: Terminator, args: ThinVec<MirTypeId>) -> Self {
        Self {
            statements,
            terminator,
            args,
        }
    }

    /// Returns the successor blocks of this block's terminator.
    #[must_use]
    pub fn successors(&self) -> ThinVec<BasicBlockId> {
        match &self.terminator {
            Terminator::Goto { target, .. } => [target.clone()].into_iter().collect(),
            Terminator::If {
                true_target,
                false_target,
                ..
            } => [true_target.clone(), false_target.clone()].into_iter().collect(),
            Terminator::SwitchInt { targets, otherwise, .. } => {
                let mut succs: ThinVec<BasicBlockId> = targets.iter().map(|(_, bb, _)| bb.clone()).collect();
                succs.push(otherwise.clone());
                succs
            }
            Terminator::Return { .. } | Terminator::Unreachable => ThinVec::new(),
            Terminator::Call { target: Some(t), .. } => [t.clone()].into_iter().collect(),
            Terminator::Call { target: None, .. } => ThinVec::new(),
        }
    }
}
