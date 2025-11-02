use crate::{
    BlockId, EnumDefId, FunctionId, GlobalVariableId, LocalVariableId, ModuleId, ParameterId,
    Store, StructDefId, TraitId, TypeAliasDefId, ValueId,
};
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

impl PartialEq for GlobalVariableId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to GlobalVariableId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for GlobalVariableId {}

impl std::hash::Hash for GlobalVariableId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to GlobalVariableId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for LocalVariableId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to LocalVariableId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for LocalVariableId {}

impl std::hash::Hash for LocalVariableId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to LocalVariableId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for ParameterId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ParameterId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for ParameterId {}

impl std::hash::Hash for ParameterId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ParameterId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for FunctionId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to FunctionId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for FunctionId {}

impl std::hash::Hash for FunctionId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to FunctionId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for TraitId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to TraitId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for TraitId {}

impl std::hash::Hash for TraitId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to TraitId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for ModuleId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ModuleId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for ModuleId {}

impl std::hash::Hash for ModuleId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to ModuleId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for TypeAliasDefId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to TypeAliasId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for TypeAliasDefId {}

impl std::hash::Hash for TypeAliasDefId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to TypeAliasId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for StructDefId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to StructDefId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for StructDefId {}

impl std::hash::Hash for StructDefId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to StructDefId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            store[self].borrow().hash(state)
        })
    }
}

impl PartialEq for EnumDefId {
    fn eq(&self, other: &Self) -> bool {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to EnumDefId::eq outside of `using_storage(...)`");

            let store = unsafe { &*store_ptr };
            let lhs = store[self].borrow();
            let rhs = store[other].borrow();
            lhs.eq(&*rhs)
        })
    }
}

impl Eq for EnumDefId {}

impl std::hash::Hash for EnumDefId {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        TLS_STORE.with(|tls| {
            let store_ptr = tls
                .get()
                .expect("call to EnumDefId::eq outside of `using_storage(...)`");

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
