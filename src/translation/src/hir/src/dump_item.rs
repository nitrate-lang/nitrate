use crate::prelude::{hir::*, *};

impl Dump for Function {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Function::External {
                visibility,
                attributes,
                name,
                parameters,
                return_type,
            } => {
                match visibility {
                    Visibility::Sec => write!(o, "sec ")?,
                    Visibility::Pro => write!(o, "pro ")?,
                    Visibility::Pub => write!(o, "pub ")?,
                }

                write!(o, "sym extern fn ")?;
                if !attributes.is_empty() {
                    write!(o, "[")?;
                    for (attr, i) in attributes.iter().zip(0..) {
                        if i != 0 {
                            write!(o, ", ")?;
                        }
                        attr.dump(ctx, o)?;
                    }
                    write!(o, "] ")?;
                }
                write!(o, "{}(", name.0)?;
                for (param, i) in parameters.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    write!(o, "{}: ", param.name.0)?;
                    ctx.store[&param.ty].dump(ctx, o)?;
                    if let Some(default_value) = &param.default_value {
                        write!(o, " = ")?;
                        ctx.store[default_value].dump(ctx, o)?;
                    }
                }
                write!(o, ") -> ")?;
                ctx.store[return_type].dump(ctx, o)
            }

            Function::Static {
                visibility,
                attributes,
                name,
                parameters,
                return_type,
                body,
            } => {
                match visibility {
                    Visibility::Sec => write!(o, "sec ")?,
                    Visibility::Pro => write!(o, "pro ")?,
                    Visibility::Pub => write!(o, "pub ")?,
                }

                write!(o, "sym static fn ")?;
                if !attributes.is_empty() {
                    write!(o, "[")?;
                    for (attr, i) in attributes.iter().zip(0..) {
                        if i != 0 {
                            write!(o, ", ")?;
                        }
                        attr.dump(ctx, o)?;
                    }
                    write!(o, "] ")?;
                }
                write!(o, "{}(", name.0)?;
                for (param, i) in parameters.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{}: ", param.name.0)?;
                    ctx.store[&param.ty].dump(ctx, o)?;
                    if let Some(default_value) = &param.default_value {
                        write!(o, " = ")?;
                        ctx.store[default_value].dump(ctx, o)?;
                    }
                }
                write!(o, ") -> ")?;
                ctx.store[return_type].dump(ctx, o)?;
                write!(o, " ")?;
                ctx.store[body].dump(ctx, o)
            }

            Function::Closure {
                closure_unique_id,
                callee,
                captures,
            } => {
                write!(o, "sym fn #{}", closure_unique_id)?;
                write!(o, " [")?;
                for (capture, i) in captures.iter().zip(0..) {
                    if i != 0 {
                        write!(o, ", ")?;
                    }
                    ctx.store[capture].dump(ctx, o)?;
                }
                write!(o, "] ")?;
                callee.dump(ctx, o)
            }
        }
    }
}

impl Dump for Item {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}
