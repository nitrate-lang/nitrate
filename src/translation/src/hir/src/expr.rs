use std::matches;

use crate::{prelude::*, store::LiteralId};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
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

impl BinaryOp {
    #[must_use]
    pub fn is_comparison(&self) -> bool {
        matches!(
            self,
            BinaryOp::Lt | BinaryOp::Gt | BinaryOp::Lte | BinaryOp::Gte | BinaryOp::Eq | BinaryOp::Ne
        )
    }

    #[must_use]
    pub fn is_equality(&self) -> bool {
        matches!(self, BinaryOp::Eq | BinaryOp::Ne)
    }

    #[must_use]
    pub fn is_logical(&self) -> bool {
        matches!(self, BinaryOp::LogicAnd | BinaryOp::LogicOr)
    }
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

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, PartialOrd)]
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
    USize(u8, u64),
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
            Lit::USize(bits, _) => usize::from(*bits / 8),
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
            Lit::Unit => None,
            Lit::Bool(_) => None,
            Lit::I8(_) => value.try_into().ok().map(Lit::I8),
            Lit::I16(_) => value.try_into().ok().map(Lit::I16),
            Lit::I32(_) => value.try_into().ok().map(Lit::I32),
            Lit::I64(_) => value.try_into().ok().map(Lit::I64),
            Lit::I128(_) => value.try_into().ok().map(Lit::I128),
            Lit::U8(_) => value.try_into().ok().map(Lit::U8),
            Lit::U16(_) => value.try_into().ok().map(Lit::U16),
            Lit::U32(_) => value.try_into().ok().map(Lit::U32),
            Lit::U64(_) => value.try_into().ok().map(Lit::U64),
            Lit::U128(_) => value.try_into().ok().map(Lit::U128),
            Lit::F32(_) | Lit::F64(_) => None,
            Lit::USize(bits, _) => value.try_into().ok().map(|v| Lit::USize(*bits, v)),
        }
    }
}

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
            Lit::USize(bits, u) => {
                17u8.hash(state);
                bits.hash(state);
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
            Lit::USize(bits, u) => write!(f, "{u}_usize{}", *bits),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
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
    pub span: ByteSpan,
    pub safety: BlockSafety,
    pub elements: Vec<BlockElement>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct Arguments<T> {
    pub positional: ThinVec<T>,
    pub named: ThinVec<(NString, T)>,
}

pub struct ArgumentsIterator<T> {
    positional: ThinVec<T>,
    named: ThinVec<(NString, T)>,
}

impl<T> Arguments<T> {
    #[allow(clippy::should_implement_trait)]
    pub fn into_iter(self) -> ArgumentsIterator<T> {
        ArgumentsIterator {
            positional: self.positional,
            named: self.named,
        }
    }
}

impl<T> Iterator for ArgumentsIterator<T> {
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        if !self.positional.is_empty() {
            Some(self.positional.remove(0))
        } else if !self.named.is_empty() {
            Some(self.named.remove(0).1)
        } else {
            None
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum Value {
    Unit {
        span: ByteSpan,
    },
    Bool {
        span: ByteSpan,
        value: bool,
    },
    I8 {
        span: ByteSpan,
        value: i8,
    },
    I16 {
        span: ByteSpan,
        value: i16,
    },
    I32 {
        span: ByteSpan,
        value: i32,
    },
    I64 {
        span: ByteSpan,
        value: i64,
    },
    I128 {
        span: ByteSpan,
        value: Box<i128>,
    },
    U8 {
        span: ByteSpan,
        value: u8,
    },
    U16 {
        span: ByteSpan,
        value: u16,
    },
    U32 {
        span: ByteSpan,
        value: u32,
    },
    U64 {
        span: ByteSpan,
        value: u64,
    },
    U128 {
        span: ByteSpan,
        value: Box<u128>,
    },
    F32 {
        span: ByteSpan,
        value: OrderedFloat<f32>,
    },
    F64 {
        span: ByteSpan,
        value: OrderedFloat<f64>,
    },
    USize {
        span: ByteSpan,
        bits: u8,
        value: u64,
    },
    StringLit {
        span: ByteSpan,
        value: ThinStr,
    },
    BStringLit {
        span: ByteSpan,
        value: ThinVec<u8>,
    },
    InferredInteger {
        span: ByteSpan,
        value: Box<u128>,
    },
    InferredFloat {
        span: ByteSpan,
        value: OrderedFloat<f64>,
    },

    StructObject {
        span: ByteSpan,
        struct_def: StructDefId,
        fields: ThinVec<(NString, ValueId)>,
    },

    EnumVariant {
        span: ByteSpan,
        enum_def: EnumDefId,
        variant: NString,
        value: ValueId,
    },

    Binary {
        span: ByteSpan,
        left: ValueId,
        op: BinaryOp,
        right: ValueId,
    },

    Unary {
        span: ByteSpan,
        op: UnaryOp,
        operand: ValueId,
    },

    IndexAccess {
        span: ByteSpan,
        collection: ValueId,
        index: ValueId,
    },

    FieldAccess {
        span: ByteSpan,
        expr: ValueId,
        field_name: NString,
    },

    Assign {
        span: ByteSpan,
        place: ValueId,
        value: ValueId,
    },

    Deref {
        span: ByteSpan,
        place: ValueId,
    },

    Cast {
        span: ByteSpan,
        value: ValueId,
        target_type: TypeId,
    },

    Borrow {
        span: ByteSpan,
        exclusive: bool,
        mutable: bool,
        place: ValueId,
    },

    List {
        span: ByteSpan,
        elements: ThinVec<ValueId>,
    },

    Tuple {
        span: ByteSpan,
        elements: ThinVec<ValueId>,
    },

    If {
        span: ByteSpan,
        condition: ValueId,
        true_branch: BlockId,
        false_branch: Option<BlockId>,
    },

    While {
        span: ByteSpan,
        condition: ValueId,
        body: BlockId,
    },

    Loop {
        span: ByteSpan,
        body: BlockId,
    },

    Break {
        span: ByteSpan,
        label: Option<NString>,
    },

    Continue {
        span: ByteSpan,
        label: Option<NString>,
    },

    Return {
        span: ByteSpan,
        value: ValueId,
    },

    Block {
        span: ByteSpan,
        block: BlockId,
    },

    Call {
        span: ByteSpan,
        callee: ValueId,
        args: Arguments<ValueId>,
    },

    MethodCall {
        span: ByteSpan,
        object: ValueId,
        method_name: NString,
        args: Arguments<ValueId>,
    },

    FunctionSymbol {
        span: ByteSpan,
        id: FunctionId,
    },

    GlobalVariableSymbol {
        span: ByteSpan,
        id: GlobalVariableId,
    },

    LocalVariableSymbol {
        span: ByteSpan,
        id: LocalVariableId,
    },

    ParameterSymbol {
        span: ByteSpan,
        id: ParameterId,
    },
}

impl Value {
    #[must_use]
    pub fn span(&self) -> ByteSpan {
        match self {
            Value::Unit { span } => *span,
            Value::Bool { span, .. } => *span,
            Value::I8 { span, .. } => *span,
            Value::I16 { span, .. } => *span,
            Value::I32 { span, .. } => *span,
            Value::I64 { span, .. } => *span,
            Value::I128 { span, .. } => *span,
            Value::U8 { span, .. } => *span,
            Value::U16 { span, .. } => *span,
            Value::U32 { span, .. } => *span,
            Value::U64 { span, .. } => *span,
            Value::U128 { span, .. } => *span,
            Value::F32 { span, .. } => *span,
            Value::F64 { span, .. } => *span,
            Value::USize { span, .. } => *span,
            Value::StringLit { span, .. } => *span,
            Value::BStringLit { span, .. } => *span,
            Value::InferredInteger { span, .. } => *span,
            Value::InferredFloat { span, .. } => *span,
            Value::StructObject { span, .. } => *span,
            Value::EnumVariant { span, .. } => *span,
            Value::Binary { span, .. } => *span,
            Value::Unary { span, .. } => *span,
            Value::IndexAccess { span, .. } => *span,
            Value::FieldAccess { span, .. } => *span,
            Value::Assign { span, .. } => *span,
            Value::Deref { span, .. } => *span,
            Value::Cast { span, .. } => *span,
            Value::Borrow { span, .. } => *span,
            Value::List { span, .. } => *span,
            Value::Tuple { span, .. } => *span,
            Value::If { span, .. } => *span,
            Value::While { span, .. } => *span,
            Value::Loop { span, .. } => *span,
            Value::Break { span, .. } => *span,
            Value::Continue { span, .. } => *span,
            Value::Return { span, .. } => *span,
            Value::Block { span, .. } => *span,
            Value::Call { span, .. } => *span,
            Value::MethodCall { span, .. } => *span,
            Value::FunctionSymbol { span, .. } => *span,
            Value::GlobalVariableSymbol { span, .. } => *span,
            Value::LocalVariableSymbol { span, .. } => *span,
            Value::ParameterSymbol { span, .. } => *span,
        }
    }

    #[must_use]
    pub fn is_unit(&self) -> bool {
        matches!(self, Value::Unit { .. })
    }

    #[must_use]
    pub fn is_bool(&self) -> bool {
        matches!(self, Value::Bool { .. })
    }

    #[must_use]
    pub fn is_i8(&self) -> bool {
        matches!(self, Value::I8 { .. })
    }

    #[must_use]
    pub fn is_i16(&self) -> bool {
        matches!(self, Value::I16 { .. })
    }

    #[must_use]
    pub fn is_i32(&self) -> bool {
        matches!(self, Value::I32 { .. })
    }

    #[must_use]
    pub fn is_i64(&self) -> bool {
        matches!(self, Value::I64 { .. })
    }

    #[must_use]
    pub fn is_i128(&self) -> bool {
        matches!(self, Value::I128 { .. })
    }

    #[must_use]
    pub fn is_u8(&self) -> bool {
        matches!(self, Value::U8 { .. })
    }

    #[must_use]
    pub fn is_u16(&self) -> bool {
        matches!(self, Value::U16 { .. })
    }

    #[must_use]
    pub fn is_u32(&self) -> bool {
        matches!(self, Value::U32 { .. })
    }

    #[must_use]
    pub fn is_u64(&self) -> bool {
        matches!(self, Value::U64 { .. })
    }

    #[must_use]
    pub fn is_u128(&self) -> bool {
        matches!(self, Value::U128 { .. })
    }

    #[must_use]
    pub fn is_f32(&self) -> bool {
        matches!(self, Value::F32 { .. })
    }

    #[must_use]
    pub fn is_f64(&self) -> bool {
        matches!(self, Value::F64 { .. })
    }

    #[must_use]
    pub fn is_usize(&self) -> bool {
        matches!(self, Value::USize { .. })
    }

    #[must_use]
    pub fn is_string_lit(&self) -> bool {
        matches!(self, Value::StringLit { .. })
    }

    #[must_use]
    pub fn is_bstring_lit(&self) -> bool {
        matches!(self, Value::BStringLit { .. })
    }

    #[must_use]
    pub fn is_inferred_integer(&self) -> bool {
        matches!(self, Value::InferredInteger { .. })
    }

    #[must_use]
    pub fn is_inferred_float(&self) -> bool {
        matches!(self, Value::InferredFloat { .. })
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
    pub fn is_index_access(&self) -> bool {
        matches!(self, Value::IndexAccess { .. })
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
    pub fn is_call(&self) -> bool {
        matches!(self, Value::Call { .. })
    }

    #[must_use]
    pub fn is_method_call(&self) -> bool {
        matches!(self, Value::MethodCall { .. })
    }

    #[must_use]
    pub fn is_function_symbol(&self) -> bool {
        matches!(self, Value::FunctionSymbol { .. })
    }

    #[must_use]
    pub fn is_global_variable_symbol(&self) -> bool {
        matches!(self, Value::GlobalVariableSymbol { .. })
    }

    #[must_use]
    pub fn is_local_variable_symbol(&self) -> bool {
        matches!(self, Value::LocalVariableSymbol { .. })
    }

    #[must_use]
    pub fn is_parameter_symbol(&self) -> bool {
        matches!(self, Value::ParameterSymbol { .. })
    }
}

impl TryFrom<Value> for Lit {
    type Error = Value;

    fn try_from(value: Value) -> Result<Self, Self::Error> {
        match value {
            Value::Unit { .. } => Ok(Lit::Unit),
            Value::Bool { value, .. } => Ok(Lit::Bool(value)),
            Value::I8 { value, .. } => Ok(Lit::I8(value)),
            Value::I16 { value, .. } => Ok(Lit::I16(value)),
            Value::I32 { value, .. } => Ok(Lit::I32(value)),
            Value::I64 { value, .. } => Ok(Lit::I64(value)),
            Value::I128 { value, .. } => Ok(Lit::I128(*value)),
            Value::U8 { value, .. } => Ok(Lit::U8(value)),
            Value::U16 { value, .. } => Ok(Lit::U16(value)),
            Value::U32 { value, .. } => Ok(Lit::U32(value)),
            Value::U64 { value, .. } => Ok(Lit::U64(value)),
            Value::U128 { value, .. } => Ok(Lit::U128(*value)),
            Value::F32 { value, .. } => Ok(Lit::F32(value)),
            Value::F64 { value, .. } => Ok(Lit::F64(value)),
            Value::USize { bits, value, .. } => Ok(Lit::USize(bits, value)),
            other => Err(other),
        }
    }
}

impl From<Lit> for Value {
    fn from(value: Lit) -> Self {
        match value {
            Lit::Unit => Value::Unit {
                span: ByteSpan::default(),
            },
            Lit::Bool(b) => Value::Bool {
                span: ByteSpan::default(),
                value: b,
            },
            Lit::I8(i) => Value::I8 {
                span: ByteSpan::default(),
                value: i,
            },
            Lit::I16(i) => Value::I16 {
                span: ByteSpan::default(),
                value: i,
            },
            Lit::I32(i) => Value::I32 {
                span: ByteSpan::default(),
                value: i,
            },
            Lit::I64(i) => Value::I64 {
                span: ByteSpan::default(),
                value: i,
            },
            Lit::I128(i) => Value::I128 {
                span: ByteSpan::default(),
                value: Box::new(i),
            },
            Lit::U8(u) => Value::U8 {
                span: ByteSpan::default(),
                value: u,
            },
            Lit::U16(u) => Value::U16 {
                span: ByteSpan::default(),
                value: u,
            },
            Lit::U32(u) => Value::U32 {
                span: ByteSpan::default(),
                value: u,
            },
            Lit::U64(u) => Value::U64 {
                span: ByteSpan::default(),
                value: u,
            },
            Lit::U128(u) => Value::U128 {
                span: ByteSpan::default(),
                value: Box::new(u),
            },
            Lit::F32(f) => Value::F32 {
                span: ByteSpan::default(),
                value: f,
            },
            Lit::F64(f) => Value::F64 {
                span: ByteSpan::default(),
                value: f,
            },
            Lit::USize(bits, u) => Value::USize {
                span: ByteSpan::default(),
                bits,
                value: u,
            },
        }
    }
}

impl Value {
    #[must_use]
    pub fn is_literal(&self) -> bool {
        matches!(
            self,
            Value::Unit { .. }
                | Value::Bool { .. }
                | Value::I8 { .. }
                | Value::I16 { .. }
                | Value::I32 { .. }
                | Value::I64 { .. }
                | Value::I128 { .. }
                | Value::U8 { .. }
                | Value::U16 { .. }
                | Value::U32 { .. }
                | Value::U64 { .. }
                | Value::U128 { .. }
                | Value::F32 { .. }
                | Value::F64 { .. }
                | Value::USize { .. }
                | Value::InferredInteger { .. }
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
