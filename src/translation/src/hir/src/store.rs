use crate::prelude::*;
use append_only_vec::AppendOnlyVec;
use bimap::BiMap;
use serde::{Deserialize, Serialize};
use std::cell::RefCell;
use std::num::NonZeroU32;
use std::sync::{Arc, RwLock};

macro_rules! impl_dedup_store {
    ($handle_name:ident, $item_name:ident, $store_name:ident) => {
        #[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
        pub struct $handle_name(NonZeroU32);

        pub struct $store_name {
            bimap: RwLock<BiMap<Arc<$item_name>, $handle_name>>,
            quick_vec: AppendOnlyVec<Arc<$item_name>>,
        }

        impl $store_name {
            pub fn new() -> Self {
                Self {
                    bimap: RwLock::new(BiMap::new()),
                    quick_vec: AppendOnlyVec::new(),
                }
            }

            pub fn store(&self, item: $item_name) -> $handle_name {
                if let Some(id) = self.bimap.read().unwrap().get_by_left(&item) {
                    return id.clone();
                }

                let mut bimap = self.bimap.write().unwrap();
                if let Some(id) = bimap.get_by_left(&item) {
                    return id.clone();
                }

                let arc_item = Arc::new(item);
                self.quick_vec.push(arc_item.clone());

                let id = NonZeroU32::new(self.quick_vec.len() as u32).expect("Store overflowed");
                let handle = $handle_name(id);
                bimap.insert(arc_item, handle.clone());

                handle
            }

            fn get(&self, id: &$handle_name) -> &$item_name {
                &self.quick_vec[id.0.get() as usize - 1]
            }

            pub fn reset(&mut self) {
                self.bimap = RwLock::new(BiMap::new());
                self.quick_vec = AppendOnlyVec::new();
            }

            pub fn shrink_to_fit(&self) {
                self.bimap.write().unwrap().shrink_to_fit();
                // AppendOnlyVec does not have a shrink_to_fit method
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
            vec: AppendOnlyVec<RefCell<$item_name>>,
        }

        impl $store_name {
            pub fn new() -> Self {
                Self {
                    vec: AppendOnlyVec::new(),
                }
            }

            pub fn store(&self, item: $item_name) -> $handle_name {
                self.vec.push(RefCell::new(item));
                let id = NonZeroU32::new(self.vec.len() as u32).unwrap();
                $handle_name(id)
            }

            fn get(&self, id: &$handle_name) -> &RefCell<$item_name> {
                &self.vec[id.0.get() as usize - 1]
            }

            pub fn reset(&mut self) {
                self.vec = AppendOnlyVec::new();
            }

            pub fn shrink_to_fit(&mut self) {
                // AppendOnlyVec does not have a shrink_to_fit method
            }
        }

        impl std::ops::Index<&$handle_name> for $store_name {
            type Output = RefCell<$item_name>;

            fn index(&self, index: &$handle_name) -> &Self::Output {
                self.get(index)
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

impl_dedup_store!(LiteralId, Lit, ExprLiteralStore);

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

    pub fn store_type(&self, ty: Type) -> TypeId {
        self.types.store(ty)
    }

    pub fn store_type_list(&self, ty_list: TypeList) -> TypeListId {
        self.type_lists.store(ty_list)
    }

    pub fn store_struct_fields(&self, fields: StructFields) -> StructFieldsId {
        self.struct_fields.store(fields)
    }

    pub fn store_enum_variants(&self, variants: EnumVariants) -> EnumVariantsId {
        self.enum_variants.store(variants)
    }

    pub fn store_struct_attributes(&self, attrs: StructAttributes) -> StructAttributesId {
        self.struct_attributes.store(attrs)
    }

    pub fn store_enum_attributes(&self, attrs: EnumAttributes) -> EnumAttributesId {
        self.enum_attributes.store(attrs)
    }

    pub fn store_function_attributes(&self, attrs: FunctionAttributes) -> FuncAttributesId {
        self.function_attributes.store(attrs)
    }

    pub fn store_item(&self, item: Item) -> ItemId {
        self.items.store(item)
    }

    pub fn store_symbol(&self, symbol: Symbol) -> SymbolId {
        self.symbols.store(symbol)
    }

    pub fn store_value(&self, expr: Value) -> ValueId {
        self.values.store(expr)
    }

    pub fn store_literal(&self, literal: Lit) -> LiteralId {
        self.literals.store(literal)
    }

    pub fn store_block(&self, block: Block) -> BlockId {
        self.blocks.store(block)
    }

    pub fn store_place(&self, place: Place) -> PlaceId {
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
    type Output = RefCell<Item>;

    fn index(&self, index: &ItemId) -> &Self::Output {
        &self.items[index]
    }
}

impl std::ops::Index<&SymbolId> for Store {
    type Output = RefCell<Symbol>;

    fn index(&self, index: &SymbolId) -> &Self::Output {
        &self.symbols[index]
    }
}

impl std::ops::Index<&ValueId> for Store {
    type Output = RefCell<Value>;

    fn index(&self, index: &ValueId) -> &Self::Output {
        &self.values[index]
    }
}

impl std::ops::Index<&LiteralId> for Store {
    type Output = Lit;

    fn index(&self, index: &LiteralId) -> &Self::Output {
        &self.literals[index]
    }
}

impl std::ops::Index<&BlockId> for Store {
    type Output = RefCell<Block>;

    fn index(&self, index: &BlockId) -> &Self::Output {
        &self.blocks[index]
    }
}

impl std::ops::Index<&PlaceId> for Store {
    type Output = RefCell<Place>;

    fn index(&self, index: &PlaceId) -> &Self::Output {
        &self.places[index]
    }
}
