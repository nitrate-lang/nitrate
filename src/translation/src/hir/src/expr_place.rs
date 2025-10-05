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

impl SaveToStorage for Place {
    type Id = PlaceId;

    fn save_to_storage(self, ctx: &mut Store) -> Self::Id {
        ctx.store_place(self)
    }
}
