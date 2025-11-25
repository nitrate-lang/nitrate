use crate::prelude::*;
use bitflags::bitflags;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use thin_vec::ThinVec;

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct AttributeList {
    /* .. [<expr>, ... ] */
    pub source_offset: u32,
    pub trivia: [Trivia; 1],
    pub attributes: ThinVec<ExprId>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct ItemModulePresent(u8);

bitflags! {
    impl ItemModulePresent: u8 {
        const OPEN_BRACE_PRESENT =  0b00000001;
        const CLOSE_BRACE_PRESENT = 0b00000010;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Item {
    Root {
        items: ThinVec<ItemId>,
    },

    Module {
        /* .. mod <attributes_list>? .. <name> .. {} */
        source_offset: u32,
        present: ItemModulePresent,
        trivia: [Trivia; 3],
        attributes: Option<AttributeList>,
        name: NString,
        items: ThinVec<ItemId>,
    },
}

impl From<Item> for ItemId {
    fn from(item: Item) -> Self {
        get_storage(|store| store.store_item(item))
    }
}
