use crate::prelude::hir::*;
use hashbrown::HashMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

macro_rules! impl_store {
    ($handle_name:ident, $item_name:ident, $store_name:ident) => {
        #[derive(Debug, Clone, Serialize, Deserialize)]
        pub struct $handle_name(NonZeroU32);

        pub struct $store_name {
            items: HashMap<NonZeroU32, $item_name>,
            next_id: NonZeroU32,
        }

        impl $store_name {
            pub fn new() -> Self {
                Self {
                    items: HashMap::new(),
                    next_id: NonZeroU32::new(1).unwrap(),
                }
            }

            pub fn store(&mut self, item: $item_name) -> $handle_name {
                let id = self.next_id;
                self.items.insert(id, item);

                self.next_id = self
                    .next_id
                    .checked_add(1)
                    .expect("Store overflowed NonZeroU32");

                $handle_name(id)
            }

            fn get(&self, id: &$handle_name) -> &$item_name {
                self.items.get(&id.0).expect("Id not found in Store")
            }

            fn get_mut(&mut self, id: &$handle_name) -> &mut $item_name {
                self.items.get_mut(&id.0).expect("Id not found in Store")
            }

            pub fn reset(&mut self) {
                self.items = HashMap::new();
                self.next_id = NonZeroU32::new(1).unwrap();
            }
        }

        impl std::ops::Index<&$handle_name> for $store_name {
            type Output = $item_name;

            fn index(&self, index: &$handle_name) -> &Self::Output {
                self.get(index)
            }
        }

        impl std::ops::IndexMut<&$handle_name> for $store_name {
            fn index_mut(&mut self, index: &$handle_name) -> &mut Self::Output {
                self.get_mut(index)
            }
        }
    };
}

impl_store!(TypeId, Type, TypeStore);
impl_store!(ItemId, Item, ItemStore);
impl_store!(SymbolId, Symbol, SymbolStore);
impl_store!(ValueId, Value, ExprValueStore);
impl_store!(BlockId, Block, ExprBlockStore);
impl_store!(PlaceId, Place, ExprPlaceStore);

pub struct Store {
    types: TypeStore,
    items: ItemStore,
    symbols: SymbolStore,
    values: ExprValueStore,
    blocks: ExprBlockStore,
    places: ExprPlaceStore,
}

impl Store {
    pub fn new() -> Self {
        Self {
            types: TypeStore::new(),
            items: ItemStore::new(),
            symbols: SymbolStore::new(),
            values: ExprValueStore::new(),
            blocks: ExprBlockStore::new(),
            places: ExprPlaceStore::new(),
        }
    }

    pub fn store_type(&mut self, ty: Type) -> TypeId {
        self.types.store(ty)
    }

    pub fn store_item(&mut self, item: Item) -> ItemId {
        self.items.store(item)
    }

    pub fn store_symbol(&mut self, symbol: Symbol) -> SymbolId {
        self.symbols.store(symbol)
    }

    pub fn store_expr(&mut self, expr: Value) -> ValueId {
        self.values.store(expr)
    }

    pub fn store_block(&mut self, block: Block) -> BlockId {
        self.blocks.store(block)
    }

    pub fn store_place(&mut self, place: Place) -> PlaceId {
        self.places.store(place)
    }

    pub fn reset(&mut self) {
        self.values.reset();
        self.types.reset();
        self.items.reset();
        self.symbols.reset();
        self.blocks.reset();
        self.places.reset();
    }
}

impl std::ops::Index<&TypeId> for Store {
    type Output = Type;

    fn index(&self, index: &TypeId) -> &Self::Output {
        &self.types[index]
    }
}

impl std::ops::IndexMut<&TypeId> for Store {
    fn index_mut(&mut self, index: &TypeId) -> &mut Self::Output {
        &mut self.types[index]
    }
}

impl std::ops::Index<&ItemId> for Store {
    type Output = Item;

    fn index(&self, index: &ItemId) -> &Self::Output {
        &self.items[index]
    }
}

impl std::ops::IndexMut<&ItemId> for Store {
    fn index_mut(&mut self, index: &ItemId) -> &mut Self::Output {
        &mut self.items[index]
    }
}

impl std::ops::Index<&SymbolId> for Store {
    type Output = Symbol;

    fn index(&self, index: &SymbolId) -> &Self::Output {
        &self.symbols[index]
    }
}

impl std::ops::IndexMut<&SymbolId> for Store {
    fn index_mut(&mut self, index: &SymbolId) -> &mut Self::Output {
        &mut self.symbols[index]
    }
}

impl std::ops::Index<&ValueId> for Store {
    type Output = Value;

    fn index(&self, index: &ValueId) -> &Self::Output {
        &self.values[index]
    }
}

impl std::ops::IndexMut<&ValueId> for Store {
    fn index_mut(&mut self, index: &ValueId) -> &mut Self::Output {
        &mut self.values[index]
    }
}

impl std::ops::Index<&BlockId> for Store {
    type Output = Block;

    fn index(&self, index: &BlockId) -> &Self::Output {
        &self.blocks[index]
    }
}

impl std::ops::IndexMut<&BlockId> for Store {
    fn index_mut(&mut self, index: &BlockId) -> &mut Self::Output {
        &mut self.blocks[index]
    }
}

impl std::ops::Index<&PlaceId> for Store {
    type Output = Place;

    fn index(&self, index: &PlaceId) -> &Self::Output {
        &self.places[index]
    }
}

impl std::ops::IndexMut<&PlaceId> for Store {
    fn index_mut(&mut self, index: &PlaceId) -> &mut Self::Output {
        &mut self.places[index]
    }
}
