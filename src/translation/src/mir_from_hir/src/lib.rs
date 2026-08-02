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

    /// Stack of if-else merge points for block expression results.
    /// Each entry is the block where the phi/merge happens.
    merge_point_stack: Vec<mir::BasicBlockId>,

    /// Map from HIR value IDs to their lowered MIR place (for symbol references).
    value_place_map: HashMap<usize, mir::Place>,
}

impl<'b> LoweringCtx<'b> {
    fn new(symbol_tab: &'b hir::SymbolTab) -> Self {
        Self {
            symbol_tab,
            local_map: HashMap::new(),
            loop_stack: Vec::new(),
            merge_point_stack: Vec::new(),
            value_place_map: HashMap::new(),
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

    fn push_merge_point(&mut self, bb: mir::BasicBlockId) {
        self.merge_point_stack.push(bb);
    }

    fn pop_merge_point(&mut self) {
        self.merge_point_stack.pop();
    }

    fn merge_point(&self) -> Option<&mir::BasicBlockId> {
        self.merge_point_stack.last()
    }

    fn map_value_place(&mut self, hir_value: &hir::ValueId, mir_place: mir::Place) {
        self.value_place_map.insert(hir_value.as_usize(), mir_place);
    }

    fn get_value_place(&self, hir_value: &hir::ValueId) -> Option<&mir::Place> {
        self.value_place_map.get(&hir_value.as_usize())
    }
}

// ─────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────

/// Lower a validated HIR module into a MIR module.
///
/// This function converts the high-level, AST-like HIR representation
/// into a control-flow-graph-based MIR representation with basic blocks,
/// flat statements, and explicit terminators.
///
/// # Design
///
/// The lowering process performs the following transformations:
/// 1. Converts HIR `Value` trees into flat MIR `Statement` + `Rvalue` sequences
/// 2. Transforms HIR control flow (`If`, `While`, `Loop`, `Break`, `Continue`,
///    `Return`) into basic blocks with `Terminator` instructions
/// 3. Monomorphizes all types into fully concrete `MirType` values
/// 4. Converts HIR locals/variables into MIR `LocalDecl` entries with SSA form
/// 5. Translates HIR expressions (that are non-branching) into DAG-like Rvalues
///    within basic blocks
///
/// After this lowering, the HIR `Store` and its associated memory can be
/// dropped (freeing all HIR data) since all relevant information has been
/// transferred to the MIR representation.
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

    // Lower all top-level items that are functions
    for item in &hir_module.items {
        match item {
            hir::Item::Function(func_id) => {
                let func = func_id.borrow();
                // Only lower functions that have a body
                if func.body.is_some() {
                    lower_function(&mut builder, &*func, symbol_tab);
                }
            }
            _ => {
                // Structs, enums, type aliases, traits, globals — nothing to lower
                // to MIR at the module level. They are referenced by functions.
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

        func_builder.start_block();
        block::lower_block_elements(&mut ctx, &mut func_builder, body);
        func_builder.ret(None);
    }

    func_builder.finish_function();
}
