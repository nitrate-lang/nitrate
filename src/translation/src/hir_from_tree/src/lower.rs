use crate::context::Ast2HirCtx;
use crate::item::lower_module;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_solve::{SolveError, resolve_all_functions, resolve_global};
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

    // Pass 1: Type inference and solving. `resolve_all_functions` runs to a
    // fixed point: solving one function can monomorphize generic callees,
    // producing new concrete functions that must themselves be solved, and
    // later rounds re-normalize earlier functions' types once the mono structs
    // they reference have been created.
    resolve_all_functions(&mut ctx.tab, log)?;

    // Collect global IDs first to avoid borrow conflict with resolve_global
    let global_ids: Vec<GlobalVariableId> = ctx.tab.globals().cloned().collect();
    for global_id in &global_ids {
        let mut global = global_id.borrow_mut();
        resolve_global(&mut global, &mut ctx.tab, log)?;
    }

    // Note: borrow checking no longer runs on HIR. Memory-safety enforcement
    // was moved to the MIR borrow checker (`nitrate_mir_borrow_check`), which
    // runs after MIR lowering (see the pipeline's `lower_mir` stage). The MIR
    // checker can reason about liveness-based NLL borrow regions, explicit
    // temporaries, two-phase borrows, and control flow — none of which are
    // available on the tree-structured HIR.

    Ok(module)
}
