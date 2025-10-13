use crate::{TyCtx, TypeResolver};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{Function, GlobalVariable, Item, Module, Trait};

impl TypeResolver for Module {
    fn resolve_type(&mut self, ctx: &mut TyCtx, log: &CompilerLog) {
        for item in &mut self.items {
            item.resolve_type(ctx, log);
        }
    }
}

impl TypeResolver for GlobalVariable {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for GlobalVariable
    }
}

impl TypeResolver for Function {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for StaticFunction
    }
}

impl TypeResolver for Trait {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for Trait
    }
}

impl TypeResolver for Item {
    fn resolve_type(&mut self, ctx: &mut TyCtx, log: &CompilerLog) {
        match self as &Item {
            Item::Module(m) => ctx.store[m].borrow_mut().resolve_type(ctx, log),
            Item::GlobalVariable(gv) => ctx.store[gv].borrow_mut().resolve_type(ctx, log),
            Item::Function(f) => ctx.store[f].borrow_mut().resolve_type(ctx, log),
            Item::Trait(t) => ctx.store[t].borrow_mut().resolve_type(ctx, log),
        }
    }
}
