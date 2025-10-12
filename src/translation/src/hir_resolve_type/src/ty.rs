use crate::{TyCtx, TypeResolver};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::Type;

impl TypeResolver for Type {
    fn resolve_type(&mut self, _ctx: &mut TyCtx, _log: &CompilerLog) {
        // TODO: Implement type resolution for Type
    }
}
