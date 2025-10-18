use crate::{lower::Ast2Hir, passover::passover_item};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{self, HirCtx};
use nitrate_parsetree::ast;

pub fn astmod2hir(
    module: ast::Module,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<hir::Module, ()> {
    for item in &module.items {
        passover_item(item, ctx, log);
    }

    module.ast2hir(ctx, log)
}
