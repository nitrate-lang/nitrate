use crate::context::Ast2HirCtx;
use crate::item::lower_module;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_borrow_check::check_function_borrows;
use nitrate_hir_solve2::{SolveError, resolve_function, resolve_global};
use nitrate_tree::ast::{self};
use nitrate_tree_resolve::{resolve_imports, resolve_paths};

pub fn convert_ast_to_hir(
    mut module: ast::Module,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Module, SolveError> {
    resolve_imports(&ctx.import_ctx, &mut module, log);

    let symbol_map = resolve_paths(&mut module, log);
    ctx.ast_symbol_map.extend(symbol_map);

    let mut module = lower_module(module, ctx, log).map_err(|_| SolveError::TypeErrors)?;

    // Pass 1: Type inference and solving.
    // Collect function IDs first to avoid borrow conflict with resolve_function
    let function_ids: Vec<FunctionId> = ctx.tab.functions().cloned().collect();
    for func_id in &function_ids {
        let mut function = func_id.borrow_mut();
        if function.body.is_some() {
            resolve_function(&mut function, &mut ctx.tab, log)?;
        }
    }

    // Collect global IDs first to avoid borrow conflict with resolve_global
    let global_ids: Vec<GlobalVariableId> = ctx.tab.globals().cloned().collect();
    for global_id in &global_ids {
        let mut global = global_id.borrow_mut();
        resolve_global(&mut global, &mut ctx.tab, log)?;
    }

    // Pass 2: Borrow checking.
    // After type inference, we have enough type information to perform
    // borrow checking. This ensures memory safety before code generation.
    for func_id in &function_ids {
        let mut function = func_id.borrow_mut();
        if function.body.is_some() {
            check_function_borrows(&mut function, &ctx.tab, log).map_err(|_| SolveError::TypeErrors)?;
        }
    }

    Ok(module)
}
