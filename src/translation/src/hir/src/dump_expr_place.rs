use crate::{Dump, DumpContext, expr_place::Place};

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
                write!(o, ".{})", field)
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
