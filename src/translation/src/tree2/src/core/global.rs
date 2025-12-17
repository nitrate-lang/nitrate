use nitrate_diagnosis::FileId;
use std::cell::Cell;

pub struct GlobalSource<'a> {
    pub full_source: &'a [u8],
    pub fileid: Option<FileId>,
}

thread_local! {
    static TLS_STORE: Cell<Option<*const GlobalSource>> = const { Cell::new(None) };
}

pub fn using_source<R>(state: &GlobalSource, f: impl FnOnce() -> R) -> R {
    TLS_STORE.with(|tls| {
        // Safety: The callback `f` never outruns the lifetime of `state`
        // because `state` is on our stack.
        let transmuted: &GlobalSource = unsafe { std::mem::transmute(state) };

        let old = tls.take();
        tls.set(Some(transmuted));
        let result = f();
        tls.set(old); // Ensure panic when misused
        result
    })
}

pub fn get_source<R>(f: impl FnOnce(&GlobalSource) -> R) -> R {
    TLS_STORE.with(|tls| {
        let state_ptr = tls
            .get()
            .expect("No GlobalSource found in TLS. Did you forget to call using_source?");

        // Safety: When the above is Some, our call stack must always contain a
        // `using_source` call that provided the `GlobalSource` reference.
        // The state reference will outlive the `using_source` call,
        // and we are guaranteed to be inside such a call here.
        let store = unsafe { &*state_ptr };
        f(store)
    })
}
