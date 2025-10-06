use crate::prelude::*;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::num::NonZeroU32;
use std::sync::Arc;

macro_rules! impl_dedup_store {
    ($handle_name:ident, $item_name:ident, $store_name:ident) => {
        #[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
        pub struct $handle_name(NonZeroU32);

        pub struct $store_name {
            map: HashMap<Arc<$item_name>, $handle_name>,
            quick_vec: Vec<Arc<$item_name>>,
        }

        impl $store_name {
            pub fn new() -> Self {
                Self {
                    map: HashMap::new(),
                    quick_vec: Vec::new(),
                }
            }

            pub fn store(&mut self, item: $item_name) -> $handle_name {
                if let Some(id) = self.map.get(&item) {
                    return id.clone();
                }

                let arc_item = Arc::new(item);
                self.quick_vec.push(arc_item.clone());

                let id = NonZeroU32::new(self.quick_vec.len() as u32).expect("Store overflowed");
                let handle = $handle_name(id);
                self.map.insert(arc_item, handle.clone());

                handle
            }

            fn get(&self, id: &$handle_name) -> &$item_name {
                self.quick_vec
                    .get(id.0.get() as usize - 1)
                    .expect("Id not found in Store")
            }

            pub fn reset(&mut self) {
                self.map = HashMap::new();
                self.quick_vec = Vec::new();
            }

            pub fn shrink_to_fit(&mut self) {
                self.map.shrink_to_fit();
                self.quick_vec.shrink_to_fit();
            }
        }

        impl std::ops::Index<&$handle_name> for $store_name {
            type Output = $item_name;

            fn index(&self, index: &$handle_name) -> &Self::Output {
                self.get(index)
            }
        }
    };
}

macro_rules! impl_store_mut {
    ($handle_name:ident, $item_name:ident, $store_name:ident) => {
        #[derive(Debug, Clone, Serialize, Deserialize)]
        pub struct $handle_name(NonZeroU32);

        pub struct $store_name {
            items: Vec<$item_name>,
        }

        impl $store_name {
            pub fn new() -> Self {
                Self { items: Vec::new() }
            }

            pub fn store(&mut self, item: $item_name) -> $handle_name {
                self.items.push(item);
                let id = NonZeroU32::new(self.items.len() as u32).unwrap();
                $handle_name(id)
            }

            fn get(&self, id: &$handle_name) -> &$item_name {
                self.items
                    .get(id.0.get() as usize - 1)
                    .expect("Id not found in Store")
            }

            fn get_mut(&mut self, id: &$handle_name) -> &mut $item_name {
                self.items
                    .get_mut(id.0.get() as usize - 1)
                    .expect("Id not found in Store")
            }

            pub fn reset(&mut self) {
                self.items = Vec::new();
            }

            pub fn shrink_to_fit(&mut self) {
                self.items.shrink_to_fit();
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

impl_dedup_store!(TypeId, Type, TypeStore);

impl_dedup_store!(TypeListId, TypeList, TypeListStore);

impl_dedup_store!(StructFieldsId, StructFields, StructFieldsStore);

impl_dedup_store!(EnumVariantsId, EnumVariants, EnumVariantsStore);

impl_dedup_store!(StructAttributesId, StructAttributes, StructAttributesStore);

impl_dedup_store!(EnumAttributesId, EnumAttributes, EnumAttributesStore);

impl_dedup_store!(FuncAttributesId, FunctionAttributes, FuncAttributesStore);

impl_store_mut!(ItemId, Item, ItemStore);

impl_store_mut!(SymbolId, Symbol, SymbolStore);

impl_store_mut!(ValueId, Value, ExprValueStore);

impl_dedup_store!(LiteralId, Literal, ExprLiteralStore);

impl_store_mut!(BlockId, Block, ExprBlockStore);

impl_store_mut!(PlaceId, Place, ExprPlaceStore);

pub struct Store {
    types: TypeStore,
    type_lists: TypeListStore,
    struct_fields: StructFieldsStore,
    enum_variants: EnumVariantsStore,
    struct_attributes: StructAttributesStore,
    enum_attributes: EnumAttributesStore,
    function_attributes: FuncAttributesStore,
    items: ItemStore,
    symbols: SymbolStore,
    values: ExprValueStore,
    literals: ExprLiteralStore,
    blocks: ExprBlockStore,
    places: ExprPlaceStore,
}

impl Store {
    pub fn new() -> Self {
        Self {
            types: TypeStore::new(),
            type_lists: TypeListStore::new(),
            struct_fields: StructFieldsStore::new(),
            enum_variants: EnumVariantsStore::new(),
            struct_attributes: StructAttributesStore::new(),
            enum_attributes: EnumAttributesStore::new(),
            function_attributes: FuncAttributesStore::new(),

            items: ItemStore::new(),
            symbols: SymbolStore::new(),
            values: ExprValueStore::new(),
            literals: ExprLiteralStore::new(),
            blocks: ExprBlockStore::new(),
            places: ExprPlaceStore::new(),
        }
    }

    pub fn store_type(&mut self, ty: Type) -> TypeId {
        self.types.store(ty)
    }

    pub fn store_type_list(&mut self, ty_list: TypeList) -> TypeListId {
        self.type_lists.store(ty_list)
    }

    pub fn store_struct_fields(&mut self, fields: StructFields) -> StructFieldsId {
        self.struct_fields.store(fields)
    }

    pub fn store_enum_variants(&mut self, variants: EnumVariants) -> EnumVariantsId {
        self.enum_variants.store(variants)
    }

    pub fn store_struct_attributes(&mut self, attrs: StructAttributes) -> StructAttributesId {
        self.struct_attributes.store(attrs)
    }

    pub fn store_enum_attributes(&mut self, attrs: EnumAttributes) -> EnumAttributesId {
        self.enum_attributes.store(attrs)
    }

    pub fn store_function_attributes(&mut self, attrs: FunctionAttributes) -> FuncAttributesId {
        self.function_attributes.store(attrs)
    }

    pub fn store_item(&mut self, item: Item) -> ItemId {
        self.items.store(item)
    }

    pub fn store_symbol(&mut self, symbol: Symbol) -> SymbolId {
        self.symbols.store(symbol)
    }

    pub fn store_value(&mut self, expr: Value) -> ValueId {
        self.values.store(expr)
    }

    pub fn store_literal(&mut self, literal: Literal) -> LiteralId {
        self.literals.store(literal)
    }

    pub fn store_block(&mut self, block: Block) -> BlockId {
        self.blocks.store(block)
    }

    pub fn store_place(&mut self, place: Place) -> PlaceId {
        self.places.store(place)
    }

    pub fn reset(&mut self) {
        self.types.reset();
        self.type_lists.reset();
        self.struct_fields.reset();
        self.enum_variants.reset();
        self.struct_attributes.reset();
        self.enum_attributes.reset();
        self.function_attributes.reset();
        self.items.reset();
        self.symbols.reset();
        self.values.reset();
        self.literals.reset();
        self.blocks.reset();
        self.places.reset();
    }

    pub fn shrink_to_fit(&mut self) {
        self.types.shrink_to_fit();
        self.type_lists.shrink_to_fit();
        self.struct_fields.shrink_to_fit();
        self.enum_variants.shrink_to_fit();
        self.struct_attributes.shrink_to_fit();
        self.enum_attributes.shrink_to_fit();
        self.function_attributes.shrink_to_fit();
        self.items.shrink_to_fit();
        self.symbols.shrink_to_fit();
        self.values.shrink_to_fit();
        self.literals.shrink_to_fit();
        self.blocks.shrink_to_fit();
        self.places.shrink_to_fit();
    }
}

impl std::ops::Index<&TypeId> for Store {
    type Output = Type;

    fn index(&self, index: &TypeId) -> &Self::Output {
        &self.types[index]
    }
}

impl std::ops::Index<&TypeListId> for Store {
    type Output = TypeList;

    fn index(&self, index: &TypeListId) -> &Self::Output {
        &self.type_lists[index]
    }
}

impl std::ops::Index<&StructFieldsId> for Store {
    type Output = StructFields;

    fn index(&self, index: &StructFieldsId) -> &Self::Output {
        &self.struct_fields[index]
    }
}

impl std::ops::Index<&EnumVariantsId> for Store {
    type Output = EnumVariants;

    fn index(&self, index: &EnumVariantsId) -> &Self::Output {
        &self.enum_variants[index]
    }
}

impl std::ops::Index<&StructAttributesId> for Store {
    type Output = StructAttributes;

    fn index(&self, index: &StructAttributesId) -> &Self::Output {
        &self.struct_attributes[index]
    }
}

impl std::ops::Index<&EnumAttributesId> for Store {
    type Output = EnumAttributes;

    fn index(&self, index: &EnumAttributesId) -> &Self::Output {
        &self.enum_attributes[index]
    }
}

impl std::ops::Index<&FuncAttributesId> for Store {
    type Output = FunctionAttributes;

    fn index(&self, index: &FuncAttributesId) -> &Self::Output {
        &self.function_attributes[index]
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

impl std::ops::Index<&LiteralId> for Store {
    type Output = Literal;

    fn index(&self, index: &LiteralId) -> &Self::Output {
        &self.literals[index]
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
