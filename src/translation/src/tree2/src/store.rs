use crate::prelude::*;
use append_only_vec::AppendOnlyVec;
use serde::{Deserialize, Serialize};
use std::cell::{Cell, RefCell};
use std::num::NonZeroU32;
use std::ops::Deref;

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

impl_store_mut!(ExprId, Expr, ExprStore);
impl_store_mut!(ItemId, Item, ItemStore);
impl_store_mut!(PatId, Pat, PatStore);
impl_store_mut!(StmtId, Stmt, StmtStore);
impl_store_mut!(TypeId, Ty, TypeStore);

#[derive(Debug)]
pub struct Store {
    exprs: ExprStore,
    items: ItemStore,
    pats: PatStore,
    stmts: StmtStore,
    types: TypeStore,
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
            exprs: ExprStore::new(),
            items: ItemStore::new(),
            pats: PatStore::new(),
            stmts: StmtStore::new(),
            types: TypeStore::new(),
        }
    }

    pub fn store_expr(&self, expr: Expr) -> ExprId {
        using_storage(self, || self.exprs.store(expr))
    }

    pub fn store_item(&self, item: Item) -> ItemId {
        using_storage(self, || self.items.store(item))
    }

    pub fn store_pat(&self, pat: Pat) -> PatId {
        using_storage(self, || self.pats.store(pat))
    }

    pub fn store_stmt(&self, stmt: Stmt) -> StmtId {
        using_storage(self, || self.stmts.store(stmt))
    }

    pub fn store_type(&self, ty: Ty) -> TypeId {
        using_storage(self, || self.types.store(ty))
    }

    pub fn reset(&mut self) {
        self.exprs.reset();
        self.items.reset();
        self.pats.reset();
        self.stmts.reset();
        self.types.reset();
    }

    pub fn shrink_to_fit(&mut self) {
        self.exprs.shrink_to_fit();
        self.items.shrink_to_fit();
        self.pats.shrink_to_fit();
        self.stmts.shrink_to_fit();
        self.types.shrink_to_fit();
    }
}

impl std::ops::Index<&ExprId> for Store {
    type Output = RefCell<Expr>;

    fn index(&self, index: &ExprId) -> &Self::Output {
        &self.exprs[index]
    }
}

impl std::ops::Index<&ItemId> for Store {
    type Output = RefCell<Item>;

    fn index(&self, index: &ItemId) -> &Self::Output {
        &self.items[index]
    }
}

impl std::ops::Index<&PatId> for Store {
    type Output = RefCell<Pat>;

    fn index(&self, index: &PatId) -> &Self::Output {
        &self.pats[index]
    }
}

impl std::ops::Index<&StmtId> for Store {
    type Output = RefCell<Stmt>;

    fn index(&self, index: &StmtId) -> &Self::Output {
        &self.stmts[index]
    }
}

impl std::ops::Index<&TypeId> for Store {
    type Output = RefCell<Ty>;

    fn index(&self, index: &TypeId) -> &Self::Output {
        &self.types[index]
    }
}
