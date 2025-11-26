use crate::{Dump, DumpContext, write_indent};
use nitrate_hir::prelude::*;
use nitrate_token::{escape_bstring, escape_string};

impl Dump for BlockElement {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            BlockElement::Expr(expr_id) => {
                expr_id.borrow().dump(ctx, o)?;
                write!(o, ";")
            }

            BlockElement::Local(local_id) => local_id.borrow().dump(ctx, o),
        }
    }
}

impl Dump for Block {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self.safety {
            BlockSafety::Safe => {}
            BlockSafety::Unsafe => write!(o, "unsafe ")?,
        }

        if self.elements.is_empty() {
            write!(o, "{{}}")
        } else {
            writeln!(o, "{{")?;

            for expr in &self.elements {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                expr.dump(ctx, o)?;
                writeln!(o)?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for Lit {
    fn dump(&self, _ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Lit::Unit => write!(o, "()"),
            Lit::Bool(b) => write!(o, "{b}"),
            Lit::I8(i) => write!(o, "i8 {i}"),
            Lit::I16(i) => write!(o, "i16 {i}"),
            Lit::I32(i) => write!(o, "i32 {i}"),
            Lit::I64(i) => write!(o, "i64 {i}"),
            Lit::I128(i) => write!(o, "i128 {i}"),
            Lit::U8(u) => write!(o, "u8 {u}"),
            Lit::U16(u) => write!(o, "u16 {u}"),
            Lit::U32(u) => write!(o, "u32 {u}"),
            Lit::U64(u) => write!(o, "u64 {u}"),
            Lit::U128(u) => write!(o, "u128 {u}"),
            Lit::F32(f) => write!(o, "f32 {f}"),
            Lit::F64(f) => write!(o, "f64 {f}"),
            Lit::USize32(u) => write!(o, "usize {u}"),
            Lit::USize64(u) => write!(o, "usize {u}"),
        }
    }
}

impl Dump for Value {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Value::Unit => write!(o, "()"),
            Value::Bool(b) => write!(o, "{b}"),
            Value::I8(i) => write!(o, "i8 {i}"),
            Value::I16(i) => write!(o, "i16 {i}"),
            Value::I32(i) => write!(o, "i32 {i}"),
            Value::I64(i) => write!(o, "i64 {i}"),
            Value::I128(i) => write!(o, "i128 {i}"),
            Value::U8(u) => write!(o, "u8 {u}"),
            Value::U16(u) => write!(o, "u16 {u}"),
            Value::U32(u) => write!(o, "u32 {u}"),
            Value::U64(u) => write!(o, "u64 {u}"),
            Value::U128(u) => write!(o, "u128 {u}"),
            Value::F32(f) => write!(o, "f32 {f}"),
            Value::F64(f) => write!(o, "f64 {f}"),
            Value::USize32(u) => write!(o, "usize {u}"),
            Value::USize64(u) => write!(o, "usize {u}"),
            Value::StringLit(s) => write!(o, "{}", escape_string(s, true)),
            Value::BStringLit(s) => write!(o, "{}", escape_bstring(s, true)),
            Value::InferredInteger(i) => write!(o, "?i {i}"),
            Value::InferredFloat(f) => write!(o, "?f {f}"),

            Value::StructObject { struct_def, fields } => {
                write!(o, "{}", struct_def.borrow().name)?;

                write!(o, " {{ ")?;

                for (i, (field_name, field_value)) in fields.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{field_name}: ")?;
                    field_value.borrow().dump(ctx, o)?;
                }

                write!(o, " }}")
            }

            Value::EnumVariant {
                enum_def,
                variant,
                value,
            } => {
                write!(o, "{}", enum_def.borrow().name)?;
                write!(o, "::{variant}")?;
                write!(o, "(")?;
                value.borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Binary { left, op, right } => {
                write!(o, "(")?;
                left.borrow().dump(ctx, o)?;
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
                right.borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Unary { op, operand: expr } => {
                write!(
                    o,
                    "({} ",
                    match op {
                        UnaryOp::Add => "+",
                        UnaryOp::Sub => "-",
                        UnaryOp::Not => "!",
                    }
                )?;
                expr.borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::FieldAccess {
                expr,
                field_name: field,
            } => {
                write!(o, "(")?;
                expr.borrow().dump(ctx, o)?;
                write!(o, ".{field})")
            }

            Value::Assign { place, value } => {
                write!(o, "(")?;
                place.borrow().dump(ctx, o)?;
                write!(o, " = ")?;
                value.borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Deref { place } => {
                write!(o, "(*")?;
                place.borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Borrow {
                exclusive,
                mutable,
                place,
            } => {
                write!(o, "(&")?;
                match (exclusive, mutable) {
                    (true, true) => write!(o, "mut ")?,
                    (true, false) => write!(o, "iso ")?,
                    (false, true) => write!(o, "poly mut ")?,
                    (false, false) => write!(o, "")?,
                }
                place.borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Value::Cast {
                value: expr,
                target_type: to,
            } => {
                write!(o, "(")?;
                expr.borrow().dump(ctx, o)?;
                write!(o, " as ")?;
                to.dump(ctx, o)?;
                write!(o, ")")
            }

            Value::List { elements } => {
                write!(o, "[")?;
                for (i, elem) in elements.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    elem.borrow().dump(ctx, o)?;
                }
                write!(o, "]")
            }

            Value::Tuple { elements } => {
                write!(o, "(")?;
                for elem in &**elements {
                    elem.borrow().dump(ctx, o)?;
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
                condition.borrow().dump(ctx, o)?;
                write!(o, " ")?;
                true_branch.borrow().dump(ctx, o)?;
                if let Some(false_branch) = false_branch {
                    write!(o, " else ")?;
                    false_branch.borrow().dump(ctx, o)?;
                }
                Ok(())
            }

            Value::While { condition, body } => {
                write!(o, "while ")?;
                condition.borrow().dump(ctx, o)?;
                write!(o, " ")?;
                body.borrow().dump(ctx, o)
            }

            Value::Loop { body } => {
                write!(o, "loop ")?;
                body.borrow().dump(ctx, o)
            }

            Value::Break { label } => {
                write!(o, "break")?;
                if let Some(label) = label {
                    write!(o, " {label}")?;
                }
                Ok(())
            }

            Value::Continue { label } => {
                write!(o, "continue")?;
                if let Some(label) = label {
                    write!(o, " {label}")?;
                }
                Ok(())
            }

            Value::Return { value } => {
                write!(o, "return ")?;
                value.borrow().dump(ctx, o)
            }

            Value::Block { block } => block.borrow().dump(ctx, o),

            Value::Call {
                callee,
                positional,
                named,
            } => {
                callee.borrow().dump(ctx, o)?;
                write!(o, "(")?;
                for (i, arg) in positional.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    arg.borrow().dump(ctx, o)?;
                }
                for (i, (name, arg)) in named.iter().enumerate() {
                    if !named.is_empty() || i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{name}: ")?;
                    arg.borrow().dump(ctx, o)?;
                }
                write!(o, ")")
            }

            Value::MethodCall {
                object,
                method_name: method,
                positional,
                named,
            } => {
                object.borrow().dump(ctx, o)?;
                write!(o, ".{method}(")?;
                for (i, arg) in positional.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    arg.borrow().dump(ctx, o)?;
                }
                for (i, (name, arg)) in named.iter().enumerate() {
                    if !named.is_empty() || i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{name}: ")?;
                    arg.borrow().dump(ctx, o)?;
                }
                write!(o, ")")
            }

            Value::FunctionSymbol { id } => {
                let func = id.borrow();
                write!(o, "fn {}", func.name)
            }

            Value::GlobalVariableSymbol { id } => {
                let global = id.borrow();
                write!(o, "global {}", global.name)
            }

            Value::LocalVariableSymbol { id } => {
                let local = id.borrow();
                write!(o, "local {}", local.name)
            }

            Value::ParameterSymbol { id } => {
                let param = id.borrow();
                write!(o, "param {}", param.name)
            }
        }
    }
}
