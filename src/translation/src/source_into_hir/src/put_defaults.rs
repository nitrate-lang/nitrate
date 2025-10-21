use crate::Ast2HirCtx;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;

pub(crate) fn module_put_defaults(module: &mut Module, ctx: &mut Ast2HirCtx, _log: &CompilerLog) {
    module
        .iter_mut()
        .for_each_value_mut(&ctx.store, &mut |_value| {
            // TODO:
        });
}
