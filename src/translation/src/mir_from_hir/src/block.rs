use super::LoweringCtx;
use crate::expr;
use nitrate_hir::prelude as hir;
use nitrate_mir::prelude as mir;

/// Lower a sequence of `BlockElement`s (expressions and local declarations) into
/// basic blocks within a `MirFunctionBuilder`.
///
/// This is the primary entry point for lowering a HIR function body or block
/// expression into MIR. Each `BlockElement::Expr` is lowered in order. The last
/// expression in the block determines the block's result value (if any).
///
/// Panics if the block is empty (callers should ensure blocks have at least one element).
pub fn lower_block_elements(ctx: &mut LoweringCtx, func: &mut mir::MirFunctionBuilder, elements: &[hir::BlockElement]) {
    if elements.is_empty() {
        return;
    }

    for (i, element) in elements.iter().enumerate() {
        let is_last = i == elements.len() - 1;
        match element {
            hir::BlockElement::Expr(value_id) => {
                expr::lower_value(ctx, func, value_id, is_last);
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
    let mir_ty = crate::ty::lower_type(&local_var.ty, func.store());

    // Create a new local in the MIR function
    let local_id = func.new_temp(mir_ty, local_var.is_mutable);

    // Register in the local map
    ctx.local_map.insert(local_var.name.clone(), local_id.clone());

    // Emit StorageLive
    func.push_storage_live(local_id.clone());

    // Lower the initializer
    let init_value = &local_var.initializer;
    let init_operand = expr::lower_value(ctx, func, init_value, false);

    // Assign the initializer to the local
    func.push_assign(mir::Place::Local(local_id.clone()), mir::Rvalue::Use(init_operand));
}

// ─────────────────────────────────────────────────────────────
// Helpers for block-level constructs
// ─────────────────────────────────────────────────────────────

/// Lower an HIR `Block` (block expression `{ ... }`) into MIR.
///
/// Each `Block` contains a sequence of `BlockElement`s. The last expression
/// provides the block's result value.
///
/// Returns the `Place` containing the block's result, or a unit place
/// if the block has no result expression.
pub fn lower_block(ctx: &mut LoweringCtx, func: &mut mir::MirFunctionBuilder, block: &hir::Block) -> mir::Operand {
    // Blocks are lowered inline — the elements are just appended to the
    // current basic block sequence. Complex scoping (nested blocks that
    // need their own locals) would require StorageLive/StorageDead markers,
    // but for now we lower them flat.

    if block.elements.is_empty() {
        return mir::Operand::Constant(mir::MirLiteral::Unit);
    }

    lower_block_elements(ctx, func, &block.elements);

    // The result of the block is the value of the last expression
    if let Some(hir::BlockElement::Expr(last_value)) = block.elements.last() {
        let _last = last_value.borrow();
        // If the last expression was already lowered with its result placed
        // somewhere, return that. Otherwise return unit.
        if let Some(place) = ctx.get_value_place(last_value) {
            return mir::Operand::Copy(place.clone());
        }
    }

    mir::Operand::Constant(mir::MirLiteral::Unit)
}
