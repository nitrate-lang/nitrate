use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Store, TypeId};

pub struct TyCtx<'store> {
    pub(crate) store: &'store Store,
    ok: bool,
}

impl<'store> TyCtx<'store> {
    pub fn new(store: &'store Store) -> Self {
        Self { store, ok: true }
    }

    pub(crate) fn set_failed_bit(&mut self) {
        self.ok = false;
    }

    pub fn ok(&self) -> bool {
        self.ok
    }
}

pub trait TypeResolver {
    fn resolve_type(&mut self, ctx: &mut TyCtx, log: &CompilerLog);
}
