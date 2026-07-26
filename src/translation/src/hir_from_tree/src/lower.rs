use crate::context::Ast2HirCtx;
use crate::item::lower_module;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_solve::{resolve_function, resolve_global};
use nitrate_tree::ast::{self};
use nitrate_tree_resolve::{resolve_imports, resolve_paths};

pub fn convert_ast_to_hir(mut module: ast::Module, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    resolve_imports(&ctx.import_ctx, &mut module, log);

    let symbol_map = resolve_paths(&mut module, log);
    ctx.ast_symbol_map.extend(symbol_map);

    let mut module = lower_module(module, ctx, log)?;

    // Perform type inference and monomorphization on functions and global variables
    for item in &mut module.items {
        if let Item::Function(func_id) = item {
            let mut function = func_id.borrow_mut();
            if function.body.is_some() {
                resolve_function(&mut function, &mut ctx.tab, log)?;
            }
        } else if let Item::GlobalVariable(global_id) = item {
            let mut global = global_id.borrow_mut();
            resolve_global(&mut global, &mut ctx.tab, log)?;
        }
    }

    Ok(module)
}
