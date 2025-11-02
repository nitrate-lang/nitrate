use crate::{TyCtx, TypeResolver};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;

impl TypeResolver for Block {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for Block
    }
}

impl TypeResolver for Value {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: resolution for Value
    }
}
