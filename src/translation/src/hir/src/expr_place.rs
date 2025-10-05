use crate::prelude::*;
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub enum Place {
    FieldAccess { expr: PlaceId, field: IString },
    ArrayIndex { expr: PlaceId, index: ValueId },
    Deref { expr: PlaceId },
}

impl Dump for Place {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Place::FieldAccess { expr, field } => {
                write!(o, "(")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, ".{})", field)
            }

            Place::ArrayIndex { expr, index } => {
                write!(o, "(")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, "[")?;
                ctx.store[index].dump(ctx, o)?;
                write!(o, "])")
            }

            Place::Deref { expr } => {
                write!(o, "(deref ")?;
                ctx.store[expr].dump(ctx, o)?;
                write!(o, ")")
            }
        }
    }
}
