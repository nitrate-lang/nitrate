use crate::prelude::*;
use bitflags::bitflags;
use nitrate_nstring::NString;
use serde::{Deserialize, Serialize};
use thin_vec::ThinVec;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Vis {
    Pub,
    Pro,
    Sec,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct AttributeListFlags(u8);

bitflags! {
    impl AttributeListFlags: u8 {
        const TRAILING_COMMA_PRESENT = 0b00000001;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct AttributeList {
    pub source_offset: u32,
    pub flags: AttributeListFlags,
    pub trivia: [Option<Trivia>; 1],
    pub attributes: ThinVec<ExprId>,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Item {
    Root {
        items: ThinVec<ItemId>,
    },

    Trivia {
        trivia: Option<Trivia>,
    },

    Visibility {
        source_offset: u32,
        trivia: [Option<Trivia>; 1],
        vis: Vis,
        item: ItemId,
    },

    Module {
        source_offset: u32,
        trivia: [Option<Trivia>; 3],
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
