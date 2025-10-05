use crate::prelude::*;
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub enum Place {
    FieldAccess { place: PlaceId, field: IString },
    ArrayIndex { place: PlaceId, index: ValueId },
    Assign { place: PlaceId, value: ValueId },
    Deref { place: PlaceId },
}

impl Dump for Place {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
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

    fn dump_trunk(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Place::FieldAccess { place, field } => {
                write!(o, "(")?;
                ctx.store[place].dump_trunk(ctx, o)?;
                write!(o, ".{})", field)
            }

            Place::ArrayIndex { place, index } => {
                write!(o, "(")?;
                ctx.store[place].dump_trunk(ctx, o)?;
                write!(o, "[")?;
                ctx.store[index].dump_trunk(ctx, o)?;
                write!(o, "])")
            }

            Place::Assign { place, value } => {
                write!(o, "(")?;
                ctx.store[place].dump_trunk(ctx, o)?;
                write!(o, " = ")?;
                ctx.store[value].dump_trunk(ctx, o)?;
                write!(o, ")")
            }

            Place::Deref { place } => {
                write!(o, "(deref ")?;
                ctx.store[place].dump_trunk(ctx, o)?;
                write!(o, ")")
            }
        }
    }
}
