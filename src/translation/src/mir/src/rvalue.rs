use crate::operand::{MirBinaryOp, MirUnaryOp, Operand};
use crate::place::Place;
use crate::store::MirTypeId;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use thin_vec::ThinVec;

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
    /// Construct an enum variant: `EnumName::Variant(payload)`.
    ///
    /// Carries the enum's MIR type and the variant's index so codegen can size
    /// the payload storage and write the discriminant tag without having to
    /// re-resolve declarations by name.
    Enum {
        name: NString,
        variant_name: NString,
        variant_index: u32,
        enum_ty: MirTypeId,
    },
}
