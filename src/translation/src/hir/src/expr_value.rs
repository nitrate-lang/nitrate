use crate::{prelude::*, store::LiteralId};
use interned_string::IString;
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};

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

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Literal {
    Unit,
    Bool(bool),
    I8(i8),
    I16(i16),
    I32(i32),
    I64(i64),
    ISize(isize),
    I128(i128),
    U8(u8),
    U16(u16),
    U32(u32),
    U64(u64),
    USize(usize),
    U128(u128),
    F8(NotNan<f32>),
    F16(NotNan<f32>),
    F32(NotNan<f32>),
    F64(NotNan<f64>),
    F128(NotNan<f64>),
    String(String),
    BString(Vec<u8>),
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
    ISize(isize),
    I128(Box<i128>),
    U8(u8),
    U16(u16),
    U32(u32),
    U64(u64),
    USize(usize),
    U128(Box<u128>),
    F8(NotNan<f32>),  // Stored as f32 because Rust does not have a native f8 type
    F16(NotNan<f32>), // Stored as f32 because Rust does not have a native f16 type
    F32(NotNan<f32>),
    F64(NotNan<f64>),
    F128(NotNan<f64>), // Stored as f64 because Rust does not have a native f128 type
    String(Box<String>),
    BString(Box<Vec<u8>>),

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
            Value::ISize(i) => Ok(Literal::ISize(i)),
            Value::I128(i) => Ok(Literal::I128(*i)),
            Value::U8(u) => Ok(Literal::U8(u)),
            Value::U16(u) => Ok(Literal::U16(u)),
            Value::U32(u) => Ok(Literal::U32(u)),
            Value::U64(u) => Ok(Literal::U64(u)),
            Value::USize(u) => Ok(Literal::USize(u)),
            Value::U128(u) => Ok(Literal::U128(*u)),
            Value::F8(f) => Ok(Literal::F8(f)),
            Value::F16(f) => Ok(Literal::F16(f)),
            Value::F32(f) => Ok(Literal::F32(f)),
            Value::F64(f) => Ok(Literal::F64(f)),
            Value::F128(f) => Ok(Literal::F128(f)),
            Value::String(s) => Ok(Literal::String(*s)),
            Value::BString(b) => Ok(Literal::BString(*b)),
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
            Literal::ISize(i) => Value::ISize(i),
            Literal::I128(i) => Value::I128(Box::new(i)),
            Literal::U8(u) => Value::U8(u),
            Literal::U16(u) => Value::U16(u),
            Literal::U32(u) => Value::U32(u),
            Literal::U64(u) => Value::U64(u),
            Literal::USize(u) => Value::USize(u),
            Literal::U128(u) => Value::U128(Box::new(u)),
            Literal::F8(f) => Value::F8(f),
            Literal::F16(f) => Value::F16(f),
            Literal::F32(f) => Value::F32(f),
            Literal::F64(f) => Value::F64(f),
            Literal::F128(f) => Value::F128(f),
            Literal::String(s) => Value::String(Box::new(s)),
            Literal::BString(b) => Value::BString(Box::new(b)),
        }
    }
}

impl IntoStoreId for Value {
    type Id = ValueId;

    fn into_id(self, ctx: &Store) -> Self::Id {
        ctx.store_value(self)
    }
}
