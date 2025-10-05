use crate::prelude::{hir::*, *};

impl Symbol {
    pub fn dump_nocycle(&self, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::GlobalVariable(v) => write!(o, "sym global {}", v.name.0),
            Symbol::LocalVariable(v) => write!(o, "sym local {}", v.name.0),
            Symbol::Parameter(p) => write!(o, "sym param {}", p.name.0),
            Symbol::Function(f) => match f {
                Function::External { name, .. } => write!(o, "sym fn {}", name.0),
                Function::Static { name, .. } => write!(o, "sym fn {}", name.0),
                Function::Closure {
                    closure_unique_id, ..
                } => {
                    write!(o, "sym fn #{}", closure_unique_id)
                }
            },
            Symbol::Unresolved { name } => write!(o, "sym nolink {}", name.0),
        }
    }
}

impl Dump for Place {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Place::Symbol { symbol } => ctx.store[symbol].dump_nocycle(o),

            Place::FieldAccess { place, field } => {
                write!(o, "(")?;
                ctx.store[place].dump(ctx, o)?;
                write!(o, ".{})", field.0)
            }

            Place::ArrayIndex { place, index } => {
                write!(o, "(")?;
                ctx.store[place].dump(ctx, o)?;
                write!(o, "[")?;
                ctx.store[index].dump(ctx, o)?;
                write!(o, "])")
            }

            Place::Assign { place, value } => {
                write!(o, "(")?;
                ctx.store[place].dump(ctx, o)?;
                write!(o, " = ")?;
                ctx.store[value].dump(ctx, o)?;
                write!(o, ")")
            }

            Place::Deref { place } => {
                write!(o, "(deref ")?;
                ctx.store[place].dump(ctx, o)?;
                write!(o, ")")
            }
        }
    }
}
