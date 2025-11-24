use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Function, GlobalVariable};

pub struct TyCtx {
    ok: bool,
}

impl TyCtx {
    pub fn new() -> Self {
        Self { ok: true }
    }

    pub fn set_failed_bit(&mut self) {
        self.ok = false;
    }

    pub fn ok(&self) -> bool {
        self.ok
    }

    pub fn resolve_function(&self, _function: &mut Function, _log: &CompilerLog) {
        // TODO: type resolution/inference/checking logic
    }

    pub fn resolve_global(&self, _global: &mut GlobalVariable, _log: &CompilerLog) {
        // TODO: type resolution/inference/checking logic
    }
}
