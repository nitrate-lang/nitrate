use crate::{prelude::*, store::LiteralId};
use nitrate_nstring::NString;
use ordered_float::OrderedFloat;
use serde::{Deserialize, Serialize};
use thin_str::ThinStr;
use thin_vec::ThinVec;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
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

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum UnaryOp {
    /// `+`
    Add,
    /// `-`
    Sub,
    /// `!`
    Not,
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
    F32(OrderedFloat<f32>),
    F64(OrderedFloat<f64>),
    USize32(u32),
    USize64(u64),
}

impl Lit {
    #[must_use]
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
            Lit::F32(_) => 4,
            Lit::F64(_) => 8,
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
            | Lit::F32(_)
            | Lit::F64(_)
            | Lit::USize32(_)
            | Lit::USize64(_) => None,
        }
    }

    pub fn new_float<T>(ty: &Lit, value: T) -> Option<Self>
    where
        T: TryInto<OrderedFloat<f32>> + TryInto<OrderedFloat<f64>>,
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

            Lit::F32(_) => value.try_into().map(Lit::F32).ok(),
            Lit::F64(_) => value.try_into().map(Lit::F64).ok(),
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
            Lit::F32(f) => {
                14u8.hash(state);
                f.to_bits().hash(state);
            }
            Lit::F64(f) => {
                15u8.hash(state);
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

impl std::fmt::Display for Lit {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Lit::Unit => write!(f, "()"),
            Lit::Bool(b) => write!(f, "{b}"),
            Lit::I8(i) => write!(f, "{i}_i8"),
            Lit::I16(i) => write!(f, "{i}_i16"),
            Lit::I32(i) => write!(f, "{i}_i32"),
            Lit::I64(i) => write!(f, "{i}_i64"),
            Lit::I128(i) => write!(f, "{i}_i128"),
            Lit::U8(u) => write!(f, "{u}_u8"),
            Lit::U16(u) => write!(f, "{u}_u16"),
            Lit::U32(u) => write!(f, "{u}_u32"),
            Lit::U64(u) => write!(f, "{u}_u64"),
            Lit::U128(u) => write!(f, "{u}_u128"),
            Lit::F32(fl) => write!(f, "{fl}_f32"),
            Lit::F64(fl) => write!(f, "{fl}_f64"),
            Lit::USize32(u) => write!(f, "{u}_usize"),
            Lit::USize64(u) => write!(f, "{u}_usize"),
        }
    }
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum BlockSafety {
    Safe,
    Unsafe,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum BlockElement {
    Expr(ValueId),
    Local(LocalVariableId),
}

impl BlockElement {
    #[must_use]
    pub fn as_expr(&self) -> Option<&ValueId> {
        if let BlockElement::Expr(expr) = self {
            Some(expr)
        } else {
            None
        }
    }

    #[must_use]
    pub fn as_local(&self) -> Option<&LocalVariableId> {
        if let BlockElement::Local(local) = self {
            Some(local)
        } else {
            None
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Block {
    pub safety: BlockSafety,
    pub elements: Vec<BlockElement>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
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
    F32(OrderedFloat<f32>),
    F64(OrderedFloat<f64>),
    USize32(u32),
    USize64(u64),
    StringLit(ThinStr),
    BStringLit(ThinVec<u8>),
    InferredInteger(Box<u128>),
    InferredFloat(OrderedFloat<f64>),

    StructObject {
        struct_def: StructDefId,
        fields: ThinVec<(NString, ValueId)>,
    },

    EnumVariant {
        enum_def: EnumDefId,
        variant: NString,
        value: ValueId,
    },

    Binary {
        left: ValueId,
        op: BinaryOp,
        right: ValueId,
    },

    Unary {
        op: UnaryOp,
        operand: ValueId,
    },

    FieldAccess {
        expr: ValueId,
        field_name: NString,
    },

    Assign {
        place: ValueId,
        value: ValueId,
    },

    Deref {
        place: ValueId,
    },

    Cast {
        value: ValueId,
        target_type: TypeId,
    },

    Borrow {
        exclusive: bool,
        mutable: bool,
        place: ValueId,
    },

    List {
        elements: ThinVec<Value>,
    },

    Tuple {
        elements: ThinVec<Value>,
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
        label: Option<NString>,
    },

    Continue {
        label: Option<NString>,
    },

    Return {
        value: ValueId,
    },

    Block {
        block: BlockId,
    },

    Closure {
        captures: ThinVec<NString>,
        callee: FunctionId,
    },

    Call {
        callee: ValueId,
        positional: ThinVec<ValueId>,
        named: ThinVec<(NString, ValueId)>,
    },

    MethodCall {
        object: ValueId,
        method_name: NString,
        positional: ThinVec<ValueId>,
        named: ThinVec<(NString, ValueId)>,
    },

    FunctionSymbol {
        id: FunctionId,
    },

    GlobalVariableSymbol {
        id: GlobalVariableId,
    },

    LocalVariableSymbol {
        id: LocalVariableId,
    },

    ParameterSymbol {
        id: ParameterId,
    },
}

impl Value {
    #[must_use]
    pub fn is_unit(&self) -> bool {
        matches!(self, Value::Unit)
    }

    #[must_use]
    pub fn is_bool(&self) -> bool {
        matches!(self, Value::Bool(_))
    }

    #[must_use]
    pub fn is_i8(&self) -> bool {
        matches!(self, Value::I8(_))
    }

    #[must_use]
    pub fn is_i16(&self) -> bool {
        matches!(self, Value::I16(_))
    }

    #[must_use]
    pub fn is_i32(&self) -> bool {
        matches!(self, Value::I32(_))
    }

    #[must_use]
    pub fn is_i64(&self) -> bool {
        matches!(self, Value::I64(_))
    }

    #[must_use]
    pub fn is_i128(&self) -> bool {
        matches!(self, Value::I128(_))
    }

    #[must_use]
    pub fn is_u8(&self) -> bool {
        matches!(self, Value::U8(_))
    }

    #[must_use]
    pub fn is_u16(&self) -> bool {
        matches!(self, Value::U16(_))
    }

    #[must_use]
    pub fn is_u32(&self) -> bool {
        matches!(self, Value::U32(_))
    }

    #[must_use]
    pub fn is_u64(&self) -> bool {
        matches!(self, Value::U64(_))
    }

    #[must_use]
    pub fn is_u128(&self) -> bool {
        matches!(self, Value::U128(_))
    }

    #[must_use]
    pub fn is_f32(&self) -> bool {
        matches!(self, Value::F32(_))
    }

    #[must_use]
    pub fn is_f64(&self) -> bool {
        matches!(self, Value::F64(_))
    }

    #[must_use]
    pub fn is_usize(&self) -> bool {
        matches!(self, Value::USize32(_) | Value::USize64(_))
    }

    #[must_use]
    pub fn is_string_lit(&self) -> bool {
        matches!(self, Value::StringLit(_))
    }

    #[must_use]
    pub fn is_bstring_lit(&self) -> bool {
        matches!(self, Value::BStringLit(_))
    }

    #[must_use]
    pub fn is_inferred_integer(&self) -> bool {
        matches!(self, Value::InferredInteger(_))
    }

    #[must_use]
    pub fn is_inferred_float(&self) -> bool {
        matches!(self, Value::InferredFloat(_))
    }

    #[must_use]
    pub fn is_struct_object(&self) -> bool {
        matches!(self, Value::StructObject { .. })
    }

    #[must_use]
    pub fn is_enum_variant(&self) -> bool {
        matches!(self, Value::EnumVariant { .. })
    }

    #[must_use]
    pub fn is_binary(&self) -> bool {
        matches!(self, Value::Binary { .. })
    }

    #[must_use]
    pub fn is_unary(&self) -> bool {
        matches!(self, Value::Unary { .. })
    }

    #[must_use]
    pub fn is_field_access(&self) -> bool {
        matches!(self, Value::FieldAccess { .. })
    }

    #[must_use]
    pub fn is_assign(&self) -> bool {
        matches!(self, Value::Assign { .. })
    }

    #[must_use]
    pub fn is_deref(&self) -> bool {
        matches!(self, Value::Deref { .. })
    }

    #[must_use]
    pub fn is_cast(&self) -> bool {
        matches!(self, Value::Cast { .. })
    }

    #[must_use]
    pub fn is_borrow(&self) -> bool {
        matches!(self, Value::Borrow { .. })
    }

    #[must_use]
    pub fn is_list(&self) -> bool {
        matches!(self, Value::List { .. })
    }

    #[must_use]
    pub fn is_tuple(&self) -> bool {
        matches!(self, Value::Tuple { .. })
    }

    #[must_use]
    pub fn is_if(&self) -> bool {
        matches!(self, Value::If { .. })
    }

    #[must_use]
    pub fn is_while(&self) -> bool {
        matches!(self, Value::While { .. })
    }

    #[must_use]
    pub fn is_loop(&self) -> bool {
        matches!(self, Value::Loop { .. })
    }

    #[must_use]
    pub fn is_break(&self) -> bool {
        matches!(self, Value::Break { .. })
    }

    #[must_use]
    pub fn is_continue(&self) -> bool {
        matches!(self, Value::Continue { .. })
    }

    #[must_use]
    pub fn is_return(&self) -> bool {
        matches!(self, Value::Return { .. })
    }

    #[must_use]
    pub fn is_block(&self) -> bool {
        matches!(self, Value::Block { .. })
    }

    #[must_use]
    pub fn is_closure(&self) -> bool {
        matches!(self, Value::Closure { .. })
    }

    #[must_use]
    pub fn is_call(&self) -> bool {
        matches!(self, Value::Call { .. })
    }

    #[must_use]
    pub fn is_method_call(&self) -> bool {
        matches!(self, Value::MethodCall { .. })
    }
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
            Value::F32(f) => Ok(Lit::F32(f)),
            Value::F64(f) => Ok(Lit::F64(f)),
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
            Lit::F32(f) => Value::F32(f),
            Lit::F64(f) => Value::F64(f),
            Lit::USize32(u) => Value::USize32(u),
            Lit::USize64(u) => Value::USize64(u),
        }
    }
}

impl Value {
    #[must_use]
    pub fn is_literal(&self) -> bool {
        matches!(
            self,
            Value::Unit
                | Value::Bool(_)
                | Value::I8(_)
                | Value::I16(_)
                | Value::I32(_)
                | Value::I64(_)
                | Value::I128(_)
                | Value::U8(_)
                | Value::U16(_)
                | Value::U32(_)
                | Value::U64(_)
                | Value::U128(_)
                | Value::F32(_)
                | Value::F64(_)
                | Value::USize32(_)
                | Value::USize64(_)
                | Value::InferredInteger(_)
        )
    }
}

impl From<Lit> for LiteralId {
    fn from(lit: Lit) -> Self {
        get_storage(|store| store.store_literal(lit))
    }
}

impl From<Block> for BlockId {
    fn from(block: Block) -> Self {
        get_storage(|store| store.store_block(block))
    }
}

impl From<Value> for ValueId {
    fn from(value: Value) -> Self {
        get_storage(|store| store.store_value(value))
    }
}
