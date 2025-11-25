use crate::prelude::*;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use thin_vec::ThinVec;

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Item {
    Root {
        items: ThinVec<ItemId>,
    },

    Module {
        /* $ mod $ <attributes>? $ <name> $ {}*/
        source_offset: u32,
        trivia: [Option<Trivia>; 4],
        attributes: ThinVec<ExprId>,
        name: NString,
        items: ThinVec<ItemId>,
    },
}

impl From<Item> for ItemId {
    fn from(item: Item) -> Self {
        get_storage(|store| store.store_item(item))
    }
}
