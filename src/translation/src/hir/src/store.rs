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

        #[derive(Debug)]
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

        #[derive(Debug)]
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

impl_dedup_store!(StructTypeId, StructType, StructTypeStore);

impl_dedup_store!(EnumTypeId, EnumType, EnumTypeStore);

impl_dedup_store!(FunctionTypeId, FunctionType, FunctionTypeStore);

impl_store_mut!(SymbolId, Symbol, SymbolStore);

impl_store_mut!(GlobalVariableId, GlobalVariable, GlobalVariableStore);

impl_store_mut!(LocalVariableId, LocalVariable, LocalVariableStore);

impl_store_mut!(ParameterId, Parameter, ParameterStore);

impl_store_mut!(FunctionId, Function, FunctionStore);

impl_store_mut!(TraitId, Trait, TraitStore);

impl_store_mut!(ModuleId, Module, ModuleStore);

impl_store_mut!(ValueId, Value, ExprValueStore);

impl_dedup_store!(LiteralId, Lit, ExprLiteralStore);

impl_store_mut!(BlockId, Block, ExprBlockStore);

#[derive(Debug)]
pub struct Store {
    types: TypeStore,
    struct_types: StructTypeStore,
    enum_types: EnumTypeStore,
    function_types: FunctionTypeStore,
    symbols: SymbolStore,
    global_variables: GlobalVariableStore,
    local_variables: LocalVariableStore,
    parameters: ParameterStore,
    functions: FunctionStore,
    traits: TraitStore,
    modules: ModuleStore,
    values: ExprValueStore,
    literals: ExprLiteralStore,
    blocks: ExprBlockStore,
}

impl Store {
    pub fn new() -> Self {
        Self {
            types: TypeStore::new(),
            struct_types: StructTypeStore::new(),
            enum_types: EnumTypeStore::new(),
            function_types: FunctionTypeStore::new(),
            symbols: SymbolStore::new(),
            global_variables: GlobalVariableStore::new(),
            local_variables: LocalVariableStore::new(),
            parameters: ParameterStore::new(),
            functions: FunctionStore::new(),
            traits: TraitStore::new(),
            modules: ModuleStore::new(),
            values: ExprValueStore::new(),
            literals: ExprLiteralStore::new(),
            blocks: ExprBlockStore::new(),
        }
    }

    pub fn store_type(&self, ty: Type) -> TypeId {
        using_storage(self, || self.types.store(ty))
    }

    pub fn store_struct_type(&self, struct_type: StructType) -> StructTypeId {
        using_storage(self, || self.struct_types.store(struct_type))
    }

    pub fn store_enum_type(&self, enum_type: EnumType) -> EnumTypeId {
        using_storage(self, || self.enum_types.store(enum_type))
    }

    pub fn store_function_type(&self, func_type: FunctionType) -> FunctionTypeId {
        using_storage(self, || self.function_types.store(func_type))
    }

    pub fn store_symbol(&self, symbol: Symbol) -> SymbolId {
        using_storage(self, || self.symbols.store(symbol))
    }

    pub fn store_global_variable(&self, var: GlobalVariable) -> GlobalVariableId {
        using_storage(self, || self.global_variables.store(var))
    }

    pub fn store_local_variable(&self, var: LocalVariable) -> LocalVariableId {
        using_storage(self, || self.local_variables.store(var))
    }

    pub fn store_parameter(&self, param: Parameter) -> ParameterId {
        using_storage(self, || self.parameters.store(param))
    }

    pub fn store_function(&self, func: Function) -> FunctionId {
        using_storage(self, || self.functions.store(func))
    }

    pub fn store_trait(&self, tr: Trait) -> TraitId {
        using_storage(self, || self.traits.store(tr))
    }

    pub fn store_module(&self, module: Module) -> ModuleId {
        using_storage(self, || self.modules.store(module))
    }

    pub fn store_value(&self, expr: Value) -> ValueId {
        using_storage(self, || self.values.store(expr))
    }

    pub fn store_literal(&self, literal: Lit) -> LiteralId {
        using_storage(self, || self.literals.store(literal))
    }

    pub fn store_block(&self, block: Block) -> BlockId {
        using_storage(self, || self.blocks.store(block))
    }

    pub fn reset(&mut self) {
        self.types.reset();
        self.struct_types.reset();
        self.enum_types.reset();
        self.function_types.reset();
        self.symbols.reset();
        self.global_variables.reset();
        self.local_variables.reset();
        self.parameters.reset();
        self.functions.reset();
        self.traits.reset();
        self.modules.reset();
        self.values.reset();
        self.literals.reset();
        self.blocks.reset();
    }

    pub fn shrink_to_fit(&mut self) {
        self.types.shrink_to_fit();
        self.struct_types.shrink_to_fit();
        self.enum_types.shrink_to_fit();
        self.function_types.shrink_to_fit();
        self.symbols.shrink_to_fit();
        self.global_variables.shrink_to_fit();
        self.local_variables.shrink_to_fit();
        self.parameters.shrink_to_fit();
        self.functions.shrink_to_fit();
        self.traits.shrink_to_fit();
        self.modules.shrink_to_fit();
        self.values.shrink_to_fit();
        self.literals.shrink_to_fit();
        self.blocks.shrink_to_fit();
    }
}

impl std::ops::Index<&TypeId> for Store {
    type Output = Type;

    fn index(&self, index: &TypeId) -> &Self::Output {
        &self.types[index]
    }
}

impl std::ops::Index<&StructTypeId> for Store {
    type Output = StructType;

    fn index(&self, index: &StructTypeId) -> &Self::Output {
        &self.struct_types[index]
    }
}

impl std::ops::Index<&EnumTypeId> for Store {
    type Output = EnumType;

    fn index(&self, index: &EnumTypeId) -> &Self::Output {
        &self.enum_types[index]
    }
}

impl std::ops::Index<&FunctionTypeId> for Store {
    type Output = FunctionType;

    fn index(&self, index: &FunctionTypeId) -> &Self::Output {
        &self.function_types[index]
    }
}

impl std::ops::Index<&SymbolId> for Store {
    type Output = RefCell<Symbol>;

    fn index(&self, index: &SymbolId) -> &Self::Output {
        &self.symbols[index]
    }
}

impl std::ops::Index<&GlobalVariableId> for Store {
    type Output = RefCell<GlobalVariable>;

    fn index(&self, index: &GlobalVariableId) -> &Self::Output {
        &self.global_variables[index]
    }
}

impl std::ops::Index<&LocalVariableId> for Store {
    type Output = RefCell<LocalVariable>;

    fn index(&self, index: &LocalVariableId) -> &Self::Output {
        &self.local_variables[index]
    }
}

impl std::ops::Index<&ParameterId> for Store {
    type Output = RefCell<Parameter>;

    fn index(&self, index: &ParameterId) -> &Self::Output {
        &self.parameters[index]
    }
}

impl std::ops::Index<&FunctionId> for Store {
    type Output = RefCell<Function>;

    fn index(&self, index: &FunctionId) -> &Self::Output {
        &self.functions[index]
    }
}

impl std::ops::Index<&TraitId> for Store {
    type Output = RefCell<Trait>;

    fn index(&self, index: &TraitId) -> &Self::Output {
        &self.traits[index]
    }
}

impl std::ops::Index<&ModuleId> for Store {
    type Output = RefCell<Module>;

    fn index(&self, index: &ModuleId) -> &Self::Output {
        &self.modules[index]
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
