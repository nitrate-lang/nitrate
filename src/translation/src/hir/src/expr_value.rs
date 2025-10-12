use crate::{prelude::*, store::LiteralId};
use hashbrown::HashMap;
use interned_string::IString;
use serde::{Deserialize, Serialize};
use thin_str::ThinStr;
use thin_vec::ThinVec;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum BinaryOp {
    /// `+`
    Add,
    /// `-`
    Sub,
    /// `*`
    Mul,
    /// `/`
    Div,
    /// `%`
    Mod,
    /// `&`
    And,
    /// `|`
    Or,
    /// `^`
    Xor,
    /// `<<`
    Shl,
    /// `>>`
    Shr,
    /// `<<<`
    Rol,
    /// `>>>`
    Ror,
    /// `&&`
    LogicAnd,
    /// `||`
    LogicOr,
    /// `^^`
    LogicXor,
    /// `<`
    Lt,
    /// `>`
    Gt,
    /// `<=`
    Lte,
    /// `>=`
    Gte,
    /// `==`
    Eq,
    /// `!=`
    Ne,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum UnaryOp {
    /// `+`
    Add,
    /// `-`
    Sub,
    /// `~`
    BitNot,
    /// `!`
    LogicNot,
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, PartialOrd)]
pub enum Lit {
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
    F8(f32),
    F16(f32),
    F32(f32),
    F64(f64),
    F128(f64),
    USize32(u32),
    USize64(u64),
}

impl Lit {
    pub fn size_of(&self) -> usize {
        match self {
            Lit::Unit => 0,
            Lit::Bool(_) => 1,
            Lit::I8(_) => 1,
            Lit::I16(_) => 2,
            Lit::I32(_) => 4,
            Lit::I64(_) => 8,
            Lit::I128(_) => 16,
            Lit::U8(_) => 1,
            Lit::U16(_) => 2,
            Lit::U32(_) => 4,
            Lit::U64(_) => 8,
            Lit::U128(_) => 16,
            Lit::F8(_) => 1,
            Lit::F16(_) => 2,
            Lit::F32(_) => 4,
            Lit::F64(_) => 8,
            Lit::F128(_) => 16,
            Lit::USize32(_) => 4,
            Lit::USize64(_) => 8,
        }
    }

    pub fn new_integer<T>(ty: &Lit, value: T) -> Option<Self>
    where
        T: TryInto<i8>
            + TryInto<i16>
            + TryInto<i32>
            + TryInto<i64>
            + TryInto<i128>
            + TryInto<u8>
            + TryInto<u16>
            + TryInto<u32>
            + TryInto<u64>
            + TryInto<u128>,
    {
        match ty {
            Lit::I8(_) => value.try_into().map(Lit::I8).ok(),
            Lit::I16(_) => value.try_into().map(Lit::I16).ok(),
            Lit::I32(_) => value.try_into().map(Lit::I32).ok(),
            Lit::I64(_) => value.try_into().map(Lit::I64).ok(),
            Lit::I128(_) => value.try_into().map(Lit::I128).ok(),
            Lit::U8(_) => value.try_into().map(Lit::U8).ok(),
            Lit::U16(_) => value.try_into().map(Lit::U16).ok(),
            Lit::U32(_) => value.try_into().map(Lit::U32).ok(),
            Lit::U64(_) => value.try_into().map(Lit::U64).ok(),
            Lit::U128(_) => value.try_into().map(Lit::U128).ok(),

            Lit::Unit
            | Lit::Bool(_)
            | Lit::F8(_)
            | Lit::F16(_)
            | Lit::F32(_)
            | Lit::F64(_)
            | Lit::F128(_)
            | Lit::USize32(_)
            | Lit::USize64(_) => None,
        }
    }

    pub fn new_float<T>(ty: &Lit, value: T) -> Option<Self>
    where
        T: TryInto<f32> + TryInto<f64>,
    {
        match ty {
            Lit::Unit
            | Lit::Bool(_)
            | Lit::I8(_)
            | Lit::I16(_)
            | Lit::I32(_)
            | Lit::I64(_)
            | Lit::I128(_)
            | Lit::U8(_)
            | Lit::U16(_)
            | Lit::U32(_)
            | Lit::U64(_)
            | Lit::U128(_)
            | Lit::USize32(_)
            | Lit::USize64(_) => None,

            Lit::F8(_) => value.try_into().map(Lit::F8).ok(),
            Lit::F16(_) => value.try_into().map(Lit::F16).ok(),
            Lit::F32(_) => value.try_into().map(Lit::F32).ok(),
            Lit::F64(_) => value.try_into().map(Lit::F64).ok(),
            Lit::F128(_) => value.try_into().map(Lit::F128).ok(),
        }
    }
}

impl Eq for Lit {}

impl std::hash::Hash for Lit {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        match self {
            Lit::Unit => {
                0u8.hash(state);
            }
            Lit::Bool(b) => {
                1u8.hash(state);
                b.hash(state);
            }
            Lit::I8(i) => {
                2u8.hash(state);
                i.hash(state);
            }
            Lit::I16(i) => {
                3u8.hash(state);
                i.hash(state);
            }
            Lit::I32(i) => {
                4u8.hash(state);
                i.hash(state);
            }
            Lit::I64(i) => {
                5u8.hash(state);
                i.hash(state);
            }
            Lit::I128(i) => {
                6u8.hash(state);
                i.hash(state);
            }
            Lit::U8(u) => {
                7u8.hash(state);
                u.hash(state);
            }
            Lit::U16(u) => {
                8u8.hash(state);
                u.hash(state);
            }
            Lit::U32(u) => {
                9u8.hash(state);
                u.hash(state);
            }
            Lit::U64(u) => {
                10u8.hash(state);
                u.hash(state);
            }
            Lit::U128(u) => {
                11u8.hash(state);
                u.hash(state);
            }
            Lit::F8(f) => {
                12u8.hash(state);
                f.to_bits().hash(state);
            }
            Lit::F16(f) => {
                13u8.hash(state);
                f.to_bits().hash(state);
            }
            Lit::F32(f) => {
                14u8.hash(state);
                f.to_bits().hash(state);
            }
            Lit::F64(f) => {
                15u8.hash(state);
                f.to_bits().hash(state);
            }
            Lit::F128(f) => {
                16u8.hash(state);
                f.to_bits().hash(state);
            }
            Lit::USize32(u) => {
                17u8.hash(state);
                u.hash(state);
            }
            Lit::USize64(u) => {
                18u8.hash(state);
                u.hash(state);
            }
        }
    }
}

impl IntoStoreId for Lit {
    type Id = LiteralId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_literal(self)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum BlockSafety {
    Safe,
    Unsafe,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Block {
    pub safety: BlockSafety,
    pub exprs: Vec<ValueId>,
}

impl IntoStoreId for Block {
    type Id = BlockId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_block(self)
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Value {
    Unit,
    Bool(bool),
    I8(i8),
    I16(i16),
    I32(i32),
    I64(i64),
    I128(Box<i128>),
    U8(u8),
    U16(u16),
    U32(u32),
    U64(u64),
    U128(Box<u128>),
    F8(f32),  // Stored as f32 because Rust does not have a native f8 type
    F16(f32), // Stored as f32 because Rust does not have a native f16 type
    F32(f32),
    F64(f64),
    F128(f64), // Stored as f64 because Rust does not have a native f128 type
    USize32(u32),
    USize64(u64),
    String(ThinStr),
    BString(ThinVec<u8>),

    Struct {
        struct_type: TypeId,
        fields: Box<HashMap<IString, ValueId>>,
    },

    Enum {
        enum_type: TypeId,
        variant: IString,
        value: ValueId,
    },

    Binary {
        left: ValueId,
        op: BinaryOp,
        right: ValueId,
    },

    Unary {
        op: UnaryOp,
        expr: ValueId,
    },

    FieldAccess {
        expr: ValueId,
        field: IString,
    },

    ArrayIndex {
        expr: ValueId,
        index: ValueId,
    },

    Assign {
        place: PlaceId,
        value: ValueId,
    },

    Deref {
        place: PlaceId,
    },

    Cast {
        expr: ValueId,
        to: TypeId,
    },

    GetAddressOf {
        place: PlaceId,
    },

    GetTypeOf {
        expr: ValueId,
    },

    List {
        elements: Box<Vec<ValueId>>,
    },

    If {
        condition: ValueId,
        true_branch: BlockId,
        false_branch: Option<BlockId>,
    },

    While {
        condition: ValueId,
        body: BlockId,
    },

    Loop {
        body: BlockId,
    },

    Break {
        label: Option<IString>,
    },

    Continue {
        label: Option<IString>,
    },

    Return {
        value: ValueId,
    },

    Block {
        block: BlockId,
    },

    Call {
        callee: ValueId,
        arguments: Box<Vec<ValueId>>,
    },

    Symbol {
        symbol: SymbolId,
    },
}

impl TryFrom<Value> for Lit {
    type Error = Value;

    fn try_from(value: Value) -> Result<Self, Self::Error> {
        match value {
            Value::Unit => Ok(Lit::Unit),
            Value::Bool(b) => Ok(Lit::Bool(b)),
            Value::I8(i) => Ok(Lit::I8(i)),
            Value::I16(i) => Ok(Lit::I16(i)),
            Value::I32(i) => Ok(Lit::I32(i)),
            Value::I64(i) => Ok(Lit::I64(i)),
            Value::I128(i) => Ok(Lit::I128(*i)),
            Value::U8(u) => Ok(Lit::U8(u)),
            Value::U16(u) => Ok(Lit::U16(u)),
            Value::U32(u) => Ok(Lit::U32(u)),
            Value::U64(u) => Ok(Lit::U64(u)),
            Value::U128(u) => Ok(Lit::U128(*u)),
            Value::F8(f) => Ok(Lit::F8(f)),
            Value::F16(f) => Ok(Lit::F16(f)),
            Value::F32(f) => Ok(Lit::F32(f)),
            Value::F64(f) => Ok(Lit::F64(f)),
            Value::F128(f) => Ok(Lit::F128(f)),
            Value::USize32(u) => Ok(Lit::USize32(u)),
            Value::USize64(u) => Ok(Lit::USize64(u)),
            other => Err(other),
        }
    }
}

impl From<Lit> for Value {
    fn from(value: Lit) -> Self {
        match value {
            Lit::Unit => Value::Unit,
            Lit::Bool(b) => Value::Bool(b),
            Lit::I8(i) => Value::I8(i),
            Lit::I16(i) => Value::I16(i),
            Lit::I32(i) => Value::I32(i),
            Lit::I64(i) => Value::I64(i),
            Lit::I128(i) => Value::I128(Box::new(i)),
            Lit::U8(u) => Value::U8(u),
            Lit::U16(u) => Value::U16(u),
            Lit::U32(u) => Value::U32(u),
            Lit::U64(u) => Value::U64(u),
            Lit::U128(u) => Value::U128(Box::new(u)),
            Lit::F8(f) => Value::F8(f),
            Lit::F16(f) => Value::F16(f),
            Lit::F32(f) => Value::F32(f),
            Lit::F64(f) => Value::F64(f),
            Lit::F128(f) => Value::F128(f),
            Lit::USize32(u) => Value::USize32(u),
            Lit::USize64(u) => Value::USize64(u),
        }
    }
}

impl IntoStoreId for Value {
    type Id = ValueId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_value(self)
    }
}
