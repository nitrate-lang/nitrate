use crate::{Ast2HirCtx, lower::Ast2Hir, put_defaults::module_put_defaults};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_tree::ast::{self};
use nitrate_tree_resolve::{resolve_imports, resolve_paths};

pub fn convert_ast_to_hir(
    mut module: ast::Module,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Module, ()> {
    resolve_imports(&ctx.import_ctx, &mut module, log);

    let symbol_map = resolve_paths(&mut module, log);
    ctx.ast_symbol_map.extend(symbol_map);

    let mut module = module.ast2hir(ctx, log)?;
    module_put_defaults(&mut module, ctx, log);

    Ok(module)
}
