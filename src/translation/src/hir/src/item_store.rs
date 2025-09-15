use crate::item::Item;

use hashbrown::HashMap;
use serde::{Deserialize, Serialize};

#[derive(Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct ItemId {
    id: u32,
}

#[derive(Default)]
pub struct ItemStore {
    items: HashMap<ItemId, Item>,
    next_id: u32,
}

impl ItemStore {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn store(&mut self, item: Item) -> ItemId {
        let id = self.next_id;
        self.items.insert(ItemId { id }, item);
        self.next_id += 1;
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
