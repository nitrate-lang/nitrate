use crate::{expr::Expr, expr_place::Place, hir::Block, item::Item, ty::Type};
use hashbrown::HashMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

macro_rules! impl_store {
    ($handle_name:ident, $item_name:ident, $store_name:ident) => {
        #[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
        pub struct $handle_name {
            id: NonZeroU32,
        }

        pub struct $store_name {
            items: HashMap<$handle_name, $item_name>,
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
                self.items.insert($handle_name { id }, item);

                self.next_id = self
                    .next_id
                    .checked_add(1)
                    .expect("Store overflowed NonZeroU32");

                $handle_name { id }
            }

            fn get(&self, id: &$handle_name) -> &$item_name {
                self.items.get(id).expect("Id not found in Store")
            }

            fn get_mut(&mut self, id: &$handle_name) -> &mut $item_name {
                self.items.get_mut(id).expect("Id not found in Store")
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
impl_store!(ExprId, Expr, ExprStore);
impl_store!(BlockId, Block, BlockStore);
impl_store!(PlaceId, Place, PlaceStore);

pub struct Store {
    types: TypeStore,
    items: ItemStore,
    exprs: ExprStore,
    blocks: BlockStore,
    places: PlaceStore,
}

impl Store {
    pub fn new() -> Self {
        Self {
            types: TypeStore::new(),
            items: ItemStore::new(),
            exprs: ExprStore::new(),
            blocks: BlockStore::new(),
            places: PlaceStore::new(),
        }
    }

    pub fn store_type(&mut self, ty: Type) -> TypeId {
        self.types.store(ty)
    }

    pub fn store_item(&mut self, item: Item) -> ItemId {
        self.items.store(item)
    }

    pub fn store_expr(&mut self, expr: Expr) -> ExprId {
        self.exprs.store(expr)
    }

    pub fn store_block(&mut self, block: Block) -> BlockId {
        self.blocks.store(block)
    }

    pub fn store_place(&mut self, place: Place) -> PlaceId {
        self.places.store(place)
    }

    pub fn reset(&mut self) {
        self.exprs.reset();
        self.types.reset();
        self.items.reset();
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

impl std::ops::Index<&ExprId> for Store {
    type Output = Expr;

    fn index(&self, index: &ExprId) -> &Self::Output {
        &self.exprs[index]
    }
}

impl std::ops::IndexMut<&ExprId> for Store {
    fn index_mut(&mut self, index: &ExprId) -> &mut Self::Output {
        &mut self.exprs[index]
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
