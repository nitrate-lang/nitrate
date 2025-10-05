use serde::{Deserialize, Serialize};
use std::sync::Arc;

use crate::{
    ExprId, TypeId,
    dump::{Dump, DumpContext},
    store::BlockId,
};

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
pub enum BlockSafety {
    Safe,
    Unsafe,
}

#[derive(Serialize, Deserialize)]
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

impl Expr {}

impl Dump for Expr {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
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
                ctx.store[left].dump(ctx, o)?;
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
                ctx.store[right].dump(ctx, o)?;
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
                ctx.store[expr].dump(ctx, o)?;
                write!(o, ")")
            }

            Expr::Cast { expr, to } => {
                write!(o, "(")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, " as ")?;
                ctx.store[to].dump(ctx, o)?;
                write!(o, ")")
            }

            Expr::GetTypeOf { expr } => {
                write!(o, "(typeof ")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, ")")
            }

            Expr::List { elements } => {
                write!(o, "[")?;
                for (elem, i) in elements.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    ctx.store[elem].dump(ctx, o)?;
                }
                write!(o, "]")
            }

            Expr::If {
                condition,
                true_branch,
                false_branch,
            } => {
                write!(o, "if ")?;
                ctx.store[condition].dump(ctx, o)?;
                write!(o, " ")?;
                ctx.store[true_branch].dump(ctx, o)?;
                if let Some(false_branch) = false_branch {
                    write!(o, " else ")?;
                    ctx.store[false_branch].dump(ctx, o)?;
                }
                Ok(())
            }

            Expr::While { condition, body } => {
                write!(o, "while ")?;
                ctx.store[condition].dump(ctx, o)?;
                write!(o, " ")?;
                ctx.store[body].dump(ctx, o)
            }

            Expr::Loop { body } => {
                write!(o, "loop ")?;
                ctx.store[body].dump(ctx, o)
            }

            Expr::Break { label } => {
                write!(o, "break")?;
                if let Some(label) = label {
                    write!(o, " {}", label)?;
                }
                Ok(())
            }

            Expr::Continue { label } => {
                write!(o, "continue")?;
                if let Some(label) = label {
                    write!(o, " {}", label)?;
                }
                Ok(())
            }

            Expr::Return { value } => {
                write!(o, "return ")?;
                ctx.store[value].dump(ctx, o)
            }

            Expr::Call {
                function,
                arguments,
            } => {
                ctx.store[function].dump(ctx, o)?;
                write!(o, "(")?;
                for (arg, i) in arguments.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    ctx.store[arg].dump(ctx, o)?;
                }
                write!(o, ")")
            }
        }
    }
}
