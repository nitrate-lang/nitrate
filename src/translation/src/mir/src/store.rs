use crate::prelude::*;
use append_only_vec::AppendOnlyVec;
use bimap::BiMap;
use serde::{Deserialize, Serialize};
use std::cell::{Cell, RefCell};
use std::num::NonZeroU32;
use std::ops::Deref;
use std::sync::{Arc, RwLock};

// ─────────────────────────────────────────────────────────────
// TLS storage infrastructure (same pattern as HIR)
// ─────────────────────────────────────────────────────────────

thread_local! {
    static TLS_STORE: Cell<Option<*const MirStore>> = const { Cell::new(None) };
}

pub fn using_storage<R>(store: &MirStore, f: impl FnOnce() -> R) -> R {
    TLS_STORE.with(|tls| {
        let old = tls.take();
        tls.set(Some(store));
        let result = f();
        tls.set(old);
        result
    })
}

pub fn get_storage<R>(f: impl FnOnce(&MirStore) -> R) -> R {
    TLS_STORE.with(|tls| {
        let store_ptr = tls
            .get()
            .expect("No MirStore found in TLS. Did you forget to call using_storage?");
        let store = unsafe { &*store_ptr };
        f(store)
    })
}

// ─────────────────────────────────────────────────────────────
// Deduplicated type store
// ─────────────────────────────────────────────────────────────

#[derive(Clone, Copy, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct MirTypeId(NonZeroU32);

impl MirTypeId {
    pub fn as_usize(&self) -> usize {
        self.0.get() as usize
    }
}

impl std::ops::Deref for MirTypeId {
    type Target = MirType;

    fn deref(&self) -> &Self::Target {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("No MirStore found in TLS. Did you forget to call using_storage?");
            let store = unsafe { &*store_ptr };
            &store.types[self]
        })
    }
}

impl std::fmt::Debug for MirTypeId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.deref().fmt(f)
    }
}

#[derive(Debug)]
pub struct MirTypeStore {
    bimap: RwLock<BiMap<Arc<MirType>, MirTypeId>>,
    quick_vec: AppendOnlyVec<Arc<MirType>>,
}

impl Default for MirTypeStore {
    fn default() -> Self {
        Self::new()
    }
}

impl MirTypeStore {
    pub fn new() -> Self {
        Self {
            bimap: RwLock::new(BiMap::new()),
            quick_vec: AppendOnlyVec::new(),
        }
    }

    pub fn store(&self, item: MirType) -> MirTypeId {
        {
            let read = self.bimap.read().unwrap();
            if let Some(id) = read.get_by_left(&item) {
                return id.clone();
            }
        }

        let mut bimap = self.bimap.write().unwrap();
        if let Some(id) = bimap.get_by_left(&item) {
            return id.clone();
        }

        let arc_item = Arc::new(item);
        self.quick_vec.push(arc_item.clone());
        let id = NonZeroU32::new(self.quick_vec.len() as u32).expect("MirTypeStore overflowed");
        let handle = MirTypeId(id);
        bimap.insert(arc_item, handle.clone());
        handle
    }

    fn get(&self, id: &MirTypeId) -> &MirType {
        &self.quick_vec[id.0.get() as usize - 1]
    }

    pub fn reset(&mut self) {
        self.bimap = RwLock::new(BiMap::new());
        self.quick_vec = AppendOnlyVec::new();
    }
}

impl std::ops::Index<&MirTypeId> for MirTypeStore {
    type Output = MirType;

    fn index(&self, index: &MirTypeId) -> &Self::Output {
        self.get(index)
    }
}

// ─────────────────────────────────────────────────────────────
// Append-only stores for mutable items
// ─────────────────────────────────────────────────────────────

macro_rules! impl_append_store {
    ($handle_name:ident, $item_name:ty, $store_name:ident) => {
        #[derive(Clone, Copy, Serialize, Deserialize)]
        pub struct $handle_name(NonZeroU32);

        impl $handle_name {
            #[must_use]
            pub fn clone(&self) -> Self {
                *self
            }
        }

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
                        .expect("No MirStore found in TLS. Did you forget to call using_storage?");
                    let store = unsafe { &*store_ptr };
                    &store[self]
                })
            }
        }

        impl std::fmt::Debug for $handle_name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                self.deref().borrow().fmt(f)
            }
        }

        impl std::cmp::PartialEq for $handle_name {
            fn eq(&self, other: &Self) -> bool {
                self.deref().as_ptr() == other.deref().as_ptr()
            }
        }

        impl std::cmp::Eq for $handle_name {}

        impl std::hash::Hash for $handle_name {
            fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
                self.0.hash(state);
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
        }

        impl std::ops::Index<&$handle_name> for $store_name {
            type Output = RefCell<$item_name>;

            fn index(&self, index: &$handle_name) -> &Self::Output {
                self.get(index)
            }
        }
    };
}

// ─────────────────────────────────────────────────────────────
// Concrete store types
// ─────────────────────────────────────────────────────────────

impl_append_store!(LocalId, LocalDecl, LocalStore);
impl_append_store!(BasicBlockId, BasicBlock, BasicBlockStore);
impl_append_store!(MirFunctionId, MirFunction, FunctionStore);

// ─────────────────────────────────────────────────────────────
// The unified MIR store
// ─────────────────────────────────────────────────────────────

#[derive(Debug)]
pub struct MirStore {
    types: MirTypeStore,
    locals: LocalStore,
    basic_blocks: BasicBlockStore,
    functions: FunctionStore,
}

impl Default for MirStore {
    fn default() -> Self {
        Self::new()
    }
}

impl MirStore {
    #[must_use]
    pub fn new() -> Self {
        Self {
            types: MirTypeStore::new(),
            locals: LocalStore::new(),
            basic_blocks: BasicBlockStore::new(),
            functions: FunctionStore::new(),
        }
    }

    pub fn store_type(&self, ty: MirType) -> MirTypeId {
        using_storage(self, || self.types.store(ty))
    }

    pub fn store_local(&self, local: LocalDecl) -> LocalId {
        using_storage(self, || self.locals.store(local))
    }

    pub fn store_basic_block(&self, bb: BasicBlock) -> BasicBlockId {
        using_storage(self, || self.basic_blocks.store(bb))
    }

    pub fn store_function(&self, func: MirFunction) -> MirFunctionId {
        using_storage(self, || self.functions.store(func))
    }

    pub fn reset(&mut self) {
        self.types.reset();
        self.locals.reset();
        self.basic_blocks.reset();
        self.functions.reset();
    }
}

impl std::ops::Index<&MirTypeId> for MirStore {
    type Output = MirType;

    fn index(&self, index: &MirTypeId) -> &Self::Output {
        &self.types[index]
    }
}

impl std::ops::Index<&LocalId> for MirStore {
    type Output = RefCell<LocalDecl>;

    fn index(&self, index: &LocalId) -> &Self::Output {
        &self.locals[index]
    }
}

impl std::ops::Index<&BasicBlockId> for MirStore {
    type Output = RefCell<BasicBlock>;

    fn index(&self, index: &BasicBlockId) -> &Self::Output {
        &self.basic_blocks[index]
    }
}

impl std::ops::Index<&MirFunctionId> for MirStore {
    type Output = RefCell<MirFunction>;

    fn index(&self, index: &MirFunctionId) -> &Self::Output {
        &self.functions[index]
    }
}

// ─────────────────────────────────────────────────────────────
// Convenience conversions
// ─────────────────────────────────────────────────────────────

impl From<MirType> for MirTypeId {
    fn from(ty: MirType) -> Self {
        get_storage(|store| store.store_type(ty))
    }
}

impl From<LocalDecl> for LocalId {
    fn from(local: LocalDecl) -> Self {
        get_storage(|store| store.store_local(local))
    }
}

impl From<BasicBlock> for BasicBlockId {
    fn from(bb: BasicBlock) -> Self {
        get_storage(|store| store.store_basic_block(bb))
    }
}

impl From<MirFunction> for MirFunctionId {
    fn from(func: MirFunction) -> Self {
        get_storage(|store| store.store_function(func))
    }
}
