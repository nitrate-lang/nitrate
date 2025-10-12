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
pub enum Literal {
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
}

impl Literal {
    pub fn size_of(&self) -> usize {
        match self {
            Literal::Unit => 0,
            Literal::Bool(_) => 1,
            Literal::I8(_) => 1,
            Literal::I16(_) => 2,
            Literal::I32(_) => 4,
            Literal::I64(_) => 8,
            Literal::I128(_) => 16,
            Literal::U8(_) => 1,
            Literal::U16(_) => 2,
            Literal::U32(_) => 4,
            Literal::U64(_) => 8,
            Literal::U128(_) => 16,
            Literal::F8(_) => 1,
            Literal::F16(_) => 2,
            Literal::F32(_) => 4,
            Literal::F64(_) => 8,
            Literal::F128(_) => 16,
        }
    }

    pub fn new_integer<T>(ty: &Literal, value: T) -> Option<Self>
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
            Literal::Unit | Literal::Bool(_) => None,
            Literal::I8(_) => value.try_into().map(Literal::I8).ok(),
            Literal::I16(_) => value.try_into().map(Literal::I16).ok(),
            Literal::I32(_) => value.try_into().map(Literal::I32).ok(),
            Literal::I64(_) => value.try_into().map(Literal::I64).ok(),
            Literal::I128(_) => value.try_into().map(Literal::I128).ok(),
            Literal::U8(_) => value.try_into().map(Literal::U8).ok(),
            Literal::U16(_) => value.try_into().map(Literal::U16).ok(),
            Literal::U32(_) => value.try_into().map(Literal::U32).ok(),
            Literal::U64(_) => value.try_into().map(Literal::U64).ok(),
            Literal::U128(_) => value.try_into().map(Literal::U128).ok(),
            Literal::F8(_)
            | Literal::F16(_)
            | Literal::F32(_)
            | Literal::F64(_)
            | Literal::F128(_) => None,
        }
    }

    pub fn new_float<T>(ty: &Literal, value: T) -> Option<Self>
    where
        T: TryInto<f32> + TryInto<f64>,
    {
        match ty {
            Literal::Unit
            | Literal::Bool(_)
            | Literal::I8(_)
            | Literal::I16(_)
            | Literal::I32(_)
            | Literal::I64(_)
            | Literal::I128(_)
            | Literal::U8(_)
            | Literal::U16(_)
            | Literal::U32(_)
            | Literal::U64(_)
            | Literal::U128(_) => None,
            Literal::F8(_) => value.try_into().map(Literal::F8).ok(),
            Literal::F16(_) => value.try_into().map(Literal::F16).ok(),
            Literal::F32(_) => value.try_into().map(Literal::F32).ok(),
            Literal::F64(_) => value.try_into().map(Literal::F64).ok(),
            Literal::F128(_) => value.try_into().map(Literal::F128).ok(),
        }
    }
}

impl Eq for Literal {}

impl std::hash::Hash for Literal {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        match self {
            Literal::Unit => {
                0u8.hash(state);
            }
            Literal::Bool(b) => {
                1u8.hash(state);
                b.hash(state);
            }
            Literal::I8(i) => {
                2u8.hash(state);
                i.hash(state);
            }
            Literal::I16(i) => {
                3u8.hash(state);
                i.hash(state);
            }
            Literal::I32(i) => {
                4u8.hash(state);
                i.hash(state);
            }
            Literal::I64(i) => {
                5u8.hash(state);
                i.hash(state);
            }
            Literal::I128(i) => {
                6u8.hash(state);
                i.hash(state);
            }
            Literal::U8(u) => {
                7u8.hash(state);
                u.hash(state);
            }
            Literal::U16(u) => {
                8u8.hash(state);
                u.hash(state);
            }
            Literal::U32(u) => {
                9u8.hash(state);
                u.hash(state);
            }
            Literal::U64(u) => {
                10u8.hash(state);
                u.hash(state);
            }
            Literal::U128(u) => {
                11u8.hash(state);
                u.hash(state);
            }
            Literal::F8(f) => {
                12u8.hash(state);
                f.to_bits().hash(state);
            }
            Literal::F16(f) => {
                13u8.hash(state);
                f.to_bits().hash(state);
            }
            Literal::F32(f) => {
                14u8.hash(state);
                f.to_bits().hash(state);
            }
            Literal::F64(f) => {
                15u8.hash(state);
                f.to_bits().hash(state);
            }
            Literal::F128(f) => {
                16u8.hash(state);
                f.to_bits().hash(state);
            }
        }
    }
}

impl IntoStoreId for Literal {
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

impl TryFrom<Value> for Literal {
    type Error = Value;

    fn try_from(value: Value) -> Result<Self, Self::Error> {
        match value {
            Value::Unit => Ok(Literal::Unit),
            Value::Bool(b) => Ok(Literal::Bool(b)),
            Value::I8(i) => Ok(Literal::I8(i)),
            Value::I16(i) => Ok(Literal::I16(i)),
            Value::I32(i) => Ok(Literal::I32(i)),
            Value::I64(i) => Ok(Literal::I64(i)),
            Value::I128(i) => Ok(Literal::I128(*i)),
            Value::U8(u) => Ok(Literal::U8(u)),
            Value::U16(u) => Ok(Literal::U16(u)),
            Value::U32(u) => Ok(Literal::U32(u)),
            Value::U64(u) => Ok(Literal::U64(u)),
            Value::U128(u) => Ok(Literal::U128(*u)),
            Value::F8(f) => Ok(Literal::F8(f)),
            Value::F16(f) => Ok(Literal::F16(f)),
            Value::F32(f) => Ok(Literal::F32(f)),
            Value::F64(f) => Ok(Literal::F64(f)),
            Value::F128(f) => Ok(Literal::F128(f)),
            other => Err(other),
        }
    }
}

impl From<Literal> for Value {
    fn from(value: Literal) -> Self {
        match value {
            Literal::Unit => Value::Unit,
            Literal::Bool(b) => Value::Bool(b),
            Literal::I8(i) => Value::I8(i),
            Literal::I16(i) => Value::I16(i),
            Literal::I32(i) => Value::I32(i),
            Literal::I64(i) => Value::I64(i),
            Literal::I128(i) => Value::I128(Box::new(i)),
            Literal::U8(u) => Value::U8(u),
            Literal::U16(u) => Value::U16(u),
            Literal::U32(u) => Value::U32(u),
            Literal::U64(u) => Value::U64(u),
            Literal::U128(u) => Value::U128(Box::new(u)),
            Literal::F8(f) => Value::F8(f),
            Literal::F16(f) => Value::F16(f),
            Literal::F32(f) => Value::F32(f),
            Literal::F64(f) => Value::F64(f),
            Literal::F128(f) => Value::F128(f),
        }
    }
}

impl IntoStoreId for Value {
    type Id = ValueId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_value(self)
    }
}
