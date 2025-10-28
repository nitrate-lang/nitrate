use crate::{Ast2HirCtx, lower::Ast2Hir, put_defaults::module_put_defaults};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::Module;
use nitrate_source::ast;

pub fn convert_ast_to_hir(
    module: ast::Module,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Module, ()> {
    let mut module = module.ast2hir(ctx, log)?;
    module_put_defaults(&mut module, ctx, log);
    Ok(module)
}
