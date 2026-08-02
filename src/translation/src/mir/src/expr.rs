use std::matches;

use crate::prelude::*;
use nitrate_nstring::NString;
use ordered_float::OrderedFloat;
use serde::{Deserialize, Serialize};
use thin_str::ThinStr;
use thin_vec::ThinVec;

// ─────────────────────────────────────────────────────────────
// Literals
// ─────────────────────────────────────────────────────────────

/// A compile-time constant literal value.
/// Mirrors HIR's `Lit` but without source span information.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum MirLiteral {
    Unit,
    Bool(bool),
    I8(i8),
    I16(i16),
    I32(i32),
    I64(i64),
    I128(i128),
    U8(u8),
    U16(u16),
    U32(u32),
    U64(u64),
    U128(u128),
    F32(OrderedFloat<f32>),
    F64(OrderedFloat<f64>),
    USize {
        bits: u8,
        value: u64,
    },
    /// UTF-8 string literal (not null-terminated)
    Str(ThinStr),
    /// Byte string literal
    BStr(ThinVec<u8>),
}

impl MirLiteral {
    #[must_use]
    pub fn ty(&self, ptr_size: PtrSize) -> MirType {
        match self {
            MirLiteral::Unit => MirType::Unit,
            MirLiteral::Bool(_) => MirType::Bool,
            MirLiteral::I8(_) => MirType::I8,
            MirLiteral::I16(_) => MirType::I16,
            MirLiteral::I32(_) => MirType::I32,
            MirLiteral::I64(_) => MirType::I64,
            MirLiteral::I128(_) => MirType::I128,
            MirLiteral::U8(_) => MirType::U8,
            MirLiteral::U16(_) => MirType::U16,
            MirLiteral::U32(_) => MirType::U32,
            MirLiteral::U64(_) => MirType::U64,
            MirLiteral::U128(_) => MirType::U128,
            MirLiteral::F32(_) => MirType::F32,
            MirLiteral::F64(_) => MirType::F64,
            MirLiteral::USize { bits, .. } if *bits == 64 => MirType::U64,
            MirLiteral::USize { bits, .. } if *bits == 32 => MirType::U32,
            MirLiteral::USize { .. } => match ptr_size {
                PtrSize::U32 => MirType::U32,
                PtrSize::U64 => MirType::U64,
            },
            MirLiteral::Str(_) => MirType::Str,
            MirLiteral::BStr(_) => MirType::SliceRef {
                exclusive: false,
                mutable: false,
                element_type: MirType::U8.into(),
            },
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Binary and Unary Operations
// ─────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum MirBinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    And,
    Or,
    Xor,
    Shl,
    Shr,
    Rol,
    Ror,
    LogicAnd,
    LogicOr,
    Lt,
    Gt,
    Lte,
    Gte,
    Eq,
    Ne,
}

impl MirBinaryOp {
    #[must_use]
    pub fn is_comparison(&self) -> bool {
        matches!(
            self,
            MirBinaryOp::Lt | MirBinaryOp::Gt | MirBinaryOp::Lte | MirBinaryOp::Gte | MirBinaryOp::Eq | MirBinaryOp::Ne
        )
    }

    #[must_use]
    pub fn is_logical(&self) -> bool {
        matches!(self, MirBinaryOp::LogicAnd | MirBinaryOp::LogicOr)
    }
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum MirUnaryOp {
    Neg, // arithmetic negation
    Not, // bitwise/logical NOT
}

// ─────────────────────────────────────────────────────────────
// Places — memory locations
// ─────────────────────────────────────────────────────────────

/// A Place represents a memory location: where data is stored.
/// Places are used to read from (via `Operand::Copy` or `Operand::Move`)
/// and write to (via `Statement::Assign`).
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Place {
    /// A local variable (SSA register). The type is always Some because
    /// all locals are declared with known types in MIR.
    Local(LocalId),

    /// A static (global) variable, referenced by name.
    Static(NString),

    /// Dereference of a pointer/reference: `*place`
    Deref(Box<Place>),

    /// Field access on a struct: `place.field_name`
    Field { base: Box<Place>, field_name: NString },

    /// Index into an array or slice: `place[index]`
    Index { base: Box<Place>, index: Box<Place> },

    /// Downcast to a specific enum variant: `place as VariantName`
    Downcast { base: Box<Place>, variant_name: NString },
}

impl Place {
    /// Returns the Place for a local variable.
    #[must_use]
    pub fn local(local: LocalId) -> Self {
        Place::Local(local)
    }

    /// Returns the Place for a static/global.
    #[must_use]
    pub fn static_(name: NString) -> Self {
        Place::Static(name)
    }

    /// Returns a Deref place.
    #[must_use]
    pub fn deref(base: Place) -> Self {
        Place::Deref(Box::new(base))
    }

    /// Returns a Field place.
    #[must_use]
    pub fn field(base: Place, field_name: NString) -> Self {
        Place::Field {
            base: Box::new(base),
            field_name,
        }
    }

    /// Returns an Index place.
    #[must_use]
    pub fn index(base: Place, index: Place) -> Self {
        Place::Index {
            base: Box::new(base),
            index: Box::new(index),
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Operands — values read from places or constants
// ─────────────────────────────────────────────────────────────

/// An Operand is a value used by an Rvalue or Terminator.
/// It is either a copy/move from a Place, or a compile-time constant.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Operand {
    /// Copy the value from the place. The place remains usable.
    Copy(Place),
    /// Move the value from the place. The place becomes deinitialized.
    Move(Place),
    /// A compile-time constant literal.
    Constant(MirLiteral),
}

// ─────────────────────────────────────────────────────────────
// Rvalues — computed values
// ─────────────────────────────────────────────────────────────

/// An Rvalue produces a value. Rvalues are the right-hand side of `Assign`
/// statements. Unlike HIR expressions which form a full AST, Rvalues
/// are flat — operands are already evaluated Place references or constants.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Rvalue {
    /// Use an operand directly (copy or move).
    /// This is the identity rvalue: reading from a place or constant.
    Use(Operand),

    /// Create a reference: `&place` or `&mut place`
    Ref { region: BorrowKind, place: Place },

    /// Get the length of a slice: `slice.len()`
    Len(Place),

    /// Type cast: `value as TargetType`
    Cast { value: Operand, target_ty: MirTypeId },

    /// Binary operation on two operands
    BinaryOp {
        op: MirBinaryOp,
        lhs: Operand,
        rhs: Operand,
    },

    /// Checked binary operation with overflow detection.
    /// Produces `(result, overflow_flag: bool)`.
    CheckedBinaryOp {
        op: MirBinaryOp,
        lhs: Operand,
        rhs: Operand,
    },

    /// Unary operation
    UnaryOp { op: MirUnaryOp, operand: Operand },

    /// Nullary operation: produce metadata about a type.
    NullaryOp(NullaryOp, MirTypeId),

    /// Aggregate construction — builds a struct, tuple, array, or enum variant.
    /// Each operand is the value for one element/field.
    Aggregate(AggregateKind, ThinVec<Operand>),
}

/// The kind of borrow: shared or mutable.
#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum BorrowKind {
    /// `&T` — shared reference
    Shared,
    /// `&mut T` — mutable (exclusive) reference
    Mutable,
}

/// A nullary (zero-operand) operation on a type.
#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum NullaryOp {
    /// `size_of::<T>()` — return the size in bytes of T
    SizeOf,
    /// `align_of::<T>()` — return the required alignment in bytes of T
    AlignOf,
}

/// Describes how to construct an aggregate value.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum AggregateKind {
    /// Construct a tuple: `(a, b, c)`
    Tuple,
    /// Construct an array: `[a; N]` — all elements have the same value
    Array(MirTypeId),
    /// Construct a struct literal: `StructName { field1: val1, ... }`
    /// The field names are stored separately for matching.
    Struct(NString, ThinVec<NString>),
    /// Construct an enum variant: `EnumName::Variant(payload)`
    Enum(NString, NString),
}

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
// Local declarations
// ─────────────────────────────────────────────────────────────

/// A local variable declaration. Each local has a known type and mutability.
/// In SSA form, each local is assigned exactly once within a function.
/// Multiple versions of the same source variable get different LocalIds.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct LocalDecl {
    pub ty: MirTypeId,
    pub mutable: bool,
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

// ─────────────────────────────────────────────────────────────
// MIR Function
// ─────────────────────────────────────────────────────────────

/// The complete MIR representation of a function body.
/// Contains local declarations, the control flow graph (as basic blocks),
/// and function metadata.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirFunction {
    /// Function name (mangled, for codegen)
    pub name: NString,

    /// Parameter declarations.
    /// Parameters are stored as locals — the first N locals are params.
    /// This is intentionally typed as Vec<LocalId> not Option.
    pub params: ThinVec<LocalId>,

    /// Return type of the function.
    pub return_ty: MirTypeId,

    /// All local variable declarations (parameters + temporaries + user variables).
    pub locals: ThinVec<LocalDecl>,

    /// The entry basic block (where execution starts).
    pub entry_block: BasicBlockId,

    /// All basic blocks in the function body.
    pub blocks: ThinVec<BasicBlockId>,

    /// Number of "arguments" (parameter locals). The first `arg_count`
    /// entries in `locals` are parameters.
    pub arg_count: u32,
}

impl MirFunction {
    #[must_use]
    pub fn new(
        name: NString,
        params: ThinVec<LocalId>,
        return_ty: MirTypeId,
        locals: ThinVec<LocalDecl>,
        entry_block: BasicBlockId,
        blocks: ThinVec<BasicBlockId>,
        arg_count: u32,
    ) -> Self {
        Self {
            name,
            params,
            return_ty,
            locals,
            entry_block,
            blocks,
            arg_count,
        }
    }

    /// Iterate over all basic blocks in the function.
    #[must_use]
    pub fn iter_blocks(&self) -> impl Iterator<Item = &BasicBlockId> {
        self.blocks.iter()
    }

    /// Returns true if the function has no body (external/extern function).
    #[must_use]
    pub fn is_extern(&self) -> bool {
        self.blocks.is_empty()
    }
}

// ─────────────────────────────────────────────────────────────
// Module-level MIR
// ─────────────────────────────────────────────────────────────

/// A complete MIR module, containing all functions for a compilation unit.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirModule {
    /// Functions defined in this module (mangled name → function).
    pub functions: ThinVec<MirFunctionId>,
    /// Global variable declarations referenced by this module.
    pub globals: ThinVec<(NString, MirTypeId)>,
    /// Target pointer size
    pub ptr_size: PtrSize,
}

impl MirModule {
    #[must_use]
    pub fn new(functions: ThinVec<MirFunctionId>, globals: ThinVec<(NString, MirTypeId)>, ptr_size: PtrSize) -> Self {
        Self {
            functions,
            globals,
            ptr_size,
        }
    }
}
