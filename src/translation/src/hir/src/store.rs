use crate::prelude::*;
use append_only_vec::AppendOnlyVec;
use bimap::BiMap;
use serde::{Deserialize, Serialize};
use std::cell::{Cell, RefCell};
use std::num::NonZeroU32;
use std::ops::Deref;
use std::sync::{Arc, RwLock};

thread_local! {
    static TLS_STORE: Cell<Option<*const Store>> = const { Cell::new(None) };
}

pub fn using_storage<R>(store: &Store, f: impl FnOnce() -> R) -> R {
    TLS_STORE.with(|tls| {
        let old = tls.take();
        tls.set(Some(store));
        let result = f();
        tls.set(old); // Ensure panic when misused
        result
    })
}

pub fn get_storage<R>(f: impl FnOnce(&Store) -> R) -> R {
    TLS_STORE.with(|tls| {
        let store_ptr = tls
            .get()
            .expect("No Store found in TLS. Did you forget to call using_storage?");

        let store = unsafe { &*store_ptr };
        f(store)
    })
}

macro_rules! impl_dedup_store {
    ($handle_name:ident, $item_name:ident, $store_name:ident) => {
        #[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
        pub struct $handle_name(NonZeroU32);

        impl $handle_name {
            pub fn as_usize(&self) -> usize {
                self.0.get() as usize
            }
        }

        impl std::ops::Deref for $handle_name {
            type Target = $item_name;

            fn deref(&self) -> &Self::Target {
                TLS_STORE.with(|tls| {
                    let store_ptr = tls
                        .get()
                        .expect("No Store found in TLS. Did you forget to call using_storage?");

                    let store = unsafe { &*store_ptr };
                    &store[self]
                })
            }
        }

        #[derive(Debug)]
        pub struct $store_name {
            bimap: RwLock<BiMap<Arc<$item_name>, $handle_name>>,
            quick_vec: AppendOnlyVec<Arc<$item_name>>,
        }

        impl Default for $store_name {
            fn default() -> Self {
                Self::new()
            }
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

        impl $handle_name {
            pub fn as_usize(&self) -> usize {
                self.0.get() as usize
            }
        }

        impl std::ops::Deref for $handle_name {
            type Target = RefCell<$item_name>;

            fn deref(&self) -> &Self::Target {
                TLS_STORE.with(|tls| {
                    let store_ptr = tls
                        .get()
                        .expect("No Store found in TLS. Did you forget to call using_storage?");

                    let store = unsafe { &*store_ptr };
                    &store[self]
                })
            }
        }

        impl std::cmp::PartialEq for $handle_name {
            fn eq(&self, other: &Self) -> bool {
                self.deref() == other.deref()
            }
        }

        impl std::cmp::Eq for $handle_name {}

        impl std::hash::Hash for $handle_name {
            fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
                self.deref().borrow().hash(state);
            }
        }

        #[derive(Debug)]
        pub struct $store_name {
            vec: AppendOnlyVec<RefCell<$item_name>>,
        }

        impl Default for $store_name {
            fn default() -> Self {
                Self::new()
            }
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

impl_store_mut!(GlobalVariableId, GlobalVariable, GlobalVariableStore);

impl_store_mut!(LocalVariableId, LocalVariable, LocalVariableStore);

impl_store_mut!(ParameterId, Parameter, ParameterStore);

impl_store_mut!(FunctionId, Function, FunctionStore);

impl_store_mut!(TraitId, Trait, TraitStore);

impl_store_mut!(ModuleId, Module, ModuleStore);

impl_store_mut!(TypeAliasDefId, TypeAliasDef, TypeAliasStore);

impl_store_mut!(StructDefId, StructDef, StructDefStore);

impl_store_mut!(EnumDefId, EnumDef, EnumDefStore);

impl_store_mut!(ValueId, Value, ExprValueStore);

impl_dedup_store!(LiteralId, Lit, ExprLiteralStore);

impl_store_mut!(BlockId, Block, ExprBlockStore);

#[derive(Debug)]
pub struct Store {
    types: TypeStore,
    global_variables: GlobalVariableStore,
    local_variables: LocalVariableStore,
    parameters: ParameterStore,
    functions: FunctionStore,
    traits: TraitStore,
    modules: ModuleStore,
    type_aliases: TypeAliasStore,
    struct_defs: StructDefStore,
    enum_defs: EnumDefStore,
    values: ExprValueStore,
    literals: ExprLiteralStore,
    blocks: ExprBlockStore,
}

impl Default for Store {
    fn default() -> Self {
        Self::new()
    }
}

impl Store {
    #[must_use]
    pub fn new() -> Self {
        Self {
            types: TypeStore::new(),
            global_variables: GlobalVariableStore::new(),
            local_variables: LocalVariableStore::new(),
            parameters: ParameterStore::new(),
            functions: FunctionStore::new(),
            traits: TraitStore::new(),
            modules: ModuleStore::new(),
            type_aliases: TypeAliasStore::new(),
            struct_defs: StructDefStore::new(),
            enum_defs: EnumDefStore::new(),
            values: ExprValueStore::new(),
            literals: ExprLiteralStore::new(),
            blocks: ExprBlockStore::new(),
        }
    }

    pub fn store_type(&self, ty: Type) -> TypeId {
        using_storage(self, || self.types.store(ty))
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

    pub fn store_type_alias(&self, type_alias: TypeAliasDef) -> TypeAliasDefId {
        using_storage(self, || self.type_aliases.store(type_alias))
    }

    pub fn store_struct_def(&self, struct_def: StructDef) -> StructDefId {
        using_storage(self, || self.struct_defs.store(struct_def))
    }

    pub fn store_enum_def(&self, enum_def: EnumDef) -> EnumDefId {
        using_storage(self, || self.enum_defs.store(enum_def))
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
        self.global_variables.reset();
        self.local_variables.reset();
        self.parameters.reset();
        self.functions.reset();
        self.traits.reset();
        self.modules.reset();
        self.type_aliases.reset();
        self.struct_defs.reset();
        self.enum_defs.reset();
        self.values.reset();
        self.literals.reset();
        self.blocks.reset();
    }

    pub fn shrink_to_fit(&mut self) {
        self.types.shrink_to_fit();
        self.global_variables.shrink_to_fit();
        self.local_variables.shrink_to_fit();
        self.parameters.shrink_to_fit();
        self.functions.shrink_to_fit();
        self.traits.shrink_to_fit();
        self.modules.shrink_to_fit();
        self.type_aliases.shrink_to_fit();
        self.struct_defs.shrink_to_fit();
        self.enum_defs.shrink_to_fit();
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

impl std::ops::Index<&TypeAliasDefId> for Store {
    type Output = RefCell<TypeAliasDef>;

    fn index(&self, index: &TypeAliasDefId) -> &Self::Output {
        &self.type_aliases[index]
    }
}

impl std::ops::Index<&StructDefId> for Store {
    type Output = RefCell<StructDef>;

    fn index(&self, index: &StructDefId) -> &Self::Output {
        &self.struct_defs[index]
    }
}

impl std::ops::Index<&EnumDefId> for Store {
    type Output = RefCell<EnumDef>;

    fn index(&self, index: &EnumDefId) -> &Self::Output {
        &self.enum_defs[index]
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
