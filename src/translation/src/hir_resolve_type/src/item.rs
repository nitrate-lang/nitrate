use crate::{TyCtx, TypeResolver};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;

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

impl TypeResolver for TypeAliasDef {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for TypeAlias
    }
}

impl TypeResolver for StructDef {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for StructDef
    }
}

impl TypeResolver for EnumDef {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for EnumDef
    }
}

impl TypeResolver for Item {
    fn resolve_type(&mut self, ctx: &mut TyCtx, log: &CompilerLog) {
        match self as &Item {
            Item::Module(id) => ctx.store[id].borrow_mut().resolve_type(ctx, log),
            Item::GlobalVariable(id) => ctx.store[id].borrow_mut().resolve_type(ctx, log),
            Item::Function(id) => ctx.store[id].borrow_mut().resolve_type(ctx, log),
            Item::TypeAliasDef(id) => ctx.store[id].borrow_mut().resolve_type(ctx, log),
            Item::StructDef(id) => ctx.store[id].borrow_mut().resolve_type(ctx, log),
            Item::EnumDef(id) => ctx.store[id].borrow_mut().resolve_type(ctx, log),
        }
    }
}
