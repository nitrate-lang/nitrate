use crate::hindley_milner::HindleyMilner;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{Function, GlobalVariable, TypeId, Value, ValueId};

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

    pub fn resolve_function(&mut self, function: &mut Function, log: &CompilerLog) {
        let mut hm = HindleyMilner::new();
        hm.solve_function(function, log);
    }

    pub fn resolve_global(&mut self, global: &mut GlobalVariable, log: &CompilerLog) {
        let mut hm = HindleyMilner::new();
        hm.solve_global_variable(global, log);
    }
}
