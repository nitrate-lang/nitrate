use crate::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Item {}

impl From<Item> for ItemId {
    fn from(item: Item) -> Self {
        get_storage(|store| store.store_item(item))
    }
}
