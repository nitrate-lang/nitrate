use crate::{TyCtx, TypeResolver};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    ItemId,
    hir::{ExternalFunction, GlobalVariable, Item, Module, StaticFunction},
};

impl TypeResolver for Module {
    fn resolve_type(&self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for Module
    }
}

impl TypeResolver for GlobalVariable {
    fn resolve_type(&self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for GlobalVariable
    }
}

impl TypeResolver for ExternalFunction {
    fn resolve_type(&self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for ExternalFunction
    }
}

impl TypeResolver for StaticFunction {
    fn resolve_type(&self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for StaticFunction
    }
}

// impl TypeResolver for ItemId {
//     fn resolve_type(&self, ctx: &mut TyCtx, log: &CompilerLog) {
//         let item = &mut ctx.store[self];

//         match item {
//             Item::Module(m) => m.resolve_type(ctx, log),
//             Item::GlobalVariable(gv) => gv.resolve_type(ctx, log),
//             Item::ExternalFunction(ef) => ef.resolve_type(ctx, log),
//             Item::StaticFunction(sf) => sf.resolve_type(ctx, log),
//         }
//     }
// }
