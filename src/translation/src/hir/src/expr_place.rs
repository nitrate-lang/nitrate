use crate::prelude::{hir::*, *};
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub enum Place {
    Symbol { symbol: SymbolId },
    FieldAccess { place: PlaceId, field: EntityName },
    ArrayIndex { place: PlaceId, index: ValueId },
    Assign { place: PlaceId, value: ValueId },
    Deref { place: PlaceId },
}
