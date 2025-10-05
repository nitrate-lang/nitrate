use hashbrown::HashMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

use crate::item::Item;

#[derive(Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct ItemId {
    id: NonZeroU32,
}

pub struct ItemStore {
    items: HashMap<ItemId, Item>,
    next_id: NonZeroU32,
}

impl ItemStore {
    pub fn new() -> Self {
        Self {
            items: HashMap::new(),
            next_id: NonZeroU32::new(1).unwrap(),
        }
    }

    pub fn store(&mut self, item: Item) -> ItemId {
        let id = self.next_id;
        self.items.insert(ItemId { id }, item);

        self.next_id = self
            .next_id
            .checked_add(1)
            .expect("ItemStore overflowed NonZeroU32");

        ItemId { id }
    }

    fn get(&self, id: &ItemId) -> &Item {
        self.items.get(id).expect("ItemId not found in ItemStore")
    }

    fn get_mut(&mut self, id: &ItemId) -> &mut Item {
        self.items
            .get_mut(id)
            .expect("ItemId not found in ItemStore")
    }
}

impl std::ops::Index<&ItemId> for ItemStore {
    type Output = Item;

    fn index(&self, index: &ItemId) -> &Self::Output {
        self.get(index)
    }
}

impl std::ops::IndexMut<&ItemId> for ItemStore {
    fn index_mut(&mut self, index: &ItemId) -> &mut Self::Output {
        self.get_mut(index)
    }
}
