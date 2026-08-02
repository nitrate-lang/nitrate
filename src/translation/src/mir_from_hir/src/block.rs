use super::LoweringCtx;
use crate::expr;
use nitrate_hir::prelude as hir;
use nitrate_mir::prelude as mir;

/// Lower a sequence of `BlockElement`s (expressions and local declarations) into
/// basic blocks within a `MirFunctionBuilder`.
///
/// This is the primary entry point for lowering a HIR function body or block
/// expression into MIR. Each `BlockElement::Expr` is lowered in order. The last
/// expression in the block determines the block's result operand.
pub fn lower_block_elements(ctx: &mut LoweringCtx, func: &mut mir::MirFunctionBuilder, elements: &[hir::BlockElement]) {
    if elements.is_empty() {
        return;
    }

    for (i, element) in elements.iter().enumerate() {
        let is_last = i == elements.len() - 1;
        match element {
            hir::BlockElement::Expr(value_id) => {
                let operand = expr::lower_value(ctx, func, value_id, is_last);
                if is_last {
                    // Store the result operand for the caller to use
                    ctx.set_value_operand(value_id, operand);
                }
            }
            hir::BlockElement::Local(local_var_id) => {
                let local_var = local_var_id.borrow();
                lower_local_declaration(ctx, func, &local_var);
            }
        }
    }
}

/// Lower a local variable declaration (`let x = ...` or `var x = ...`).
///
/// Creates a new MIR local, lowers the initializer expression, and emits an
/// `Assign` statement. The local is registered in the context's local map
/// for future reference.
fn lower_local_declaration(ctx: &mut LoweringCtx, func: &mut mir::MirFunctionBuilder, local_var: &hir::LocalVariable) {
    let mir_ty = crate::ty::lower_type(&local_var.ty);

    // Create a new local in the MIR function
    let local_id = func.new_temp(mir_ty, local_var.is_mutable);

    // Register in the local map
    ctx.local_map.insert(local_var.name.clone(), local_id.clone());

    // Emit StorageLive
    func.push_storage_live(local_id.clone());

    // Lower the initializer
    let init_value = &local_var.initializer;
    let init_operand = expr::lower_value(ctx, func, init_value, false);

    // Assign the initializer to the local (resolve through a temp if needed)
    let init_rvalue = mir::Rvalue::Use(init_operand);
    func.push_assign(mir::Place::Local(local_id.clone()), init_rvalue);
}

// ─────────────────────────────────────────────────────────────
// Helpers for block-level constructs
// ─────────────────────────────────────────────────────────────

/// Lower an HIR `Block` (block expression `{ ... }`) into MIR.
///
/// Returns the `Operand` containing the block's result, or unit
/// if the block has no result expression.
pub fn lower_block(ctx: &mut LoweringCtx, func: &mut mir::MirFunctionBuilder, block: &hir::Block) -> mir::Operand {
    if block.elements.is_empty() {
        return mir::Operand::Constant(mir::MirLiteral::Unit);
    }

    // Clear any prior value map entries to avoid stale results
    lower_block_elements(ctx, func, &block.elements);

    // The result of the block is the value of the last expression
    if let Some(hir::BlockElement::Expr(last_value)) = block.elements.last() {
        if let Some(operand) = ctx.get_value_operand(last_value) {
            return operand.clone();
        }
    }

    mir::Operand::Constant(mir::MirLiteral::Unit)
}
