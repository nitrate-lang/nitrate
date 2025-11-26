use crate::{Ast2HirCtx, item::ast_module2hir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_polish::TyCtx;
use nitrate_tree::ast::{self};
use nitrate_tree_resolve::{resolve_imports, resolve_paths};

pub fn convert_ast_to_hir(mut module: ast::Module, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    resolve_imports(&ctx.import_ctx, &mut module, log);

    let symbol_map = resolve_paths(&mut module, log);
    ctx.ast_symbol_map.extend(symbol_map);

    let mut module = ast_module2hir(module, ctx, log)?;

    // Perform modified Hindley-Milner type inference on functions and global variables
    for item in &mut module.items {
        if let Item::Function(func_id) = item {
            let mut function = func_id.borrow_mut();
            if function.body.is_some() {
                TyCtx::new(ctx.ptr_size).resolve_function(&mut function, log)?;
            }
        } else if let Item::GlobalVariable(global_id) = item {
            let mut global = global_id.borrow_mut();
            TyCtx::new(ctx.ptr_size).resolve_global(&mut global, log)?;
        }
    }

    Ok(module)
}
