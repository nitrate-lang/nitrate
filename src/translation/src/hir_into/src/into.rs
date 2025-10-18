use crate::{lower::Ast2Hir, passover::passover_module};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{self, HirCtx};
use nitrate_parsetree::ast;

pub fn astmod2hir(
    mut module: ast::Module,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<hir::Module, ()> {
    passover_module(&mut module, ctx, log);
    let hir_module = module.ast2hir(ctx, log);

    println!("ctx = {:#?}", ctx);
    hir_module
}
