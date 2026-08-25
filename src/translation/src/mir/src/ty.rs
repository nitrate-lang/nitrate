use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use std::matches;
use std::num::NonZeroU32;
use std::ops::Deref;
use thin_vec::ThinVec;

// Re-export from store for convenience
use crate::store::MirTypeId;

/// A fully concrete MIR type — no inference, no generics.
/// All types are monomorphized and resolved at MIR construction time.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum MirType {
    Never,
    Unit,
    Bool,
    U8,
    U16,
    U32,
    U64,
    U128,
    USize,
    I8,
    I16,
    I32,
    I64,
    I128,
    F32,
    F64,
    Array {
        element_type: MirTypeId,
        len: u32,
    },
    Tuple {
        element_types: ThinVec<MirTypeId>,
    },
    Struct {
        name: NString,
        fields: ThinVec<(NString, MirTypeId)>,
        layout: MirStructLayout,
    },
    Enum {
        name: NString,
        variants: ThinVec<MirEnumVariant>,
    },
    Function {
        params: ThinVec<(NString, MirTypeId)>,
        return_type: MirTypeId,
        is_c_variadic: bool,
    },
    Reference {
        exclusive: bool,
        mutable: bool,
        to: MirTypeId,
    },
    SliceRef {
        exclusive: bool,
        mutable: bool,
        element_type: MirTypeId,
    },
    Pointer {
        exclusive: bool,
        mutable: bool,
        to: MirTypeId,
    },
    SlicePtr {
        exclusive: bool,
        mutable: bool,
        element_type: MirTypeId,
    },
    Range,
    Str,
}

/// Memory layout cell for structs.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum MirStructLayoutCell {
    Field { field_name: NString },
    Padding(NonZeroU32),
}

pub type MirStructLayout = ThinVec<MirStructLayoutCell>;

/// A variant of an enum, with optional payload.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirEnumVariant {
    pub name: NString,
    pub payload: Option<MirTypeId>,
}

/// Target pointer size.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub enum PtrSize {
    U32 = 4,
    U64 = 8,
}

// ── Helpers ──────────────────────────────────────────────────

impl MirType {
    #[must_use]
    pub fn is_diverging(&self) -> bool {
        matches!(self, MirType::Never)
    }
    #[must_use]
    pub fn is_bool(&self) -> bool {
        matches!(self, MirType::Bool)
    }
    #[must_use]
    pub fn is_unsigned_primitive(&self) -> bool {
        matches!(
            self,
            MirType::U8 | MirType::U16 | MirType::U32 | MirType::U64 | MirType::U128 | MirType::USize
        )
    }
    #[must_use]
    pub fn is_signed_primitive(&self) -> bool {
        matches!(
            self,
            MirType::I8 | MirType::I16 | MirType::I32 | MirType::I64 | MirType::I128
        )
    }
    #[must_use]
    pub fn is_integer_primitive(&self) -> bool {
        self.is_unsigned_primitive() || self.is_signed_primitive()
    }
    #[must_use]
    pub fn is_float_primitive(&self) -> bool {
        matches!(self, MirType::F32 | MirType::F64)
    }
    #[must_use]
    pub fn is_reference(&self) -> bool {
        matches!(self, MirType::Reference { .. })
    }
    #[must_use]
    pub fn is_pointer(&self) -> bool {
        matches!(self, MirType::Pointer { .. })
    }
    #[must_use]
    pub fn is_slice_ref(&self) -> bool {
        matches!(self, MirType::SliceRef { .. })
    }
    #[must_use]
    pub fn is_slice_ptr(&self) -> bool {
        matches!(self, MirType::SlicePtr { .. })
    }
    #[must_use]
    pub fn is_aggregate(&self) -> bool {
        matches!(
            self,
            MirType::Array { .. } | MirType::Tuple { .. } | MirType::Struct { .. } | MirType::Enum { .. }
        )
    }
    #[must_use]
    pub fn is_array(&self) -> bool {
        matches!(self, MirType::Array { .. })
    }
    #[must_use]
    pub fn is_tuple(&self) -> bool {
        matches!(self, MirType::Tuple { .. })
    }
    #[must_use]
    pub fn is_struct(&self) -> bool {
        matches!(self, MirType::Struct { .. })
    }
    #[must_use]
    pub fn is_enum(&self) -> bool {
        matches!(self, MirType::Enum { .. })
    }
    #[must_use]
    pub fn is_function(&self) -> bool {
        matches!(self, MirType::Function { .. })
    }
    #[must_use]
    pub fn is_zst(&self) -> bool {
        match self {
            MirType::Never | MirType::Unit | MirType::Range => true,
            MirType::Tuple { element_types } => element_types.iter().all(|t| t.deref().is_zst()),
            MirType::Struct { fields, .. } => fields.iter().all(|(_, t)| t.deref().is_zst()),
            MirType::Enum { variants, .. } => variants
                .iter()
                .all(|v| v.payload.as_ref().map_or(true, |t| t.deref().is_zst())),
            _ => false,
        }
    }
}

// ── PartialEq, Eq, Hash ─────────────────────────────────────

impl PartialEq for MirType {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (MirType::Never, MirType::Never) => true,
            (MirType::Unit, MirType::Unit) => true,
            (MirType::Bool, MirType::Bool) => true,
            (MirType::U8, MirType::U8) => true,
            (MirType::U16, MirType::U16) => true,
            (MirType::U32, MirType::U32) => true,
            (MirType::U64, MirType::U64) => true,
            (MirType::U128, MirType::U128) => true,
            (MirType::USize, MirType::USize) => true,
            (MirType::I8, MirType::I8) => true,
            (MirType::I16, MirType::I16) => true,
            (MirType::I32, MirType::I32) => true,
            (MirType::I64, MirType::I64) => true,
            (MirType::I128, MirType::I128) => true,
            (MirType::F32, MirType::F32) => true,
            (MirType::F64, MirType::F64) => true,
            (
                MirType::Array {
                    element_type: e1,
                    len: l1,
                },
                MirType::Array {
                    element_type: e2,
                    len: l2,
                },
            ) => e1 == e2 && l1 == l2,
            (MirType::Tuple { element_types: et1 }, MirType::Tuple { element_types: et2 }) => et1 == et2,
            (
                MirType::Struct {
                    name: n1,
                    fields: f1,
                    layout: l1,
                },
                MirType::Struct {
                    name: n2,
                    fields: f2,
                    layout: l2,
                },
            ) => n1 == n2 && f1 == f2 && l1 == l2,
            (MirType::Enum { name: n1, variants: v1 }, MirType::Enum { name: n2, variants: v2 }) => {
                n1 == n2 && v1 == v2
            }
            (
                MirType::Function {
                    params: p1,
                    return_type: r1,
                    is_c_variadic: cv1,
                },
                MirType::Function {
                    params: p2,
                    return_type: r2,
                    is_c_variadic: cv2,
                },
            ) => p1 == p2 && r1 == r2 && cv1 == cv2,
            (
                MirType::Reference {
                    exclusive: ex1,
                    mutable: m1,
                    to: t1,
                },
                MirType::Reference {
                    exclusive: ex2,
                    mutable: m2,
                    to: t2,
                },
            ) => ex1 == ex2 && m1 == m2 && t1 == t2,
            (
                MirType::SliceRef {
                    exclusive: ex1,
                    mutable: m1,
                    element_type: e1,
                },
                MirType::SliceRef {
                    exclusive: ex2,
                    mutable: m2,
                    element_type: e2,
                },
            ) => ex1 == ex2 && m1 == m2 && e1 == e2,
            (
                MirType::Pointer {
                    exclusive: ex1,
                    mutable: m1,
                    to: t1,
                },
                MirType::Pointer {
                    exclusive: ex2,
                    mutable: m2,
                    to: t2,
                },
            ) => ex1 == ex2 && m1 == m2 && t1 == t2,
            (
                MirType::SlicePtr {
                    exclusive: ex1,
                    mutable: m1,
                    element_type: e1,
                },
                MirType::SlicePtr {
                    exclusive: ex2,
                    mutable: m2,
                    element_type: e2,
                },
            ) => ex1 == ex2 && m1 == m2 && e1 == e2,
            (MirType::Range, MirType::Range) => true,
            (MirType::Str, MirType::Str) => true,
            _ => false,
        }
    }
}
impl Eq for MirType {}

impl std::hash::Hash for MirType {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        let disc: u8 = match self {
            MirType::Never => 0,
            MirType::Unit => 1,
            MirType::Bool => 2,
            MirType::U8 => 3,
            MirType::U16 => 4,
            MirType::U32 => 5,
            MirType::U64 => 6,
            MirType::U128 => 7,
            MirType::USize => 8,
            MirType::I8 => 9,
            MirType::I16 => 10,
            MirType::I32 => 11,
            MirType::I64 => 12,
            MirType::I128 => 13,
            MirType::F32 => 14,
            MirType::F64 => 15,
            MirType::Array { .. } => 16,
            MirType::Tuple { .. } => 17,
            MirType::Struct { .. } => 18,
            MirType::Enum { .. } => 19,
            MirType::Function { .. } => 20,
            MirType::Reference { .. } => 21,
            MirType::SliceRef { .. } => 22,
            MirType::Pointer { .. } => 23,
            MirType::SlicePtr { .. } => 24,
            MirType::Range => 25,
            MirType::Str => 26,
        };
        disc.hash(state);
        match self {
            MirType::Never
            | MirType::Unit
            | MirType::Bool
            | MirType::U8
            | MirType::U16
            | MirType::U32
            | MirType::U64
            | MirType::U128
            | MirType::USize
            | MirType::I8
            | MirType::I16
            | MirType::I32
            | MirType::I64
            | MirType::I128
            | MirType::F32
            | MirType::F64
            | MirType::Range
            | MirType::Str => {}
            MirType::Array { element_type, len } => {
                element_type.hash(state);
                len.hash(state);
            }
            MirType::Tuple { element_types } => {
                element_types.hash(state);
            }
            MirType::Struct { name, fields, layout } => {
                name.hash(state);
                fields.hash(state);
                layout.hash(state);
            }
            MirType::Enum { name, variants } => {
                name.hash(state);
                variants.hash(state);
            }
            MirType::Function {
                params,
                return_type,
                is_c_variadic,
            } => {
                params.hash(state);
                return_type.hash(state);
                is_c_variadic.hash(state);
            }
            MirType::Reference { exclusive, mutable, to } => {
                exclusive.hash(state);
                mutable.hash(state);
                to.hash(state);
            }
            MirType::SliceRef {
                exclusive,
                mutable,
                element_type,
            } => {
                exclusive.hash(state);
                mutable.hash(state);
                element_type.hash(state);
            }
            MirType::Pointer { exclusive, mutable, to } => {
                exclusive.hash(state);
                mutable.hash(state);
                to.hash(state);
            }
            MirType::SlicePtr {
                exclusive,
                mutable,
                element_type,
            } => {
                exclusive.hash(state);
                mutable.hash(state);
                element_type.hash(state);
            }
        }
    }
}
