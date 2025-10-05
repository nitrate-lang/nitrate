use serde::{Deserialize, Serialize};
use std::sync::Arc;

use crate::{
    ExprId, TypeId,
    dump::{Dump, DumpContext},
    store::BlockId,
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
    /// `.`
    Dot,
    /// `->`
    Arrow,
    /// `..`
    Range,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum UnaryOp {
    /// `+`
    Add,
    /// `-`
    Sub,
    /// `*`
    Deref,
    /// `&`
    AddressOf,
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
    I128(Arc<i128>),
    F32(f32),
    F64(f64),
    String(Arc<String>),
    BString(Arc<Vec<u8>>),
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
    Literal(Literal),

    Binary {
        left: ExprId,
        op: BinaryOp,
        right: ExprId,
    },

    Unary {
        op: UnaryOp,
        expr: ExprId,
    },

    Cast {
        expr: ExprId,
        to: TypeId,
    },

    GetTypeOf {
        expr: ExprId,
    },

    List {
        elements: Vec<ExprId>,
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
        label: Option<Arc<String>>,
    },

    Continue {
        label: Option<Arc<String>>,
    },

    Return {
        value: ExprId,
    },

    Call {
        function: ExprId,
        arguments: Vec<ExprId>,
    },
}
