use interned_string::IString;
use serde::{Deserialize, Serialize};

use crate::{
    ExprId, TypeId,
    dump::{Dump, DumpContext},
    store::{BlockId, PlaceId},
};

#[derive(Debug, Serialize, Deserialize)]
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

#[derive(Debug, Serialize, Deserialize)]
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

#[derive(Debug, Serialize, Deserialize)]
pub enum Literal {
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
    String(Box<String>),
    BString(Box<Vec<u8>>),
}

#[derive(Debug, Serialize, Deserialize)]
pub enum BlockSafety {
    Safe,
    Unsafe,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Block {
    pub safety: BlockSafety,
    pub exprs: Vec<ExprId>,
}

impl Dump for Block {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self.safety {
            BlockSafety::Safe => write!(o, "{{\n")?,
            BlockSafety::Unsafe => write!(o, "unsafe {{\n")?,
        }
        for expr in &self.exprs {
            ctx.store[expr].dump(ctx, o)?;
            write!(o, ";\n")?;
        }
        write!(o, "}}")
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Expr {
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
    String(Box<String>),
    BString(Box<Vec<u8>>),

    Binary {
        left: ExprId,
        op: BinaryOp,
        right: ExprId,
    },

    Unary {
        op: UnaryOp,
        expr: ExprId,
    },

    FieldAccess {
        expr: ExprId,
        field: IString,
    },

    ArrayIndex {
        expr: ExprId,
        index: ExprId,
    },

    Cast {
        expr: ExprId,
        to: TypeId,
    },

    GetAddressOf {
        expr: PlaceId,
    },

    Deref {
        expr: PlaceId,
    },

    GetTypeOf {
        expr: ExprId,
    },

    List {
        elements: Box<Vec<ExprId>>,
    },

    If {
        condition: ExprId,
        true_branch: BlockId,
        false_branch: Option<BlockId>,
    },

    While {
        condition: ExprId,
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
        value: ExprId,
    },

    Call {
        function: ExprId,
        arguments: Box<Vec<ExprId>>,
    },
}

impl TryFrom<Expr> for Literal {
    type Error = Expr;

    fn try_from(value: Expr) -> Result<Self, Self::Error> {
        match value {
            Expr::Unit => Ok(Literal::Unit),
            Expr::Bool(b) => Ok(Literal::Bool(b)),
            Expr::I8(i) => Ok(Literal::I8(i)),
            Expr::I16(i) => Ok(Literal::I16(i)),
            Expr::I32(i) => Ok(Literal::I32(i)),
            Expr::I64(i) => Ok(Literal::I64(i)),
            Expr::I128(i) => Ok(Literal::I128(i)),
            Expr::U8(u) => Ok(Literal::U8(u)),
            Expr::U16(u) => Ok(Literal::U16(u)),
            Expr::U32(u) => Ok(Literal::U32(u)),
            Expr::U64(u) => Ok(Literal::U64(u)),
            Expr::U128(u) => Ok(Literal::U128(u)),
            Expr::F8(f) => Ok(Literal::F8(f)),
            Expr::F16(f) => Ok(Literal::F16(f)),
            Expr::F32(f) => Ok(Literal::F32(f)),
            Expr::F64(f) => Ok(Literal::F64(f)),
            Expr::F128(f) => Ok(Literal::F128(f)),
            Expr::String(s) => Ok(Literal::String(s)),
            Expr::BString(b) => Ok(Literal::BString(b)),
            other => Err(other),
        }
    }
}

impl From<Literal> for Expr {
    fn from(value: Literal) -> Self {
        match value {
            Literal::Unit => Expr::Unit,
            Literal::Bool(b) => Expr::Bool(b),
            Literal::I8(i) => Expr::I8(i),
            Literal::I16(i) => Expr::I16(i),
            Literal::I32(i) => Expr::I32(i),
            Literal::I64(i) => Expr::I64(i),
            Literal::I128(i) => Expr::I128(i),
            Literal::U8(u) => Expr::U8(u),
            Literal::U16(u) => Expr::U16(u),
            Literal::U32(u) => Expr::U32(u),
            Literal::U64(u) => Expr::U64(u),
            Literal::U128(u) => Expr::U128(u),
            Literal::F8(f) => Expr::F8(f),
            Literal::F16(f) => Expr::F16(f),
            Literal::F32(f) => Expr::F32(f),
            Literal::F64(f) => Expr::F64(f),
            Literal::F128(f) => Expr::F128(f),
            Literal::String(s) => Expr::String(s),
            Literal::BString(b) => Expr::BString(b),
        }
    }
}
