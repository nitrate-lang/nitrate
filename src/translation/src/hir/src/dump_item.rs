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

                    ctx.store[param].dump(ctx, o)?;
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

                    ctx.store[param].dump(ctx, o)?;
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

impl Symbol {
    pub fn dump_nocycle(&self, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::Unresolved { name } => write!(o, "sym nolink {}", name.0),
            Symbol::GlobalVariable { name, .. } => write!(o, "sym global {}", name.0),
            Symbol::LocalVariable { name, .. } => write!(o, "sym local {}", name.0),
            Symbol::Parameter { name, .. } => write!(o, "sym param {}", name.0),
            Symbol::Function(f) => match f {
                Function::External { name, .. } => write!(o, "sym fn {}", name.0),
                Function::Static { name, .. } => write!(o, "sym fn {}", name.0),
                Function::Closure {
                    closure_unique_id, ..
                } => {
                    write!(o, "sym fn #{}", closure_unique_id)
                }
            },
        }
    }
}

impl Dump for Symbol {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::Unresolved { name } => write!(o, "sym nolink {}", name.0),
            Symbol::GlobalVariable {
                visibility,
                name,
                ty,
                initializer,
            } => {
                match visibility {
                    Visibility::Sec => write!(o, "sec ")?,
                    Visibility::Pro => write!(o, "pro ")?,
                    Visibility::Pub => write!(o, "pub ")?,
                }
                write!(o, "sym global {}: ", name.0)?;
                ctx.store[ty].dump(ctx, o)?;
                write!(o, " = ")?;
                ctx.store[initializer].dump(ctx, o)
            }
            Symbol::LocalVariable {
                name,
                ty,
                initializer,
            } => {
                write!(o, "sym local {}: ", name.0)?;
                ctx.store[ty].dump(ctx, o)?;
                write!(o, " = ")?;
                ctx.store[initializer].dump(ctx, o)
            }
            Symbol::Parameter {
                name,
                ty,
                default_value,
            } => {
                write!(o, "sym param {}: ", name.0)?;
                ctx.store[ty].dump(ctx, o)?;
                if let Some(default_value) = default_value {
                    write!(o, " = ")?;
                    ctx.store[default_value].dump(ctx, o)?;
                }
                Ok(())
            }
            Symbol::Function(f) => f.dump(ctx, o),
        }
    }
}

impl Dump for ModuleAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}

impl Dump for Module {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self.visibility {
            Visibility::Sec => write!(o, "sec ")?,
            Visibility::Pro => write!(o, "pro ")?,
            Visibility::Pub => write!(o, "pub ")?,
        }

        write!(o, "mod {} ", self.name.0)?;
        if !self.attributes.is_empty() {
            write!(o, "[")?;
            for (attr, i) in self.attributes.iter().zip(0..) {
                if i != 0 {
                    write!(o, ", ")?;
                }
                attr.dump(ctx, o)?;
            }
            write!(o, "] ")?;
        }

        write!(o, "{{\n")?;
        for (item, i) in self.items.iter().zip(0..) {
            if i != 0 {
                write!(o, ",\n")?;
            }

            ctx.store[item].dump(ctx, o)?;
        }
        write!(o, "\n}}")?;
        Ok(())
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
