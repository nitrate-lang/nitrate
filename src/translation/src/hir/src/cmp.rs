use crate::{BlockId, ItemId, Store, SymbolId, ValueId};
use std::cell::Cell;

thread_local! {
    static TLS_STORE: Cell<Option<*const Store>> = Cell::new(None);
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

impl PartialEq for ItemId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ItemId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for ItemId {}

impl std::hash::Hash for ItemId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ItemId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for SymbolId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to SymbolId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for SymbolId {}

impl std::hash::Hash for SymbolId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to SymbolId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for ValueId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ValueId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for ValueId {}

impl std::hash::Hash for ValueId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ValueId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for BlockId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to BlockId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for BlockId {}

impl std::hash::Hash for BlockId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to BlockId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}
