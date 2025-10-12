use crate::{TyCtx, TypeResolver};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{ExternalFunction, GlobalVariable, Item, Module, StaticFunction};

impl TypeResolver for Module {
    fn resolve_type(&mut self, _ctx: &TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for Module
    }
}

impl TypeResolver for GlobalVariable {
    fn resolve_type(&mut self, _ctx: &TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for GlobalVariable
    }
}

impl TypeResolver for ExternalFunction {
    fn resolve_type(&mut self, _ctx: &TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for ExternalFunction
    }
}

impl TypeResolver for StaticFunction {
    fn resolve_type(&mut self, _ctx: &TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for StaticFunction
    }
}

impl TypeResolver for Item {
    fn resolve_type(&mut self, _ctx: &TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for Item
    }
}
