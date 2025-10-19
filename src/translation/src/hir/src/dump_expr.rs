use nitrate_token::{escape_bstring, escape_string};

use crate::{dump::write_indent, prelude::*};

impl Dump for BlockElement {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            BlockElement::Expr(expr_id) => ctx.store[expr_id].borrow().dump(ctx, o),

            BlockElement::Stmt(stmt_id) => {
                ctx.store[stmt_id].borrow().dump(ctx, o)?;
                write!(o, ";")
            }

            BlockElement::Local(local_id) => local_id.dump(ctx, o),
        }
    }
}

impl Dump for Block {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self.safety {
            BlockSafety::Safe => {}
            BlockSafety::Unsafe => write!(o, "unsafe ")?,
        }

        if self.elements.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;

            for expr in &self.elements {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                expr.dump(ctx, o)?;
                write!(o, "\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for Lit {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Lit::Unit => write!(o, "()"),
            Lit::Bool(b) => write!(o, "{}", b),
            Lit::I8(i) => write!(o, "i8 {}", i),
            Lit::I16(i) => write!(o, "i16 {}", i),
            Lit::I32(i) => write!(o, "i32 {}", i),
            Lit::I64(i) => write!(o, "i64 {}", i),
            Lit::I128(i) => write!(o, "i128 {}", i),
            Lit::U8(u) => write!(o, "u8 {}", u),
            Lit::U16(u) => write!(o, "u16 {}", u),
            Lit::U32(u) => write!(o, "u32 {}", u),
            Lit::U64(u) => write!(o, "u64 {}", u),
            Lit::U128(u) => write!(o, "u128 {}", u),
            Lit::F32(f) => write!(o, "f32 {}", f),
            Lit::F64(f) => write!(o, "f64 {}", f),
            Lit::USize32(u) => write!(o, "usize {}", u),
            Lit::USize64(u) => write!(o, "usize {}", u),
        }
    }
}

impl Dump for Value {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Value::Unit => write!(o, "()"),
            Value::Bool(b) => write!(o, "{}", b),
            Value::I8(i) => write!(o, "i8 {}", i),
            Value::I16(i) => write!(o, "i16 {}", i),
            Value::I32(i) => write!(o, "i32 {}", i),
            Value::I64(i) => write!(o, "i64 {}", i),
            Value::I128(i) => write!(o, "i128 {}", i),
            Value::U8(u) => write!(o, "u8 {}", u),
            Value::U16(u) => write!(o, "u16 {}", u),
            Value::U32(u) => write!(o, "u32 {}", u),
            Value::U64(u) => write!(o, "u64 {}", u),
            Value::U128(u) => write!(o, "u128 {}", u),
            Value::F32(f) => write!(o, "f32 {}", f),
            Value::F64(f) => write!(o, "f64 {}", f),
            Value::USize32(u) => write!(o, "usize {}", u),
            Value::USize64(u) => write!(o, "usize {}", u),
            Value::StringLit(s) => write!(o, "{}", escape_string(s, true)),
            Value::BStringLit(s) => write!(o, "{}", escape_bstring(s, true)),
            Value::InferredInteger(i) => write!(o, "?i {}", i),
            Value::InferredFloat(f) => write!(o, "?f {}", f),

            Value::StructObject {
                struct_type,
                fields,
            } => {
                ctx.store[struct_type].dump(ctx, o)?;

                write!(o, " {{ ")?;

                for (i, (field_name, field_value)) in fields.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{}: ", field_name)?;
                    ctx.store[field_value].borrow().dump(ctx, o)?;
                }

                write!(o, " }}")
            }

            Value::EnumVariant {
                enum_type,
                variant,
                value,
            } => {
                ctx.store[enum_type].dump(ctx, o)?;
                write!(o, "::{}", variant)?;
                write!(o, "(")?;
                ctx.store[value].borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Binary { left, op, right } => {
                write!(o, "(")?;
                ctx.store[left].borrow().dump(ctx, o)?;
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
                        BinaryOp::Lt => "<",
                        BinaryOp::Gt => ">",
                        BinaryOp::Lte => "<=",
                        BinaryOp::Gte => ">=",
                        BinaryOp::Eq => "==",
                        BinaryOp::Ne => "!=",
                    }
                )?;
                ctx.store[right].borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Unary { op, operand: expr } => {
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
                ctx.store[expr].borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Symbol(symbol) => match symbol.link.get() {
                Some(link) => write!(o, "`{}`::{}", symbol.path, link.as_usize()),
                None => write!(o, "? `{}`", symbol.path),
            },

            Value::FieldAccess { expr, field } => {
                write!(o, "(")?;
                ctx.store[expr].borrow().dump(ctx, o)?;
                write!(o, ".{})", field)
            }

            Value::IndexAccess {
                collection: expr,
                index,
            } => {
                write!(o, "(")?;
                ctx.store[expr].borrow().dump(ctx, o)?;
                write!(o, "[")?;
                ctx.store[index].borrow().dump(ctx, o)?;
                write!(o, "])")
            }

            Value::Assign { place, value } => {
                write!(o, "(")?;
                ctx.store[place].borrow().dump(ctx, o)?;
                write!(o, " = ")?;
                ctx.store[value].borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Deref { place } => {
                write!(o, "(*")?;
                ctx.store[place].borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Borrow { mutable, place } => {
                write!(o, "(&")?;
                if *mutable {
                    write!(o, "mut ")?;
                }
                ctx.store[place].borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Cast { expr, to } => {
                write!(o, "(")?;
                ctx.store[expr].borrow().dump(ctx, o)?;
                write!(o, " as ")?;
                ctx.store[to].dump(ctx, o)?;
                write!(o, ")")
            }

            Value::List { elements } => {
                write!(o, "[")?;
                for (i, elem) in elements.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    elem.dump(ctx, o)?;
                }
                write!(o, "]")
            }

            Value::Tuple { elements } => {
                write!(o, "(")?;
                for elem in &**elements {
                    elem.dump(ctx, o)?;
                    write!(o, ", ")?;
                }
                write!(o, ")")
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                write!(o, "if ")?;
                ctx.store[condition].borrow().dump(ctx, o)?;
                write!(o, " ")?;
                ctx.store[true_branch].borrow().dump(ctx, o)?;
                if let Some(false_branch) = false_branch {
                    write!(o, " else ")?;
                    ctx.store[false_branch].borrow().dump(ctx, o)?;
                }
                Ok(())
            }

            Value::While { condition, body } => {
                write!(o, "while ")?;
                ctx.store[condition].borrow().dump(ctx, o)?;
                write!(o, " ")?;
                ctx.store[body].borrow().dump(ctx, o)
            }

            Value::Loop { body } => {
                write!(o, "loop ")?;
                ctx.store[body].borrow().dump(ctx, o)
            }

            Value::Break { label } => {
                write!(o, "break")?;
                if let Some(label) = label {
                    write!(o, " {}", label)?;
                }
                Ok(())
            }

            Value::Continue { label } => {
                write!(o, "continue")?;
                if let Some(label) = label {
                    write!(o, " {}", label)?;
                }
                Ok(())
            }

            Value::Return { value } => {
                write!(o, "return ")?;
                ctx.store[value].borrow().dump(ctx, o)
            }

            Value::Block { block } => ctx.store[block].borrow().dump(ctx, o),

            Value::Closure { captures, callee } => {
                write!(o, "fn ")?;

                write!(o, " [")?;
                for (i, capture) in captures.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    capture.dump_nocycle(ctx, o)?;
                }
                write!(o, "] ")?;

                callee.dump(ctx, o)
            }

            Value::Call { callee, arguments } => {
                ctx.store[callee].borrow().dump(ctx, o)?;
                write!(o, "(")?;
                for (i, arg) in arguments.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    ctx.store[arg].borrow().dump(ctx, o)?;
                }
                write!(o, ")")
            }
        }
    }
}
