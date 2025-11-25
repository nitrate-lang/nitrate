use crate::hindley_milner::HindleyMilner;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Function, GlobalVariable, PtrSize};

pub struct TyCtx {
    ok: bool,
    ptr_size: PtrSize,
}

impl TyCtx {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self { ok: true, ptr_size }
    }

    pub fn set_failed_bit(&mut self) {
        self.ok = false;
    }

    pub fn ok(&self) -> bool {
        self.ok
    }

    pub fn resolve_function(&mut self, function: &mut Function, log: &CompilerLog) {
        let mut hm = HindleyMilner::new(self.ptr_size);
        hm.solve_function(function, log);
    }

    pub fn resolve_global(&mut self, global: &mut GlobalVariable, log: &CompilerLog) {
        let mut hm = HindleyMilner::new(self.ptr_size);
        hm.solve_global_variable(global, log);
    }
}
