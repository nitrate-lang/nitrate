use crate::{TyCtx, TypeResolver};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{ExternalFunction, Function, GlobalVariable, Item, Module};

impl TypeResolver for Module {
    fn resolve_type(&mut self, ctx: &mut TyCtx, log: &CompilerLog) {
        for item in &self.items {
            ctx.store[item].borrow_mut().resolve_type(ctx, log);
        }
    }
}

impl TypeResolver for GlobalVariable {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for GlobalVariable
    }
}

impl TypeResolver for ExternalFunction {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for ExternalFunction
    }
}

impl TypeResolver for Function {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for StaticFunction
    }
}

impl TypeResolver for Item {
    fn resolve_type(&mut self, ctx: &mut TyCtx, log: &CompilerLog) {
        match self {
            Item::Module(m) => m.resolve_type(ctx, log),
            Item::GlobalVariable(gv) => gv.resolve_type(ctx, log),
            Item::ExternalFunction(ef) => ef.resolve_type(ctx, log),
            Item::StaticFunction(sf) => sf.resolve_type(ctx, log),
        }
    }
}
