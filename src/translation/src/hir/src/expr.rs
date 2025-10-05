use serde::{Deserialize, Serialize};
use std::sync::Arc;

use crate::{ExprId, Store, TypeId};

#[derive(Serialize, Deserialize)]
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

#[derive(Serialize, Deserialize)]
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

#[derive(Serialize, Deserialize)]
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

#[derive(Serialize, Deserialize)]
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
    // If {
    //     condition: ExprId,
    //     true_branch: BlockId,
    //     false_branch: Option<BlockId>,
    // },
}

impl Expr {}

impl Expr {
    pub fn dump(&self, store: &Store, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Expr::Literal(lit) => match lit {
                Literal::Unit => write!(o, "()"),
                Literal::Bool(b) => write!(o, "{}", b),
                Literal::I8(i) => write!(o, "i8 {}", i),
                Literal::I16(i) => write!(o, "i16 {}", i),
                Literal::I32(i) => write!(o, "i32 {}", i),
                Literal::I64(i) => write!(o, "i64 {}", i),
                Literal::I128(i) => write!(o, "i128 {}", i),
                Literal::F32(f) => write!(o, "f32 {}", f),
                Literal::F64(f) => write!(o, "f64 {}", f),
                Literal::String(s) => write!(o, "\"{}\"", s),
                Literal::BString(s) => write!(o, "b\"{:?}\"", s),
            },

            Expr::Binary { left, op, right } => {
                write!(o, "(")?;
                store[left].dump(store, o)?;
                write!(
                    o,
                    " {} ",
                    match op {
                        BinaryOp::Add => "+",
                        BinaryOp::Sub => "-",
                        BinaryOp::Mul => "*",
                        BinaryOp::Div => "/",
                        BinaryOp::Mod => "%",
                        BinaryOp::And => "&",
                        BinaryOp::Or => "|",
                        BinaryOp::Xor => "^",
                        BinaryOp::Shl => "<<",
                        BinaryOp::Shr => ">>",
                        BinaryOp::Rol => "<<<",
                        BinaryOp::Ror => ">>>",
                        BinaryOp::LogicAnd => "&&",
                        BinaryOp::LogicOr => "||",
                        BinaryOp::LogicXor => "^^",
                        BinaryOp::Lt => "<",
                        BinaryOp::Gt => ">",
                        BinaryOp::Lte => "<=",
                        BinaryOp::Gte => ">=",
                        BinaryOp::Eq => "==",
                        BinaryOp::Ne => "!=",
                        BinaryOp::Dot => ".",
                        BinaryOp::Arrow => "->",
                        BinaryOp::Range => "..",
                    }
                )?;
                store[right].dump(store, o)?;
                write!(o, ")")
            }

            Expr::Unary { op, expr } => {
                write!(
                    o,
                    "({} ",
                    match op {
                        UnaryOp::Add => "+",
                        UnaryOp::Sub => "-",
                        UnaryOp::Deref => "*",
                        UnaryOp::AddressOf => "&",
                        UnaryOp::BitNot => "~",
                        UnaryOp::LogicNot => "!",
                    }
                )?;
                store[expr].dump(store, o)?;
                write!(o, ")")
            }

            Expr::Cast { expr, to } => {
                write!(o, "(")?;
                store[expr].dump(store, o)?;
                write!(o, " as ")?;
                store[to].dump(store, o)?;
                write!(o, ")")
            }

            Expr::GetTypeOf { expr } => {
                write!(o, "(typeof ")?;
                store[expr].dump(store, o)?;
                write!(o, ")")
            }

            Expr::List { elements } => {
                write!(o, "[")?;
                for (elem, i) in elements.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    store[elem].dump(store, o)?;
                }
                write!(o, "]")
            }
        }
    }

    pub fn dump_string(&self, store: &Store) -> String {
        let mut buf = String::new();
        self.dump(store, &mut buf).ok();
        buf
    }
}
