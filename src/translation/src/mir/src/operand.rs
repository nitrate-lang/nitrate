use crate::place::Place;
use crate::ty::{MirType, PtrSize};
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
        std::matches!(
            self,
            MirBinaryOp::Lt | MirBinaryOp::Gt | MirBinaryOp::Lte | MirBinaryOp::Gte | MirBinaryOp::Eq | MirBinaryOp::Ne
        )
    }

    #[must_use]
    pub fn is_logical(&self) -> bool {
        std::matches!(self, MirBinaryOp::LogicAnd | MirBinaryOp::LogicOr)
    }
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum MirUnaryOp {
    Neg, // arithmetic negation
    Not, // bitwise/logical NOT
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
