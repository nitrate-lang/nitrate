mod block;
mod expr;
mod ty;

use nitrate_hir::prelude as hir;
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;
use std::collections::HashMap;

// ─────────────────────────────────────────────────────────────
// Lowering context — shared state during HIR→MIR lowering
// ─────────────────────────────────────────────────────────────

/// Context shared during the lowering of a single HIR module to MIR.
///
/// The context carries symbol table, local variable mappings, and control-flow
/// state. The `MirFunctionBuilder` is passed separately to lowering functions
/// to avoid double-borrow issues.
struct LoweringCtx<'b> {
    symbol_tab: &'b hir::SymbolTab,

    /// Map from HIR local variable name → MIR LocalId
    local_map: HashMap<NString, mir::LocalId>,

    /// Stack of loop labels for break/continue resolution.
    /// Each entry is (continue_target, break_target).
    loop_stack: Vec<(mir::BasicBlockId, mir::BasicBlockId)>,

    /// Map from HIR value IDs to their lowered MIR Operand (for block results).
    value_operand_map: HashMap<usize, mir::Operand>,
}

impl<'b> LoweringCtx<'b> {
    fn new(symbol_tab: &'b hir::SymbolTab) -> Self {
        Self {
            symbol_tab,
            local_map: HashMap::new(),
            loop_stack: Vec::new(),
            value_operand_map: HashMap::new(),
        }
    }

    fn push_loop(&mut self, continue_target: mir::BasicBlockId, break_target: mir::BasicBlockId) {
        self.loop_stack.push((continue_target, break_target));
    }

    fn pop_loop(&mut self) {
        self.loop_stack.pop();
    }

    fn loop_targets(&self) -> Option<&(mir::BasicBlockId, mir::BasicBlockId)> {
        self.loop_stack.last()
    }

    fn set_value_operand(&mut self, hir_value: &hir::ValueId, operand: mir::Operand) {
        self.value_operand_map.insert(hir_value.as_usize(), operand);
    }

    fn get_value_operand(&self, hir_value: &hir::ValueId) -> Option<&mir::Operand> {
        self.value_operand_map.get(&hir_value.as_usize())
    }
}

// ─────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────

/// Lower a validated HIR module into a MIR module.
///
/// This function converts the high-level, AST-like HIR representation
/// into a control-flow-graph-based MIR representation with basic blocks,
/// flat statements, and explicit terminators. Block arguments are used
/// to carry values across control flow edges instead of phi nodes.
///
/// # Arguments
///
/// * `hir_module` - The validated HIR module (from `nitrate_hir_validate`)
/// * `symbol_tab` - The HIR symbol table for name resolution
///
/// # Returns
///
/// A fully constructed `MirModule` containing all lowered functions.
#[must_use]
pub fn lower_hir_to_mir(hir_module: &hir::Module, symbol_tab: &hir::SymbolTab) -> mir::MirModule {
    let ptr_size = match symbol_tab.arch_ptr_size() {
        hir::PtrSize::U32 => mir::PtrSize::U32,
        hir::PtrSize::U64 => mir::PtrSize::U64,
    };

    let mut builder = mir::MirBuilder::new();

    // Lower all functions that have a body, including impl methods
    // which are registered in the symbol table but not in module.items.
    let mut lowered: std::collections::HashSet<usize> = std::collections::HashSet::new();
    for func_id in symbol_tab.functions() {
        let func = func_id.borrow();
        if func.body.is_some() && lowered.insert(func_id.as_usize()) {
            lower_function(&mut builder, &*func, symbol_tab);
        }
    }

    // Also lower top-level function items (includes extern declarations)
    for item in &hir_module.items {
        if let hir::Item::Function(func_id) = item {
            let func = func_id.borrow();
            if lowered.insert(func_id.as_usize()) {
                lower_function(&mut builder, &*func, symbol_tab);
            }
        }
    }

    builder.build_module(ptr_size)
}

// ─────────────────────────────────────────────────────────────
// Function lowering
// ─────────────────────────────────────────────────────────────

/// Lower a single HIR Function into a MIR MirFunction.
fn lower_function(builder: &mut mir::MirBuilder, func: &hir::Function, symbol_tab: &hir::SymbolTab) {
    let name = func.mangled_name.clone().unwrap_or_else(|| func.name.clone());

    // Pre-compute all types before borrowing builder for the function builder
    let return_ty = ty::lower_type(&func.return_type);
    let param_types: Vec<_> = func
        .params
        .iter()
        .map(|pid| {
            let param = pid.borrow();
            (param.name.clone(), ty::lower_type(&param.ty), param.is_mutable)
        })
        .collect();

    let mut func_builder = builder.start_function(name, return_ty);

    // Propagate C-variadic flag
    use nitrate_hir::FunctionAttribute;
    if func.attributes.contains(&FunctionAttribute::CVariadic) {
        func_builder.set_c_variadic();
    }

    // Lower parameters
    let mut param_locals: Vec<(NString, mir::LocalId)> = Vec::new();
    for (name, mir_ty, is_mutable) in &param_types {
        let local_id = func_builder.add_param(name.clone(), mir_ty.clone(), *is_mutable);
        param_locals.push((name.clone(), local_id));
    }

    // Lower the function body
    if let Some(ref body) = func.body {
        let mut ctx = LoweringCtx::new(symbol_tab);
        for (name, local_id) in &param_locals {
            ctx.local_map.insert(name.clone(), local_id.clone());
        }

        // Create the entry block
        func_builder.create_block();
        block::lower_block_elements(&mut ctx, &mut func_builder, body);
        // If the body didn't explicitly return, add an implicit return
        if func_builder.current_block.is_some() {
            func_builder.ret(None);
        }
    }

    func_builder.finish_function();
}
