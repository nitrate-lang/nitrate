use crate::prelude::*;

impl Dump for Place {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Place::Symbol { symbol } => ctx.store[symbol].borrow().dump_nocycle(o),

            Place::FieldAccess { place, field } => {
                write!(o, "(")?;
                ctx.store[place].borrow().dump(ctx, o)?;
                write!(o, ".{})", field.0)
            }

            Place::ArrayIndex { place, index } => {
                write!(o, "(")?;
                ctx.store[place].borrow().dump(ctx, o)?;
                write!(o, "[")?;
                ctx.store[index].borrow().dump(ctx, o)?;
                write!(o, "])")
            }

            Place::Assign { place, value } => {
                write!(o, "(")?;
                ctx.store[place].borrow().dump(ctx, o)?;
                write!(o, " = ")?;
                ctx.store[value].borrow().dump(ctx, o)?;
                write!(o, ")")
            }

            Place::Deref { place } => {
                write!(o, "(*")?;
                ctx.store[place].borrow().dump(ctx, o)?;
                write!(o, ")")
            }
        }
    }
}
