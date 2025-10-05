use crate::{
    Dump, DumpContext,
    hir::{BinaryOp, Expr, UnaryOp},
};

impl Dump for Expr {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Expr::Unit => write!(o, "()"),
            Expr::Bool(b) => write!(o, "{}", b),
            Expr::I8(i) => write!(o, "i8 {}", i),
            Expr::I16(i) => write!(o, "i16 {}", i),
            Expr::I32(i) => write!(o, "i32 {}", i),
            Expr::I64(i) => write!(o, "i64 {}", i),
            Expr::I128(i) => write!(o, "i128 {}", i),
            Expr::U8(u) => write!(o, "u8 {}", u),
            Expr::U16(u) => write!(o, "u16 {}", u),
            Expr::U32(u) => write!(o, "u32 {}", u),
            Expr::U64(u) => write!(o, "u64 {}", u),
            Expr::U128(u) => write!(o, "u128 {}", u),
            Expr::F8(f) => write!(o, "f8 {}", f),
            Expr::F16(f) => write!(o, "f16 {}", f),
            Expr::F32(f) => write!(o, "f32 {}", f),
            Expr::F64(f) => write!(o, "f64 {}", f),
            Expr::F128(f) => write!(o, "f128 {}", f),
            Expr::String(s) => write!(o, "\"{}\"", s),
            Expr::BString(s) => write!(o, "b\"{:?}\"", s),

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
                        UnaryOp::BitNot => "~",
                        UnaryOp::LogicNot => "!",
                    }
                )?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, ")")
            }

            Expr::FieldAccess { expr, field } => {
                write!(o, "(")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, ".{})", field)
            }

            Expr::ArrayIndex { expr, index } => {
                write!(o, "(")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, "[")?;
                ctx.store[index].dump(ctx, o)?;
                write!(o, "])")
            }

            Expr::GetAddressOf { expr } => {
                write!(o, "(address_of ")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, ")")
            }

            Expr::Deref { expr } => {
                write!(o, "(deref ")?;
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
