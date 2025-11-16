use std::ops::Range;

use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Function, Store};

pub struct TyCtx<'store> {
    pub(crate) store: &'store Store,
    ok: bool,
}

impl<'store> TyCtx<'store> {
    pub fn new(store: &'store Store) -> Self {
        Self { store, ok: true }
    }

    pub fn set_failed_bit(&mut self) {
        self.ok = false;
    }

    pub fn ok(&self) -> bool {
        self.ok
    }
}

pub fn resolve_types(function: &mut Function, ctx: &mut TyCtx, log: &CompilerLog) {
    // TODO: type resolution/inference/checking logic
}
