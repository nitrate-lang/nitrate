use crate::func::{LocalDecl, MirFunction, MirFunctionBody};
use crate::operand::{MirBinaryOp, MirLiteral, MirUnaryOp, Operand};
use crate::place::Place;
use crate::rvalue::{AggregateKind, BorrowKind, NullaryOp, Rvalue};
use crate::stmt::{BasicBlock, Statement, Terminator};
use crate::store::{BasicBlockId, LocalId, MirFunctionId, MirTypeId};
use crate::ty::{MirType, PtrSize};
use nitrate_nstring::NString;
use nitrate_tree::SrcPos;
use thin_vec::ThinVec;

// ─────────────────────────────────────────────────────────────
// MirBuilder — top-level module builder
// ─────────────────────────────────────────────────────────────

/// A builder for constructing MIR modules and functions.
#[derive(Debug)]
pub struct MirBuilder {
    /// Accumulated function IDs for the final module.
    functions: ThinVec<MirFunctionId>,
    /// String literal globals: (name, byte_data).
    pub string_globals: ThinVec<(NString, ThinVec<u8>)>,
    /// Counter for generating unique global names for string literals.
    string_global_counter: u64,
}

impl MirBuilder {
    #[must_use]
    pub fn new() -> Self {
        Self {
            functions: ThinVec::new(),
            string_globals: ThinVec::new(),
            string_global_counter: 0,
        }
    }

    /// Begin constructing a new function. Returns a per-function builder.
    pub fn start_function(&mut self, name: NString, return_ty: MirTypeId) -> MirFunctionBuilder<'_> {
        MirFunctionBuilder {
            builder: self,
            name,
            return_ty,
            params: ThinVec::new(),
            locals: ThinVec::new(),
            local_ids: ThinVec::new(),
            arg_count: 0,
            blocks: ThinVec::new(),
            entry_block: None,
            is_c_variadic: false,
            current_block: None,
            current_block_idx: None,
            current_span: None,
            block_spans: Vec::new(),
        }
    }

    /// Insert a pre-built MirFunctionId into the module's function list.
    pub fn add_function(&mut self, func_id: MirFunctionId) {
        self.functions.push(func_id);
    }

    /// Register a string literal as a global constant, returning a unique
    /// name that can be used as `Place::Static(name)`.
    pub fn register_string_global(&mut self, data: ThinVec<u8>) -> NString {
        let id = self.string_global_counter;
        self.string_global_counter += 1;
        let name: NString = format!("__nitrate_str_{}", id).into();
        self.string_globals.push((name.clone(), data));
        name
    }

    /// Build the final `MirModule` from all accumulated functions.
    pub fn build_module(self, ptr_size: PtrSize) -> MirModule {
        MirModule {
            functions: self.functions,
            globals: ThinVec::new(),
            string_globals: self.string_globals,
            ptr_size,
        }
    }

    /// Convenience: intern a type and return its `MirTypeId`.
    pub fn intern_type(&self, ty: MirType) -> MirTypeId {
        ty.into()
    }
}

// ─────────────────────────────────────────────────────────────
// MirFunctionBuilder — per-function builder
// ─────────────────────────────────────────────────────────────

/// Result of creating a new basic block.
#[derive(Debug)]
pub struct NewBlock {
    /// The newly created basic block's ID.
    pub block: BasicBlockId,
    /// Locals created for each block argument (one per type in `args`).
    /// These locals are SSA values that get their values from predecessor
    /// terminators' block argument lists.
    pub arg_locals: ThinVec<LocalId>,
}

/// A builder for a single MIR function body.
///
/// Holds all data locally until `finish_function()` commits it to the
/// `MirStore` and returns a `MirFunctionId`.
///
/// # Block Arguments Usage
///
/// Call `create_block(types)` to create a block with formal parameters.
/// The returned `arg_locals` are fresh SSA locals that will receive values
/// from predecessor edges. When branching to this block, use the `_with_args`
/// terminators to pass the actual operand values.
///
/// ```ignore
/// let merge = func.create_block(&[int_ty])?;
/// // merge.arg_locals[0] is a Local that will hold the merged value
///
/// // In predecessors:
/// func.goto_with_args(merge.block, &[some_operand]);
///
/// // Use merge.arg_locals[0] in merge block:
/// func.push_assign(dest, mir::Rvalue::Use(mir::Operand::Copy(mir::Place::Local(merge.arg_locals[0].clone()))));
/// ```
#[derive(Debug)]
pub struct MirFunctionBuilder<'b> {
    builder: &'b mut MirBuilder,
    name: NString,
    return_ty: MirTypeId,
    params: ThinVec<LocalId>,
    locals: ThinVec<LocalDecl>,
    arg_count: u32,
    blocks: ThinVec<BasicBlockId>,
    entry_block: Option<BasicBlockId>,
    local_ids: ThinVec<LocalId>,
    is_c_variadic: bool,

    /// The basic block currently being constructed (if any).
    pub current_block: Option<BasicBlockId>,

    /// Positional index of the block currently being constructed, in lock-step
    /// with `current_block`.
    current_block_idx: Option<usize>,

    /// Source position to attach to the next pushed statement/terminator.
    /// Consumed (reset to `None`) by `push_stmt` and `set_terminator`.
    current_span: Option<SrcPos>,

    /// Per-block source map, parallel to `blocks`: `block_spans[b][s]` is the
    /// position of block `b`'s `s`-th statement; the final entry of each
    /// inner vector is the block's terminator position.
    block_spans: Vec<Vec<Option<SrcPos>>>,
}

impl<'b> MirFunctionBuilder<'b> {
    // ── Parameters and locals ────────────────────────────────

    /// Register a parameter local and return its `LocalId`.
    ///
    /// Parameters must be added before any blocks are created.
    /// They are stored as the first N locals.
    pub fn add_param(&mut self, _name: NString, ty: MirTypeId, mutable: bool) -> LocalId {
        let local = LocalDecl { ty, mutable };
        let id: LocalId = local.into();
        self.params.push(id.clone());
        self.arg_count += 1;
        self.locals.push(LocalDecl { ty, mutable });
        self.local_ids.push(id.clone());
        id
    }

    /// Mark this function as C-variadic (uses `...` in params).
    pub fn set_c_variadic(&mut self) -> &mut Self {
        self.is_c_variadic = true;
        self
    }

    /// Create a new temporary local and return its `LocalId`.
    ///
    /// Temporary locals are used for intermediate computation results
    /// (e.g., the result of a binary operation before it is assigned
    /// to its final destination).
    pub fn new_temp(&mut self, ty: MirTypeId, mutable: bool) -> LocalId {
        let local = LocalDecl { ty, mutable };
        let id: LocalId = local.into();
        self.locals.push(LocalDecl { ty, mutable });
        self.local_ids.push(id.clone());
        id
    }

    // ── Block management ─────────────────────────────────────

    /// Create a new basic block with no block arguments and make it the
    /// current block. If a previous block existed, it MUST have a terminator
    /// already set.
    ///
    /// The first block created becomes the entry block.
    pub fn create_block(&mut self) -> BasicBlockId {
        let new_block = self.create_block_with_args(&[]);
        new_block.block
    }

    /// Create a new basic block with the given argument types.
    ///
    /// For each argument type, a fresh local is created. The returned
    /// `arg_locals` are the locals that will receive block argument values
    /// from predecessor edges.
    ///
    /// The first block created becomes the entry block.
    pub fn create_block_with_args(&mut self, arg_types: &[MirTypeId]) -> NewBlock {
        // Finalize any previous current block
        self.current_block = None;
        self.current_block_idx = None;

        // Create locals for block arguments
        let mut arg_locals: ThinVec<LocalId> = ThinVec::new();
        for ty in arg_types.iter() {
            let local_id = self.new_temp(ty.clone(), false);
            arg_locals.push(local_id);
        }

        // Build the block
        let bb = BasicBlock {
            statements: ThinVec::new(),
            terminator: Terminator::Unreachable,
            args: arg_types.iter().cloned().collect(),
            arg_local_ids: arg_locals.clone(),
        };
        let bb_id: BasicBlockId = bb.into();

        // First block created becomes the entry block
        if self.entry_block.is_none() {
            self.entry_block = Some(bb_id.clone());
        }

        self.blocks.push(bb_id.clone());
        self.block_spans.push(Vec::new());
        self.current_block = Some(bb_id.clone());
        self.current_block_idx = Some(self.blocks.len() - 1);

        NewBlock {
            block: bb_id,
            arg_locals,
        }
    }

    /// Reserve a new basic block without making it current.
    ///
    /// The block is registered and can be referenced in terminators immediately,
    /// but the current block remains unchanged. Use `switch_to_block` later
    /// to begin adding statements to the reserved block.
    ///
    /// The first block reserved becomes the entry block if no entry block
    /// has been set yet.
    pub fn reserve_block(&mut self) -> BasicBlockId {
        self.reserve_block_with_args(&[]).block
    }

    /// Reserve a new basic block with argument types, without making it current.
    ///
    /// Returns the created block's ID and its argument locals. The current
    /// block is unchanged.
    pub fn reserve_block_with_args(&mut self, arg_types: &[MirTypeId]) -> NewBlock {
        // Create locals for block arguments
        let mut arg_locals: ThinVec<LocalId> = ThinVec::new();
        for ty in arg_types.iter() {
            let local_id = self.new_temp(ty.clone(), false);
            arg_locals.push(local_id);
        }

        // Build the block
        let bb = BasicBlock {
            statements: ThinVec::new(),
            terminator: Terminator::Unreachable,
            args: arg_types.iter().cloned().collect(),
            arg_local_ids: arg_locals.clone(),
        };
        let bb_id: BasicBlockId = bb.into();

        // First block reserved/created becomes the entry block
        if self.entry_block.is_none() {
            self.entry_block = Some(bb_id.clone());
        }

        self.blocks.push(bb_id.clone());
        self.block_spans.push(Vec::new());
        // NOTE: current_block is NOT changed — caller must use switch_to_block

        NewBlock {
            block: bb_id,
            arg_locals,
        }
    }

    /// Switch to a previously reserved block, making it the current block.
    ///
    /// All subsequent `push_stmt` / `push_assign` / terminator calls will
    /// operate on this block.
    pub fn switch_to_block(&mut self, block: BasicBlockId) -> &mut Self {
        self.current_block = Some(block.clone());
        self.current_block_idx = self.blocks.iter().position(|b| *b == block);
        self
    }

    /// Set the source position attached to the next pushed statement or
    /// terminator. Empty positions (no file / zero offset) are ignored.
    pub fn set_current_span(&mut self, span: Option<SrcPos>) -> &mut Self {
        self.current_span = span.filter(|s| !s.is_empty());
        self
    }

    // ── Statements ───────────────────────────────────────────

    /// Push a statement into the current block.
    /// Panics if no current block exists (call `create_block` first).
    pub fn push_stmt(&mut self, stmt: Statement) -> &mut Self {
        let bb_id = self
            .current_block
            .as_ref()
            .cloned()
            .expect("push_stmt called with no current block — call create_block first");
        let mut borrowed = bb_id.borrow_mut();
        borrowed.statements.push(stmt.clone());

        // Record the statement's source position in the per-block map.
        if let Some(idx) = self.current_block_idx {
            if let Some(spans) = self.block_spans.get_mut(idx) {
                spans.push(self.current_span);
            }
        }
        self.current_span = None;
        self
    }

    /// Shorthand: push an `Assign` statement.
    pub fn push_assign(&mut self, place: Place, rvalue: Rvalue) -> &mut Self {
        self.push_stmt(Statement::Assign(place, rvalue))
    }

    /// Shorthand: push a `StorageLive` statement.
    pub fn push_storage_live(&mut self, local: LocalId) -> &mut Self {
        self.push_stmt(Statement::StorageLive(local))
    }

    /// Shorthand: push a `StorageDead` statement.
    pub fn push_storage_dead(&mut self, local: LocalId) -> &mut Self {
        self.push_stmt(Statement::StorageDead(local))
    }

    /// Shorthand: push a `SetDiscriminant` statement.
    pub fn push_set_discriminant(&mut self, place: Place, variant_index: u32) -> &mut Self {
        self.push_stmt(Statement::SetDiscriminant { place, variant_index })
    }

    // ── Terminators ──────────────────────────────────────────

    /// Set the terminator of the current block. This finalizes the block.
    pub fn set_terminator(&mut self, terminator: Terminator) -> &mut Self {
        let bb_id = self
            .current_block
            .take()
            .expect("set_terminator called with no current block");

        // Record the terminator's source position as the final entry of the
        // block's span map (statement index `n` where `n` is the statement
        // count — the convention used by the borrow checker's `Location`).
        if let Some(idx) = self.current_block_idx.take() {
            if let Some(spans) = self.block_spans.get_mut(idx) {
                spans.push(self.current_span);
            }
        }
        self.current_span = None;

        let mut borrowed = bb_id.borrow_mut();
        borrowed.terminator = terminator;
        self
    }

    /// Unconditional branch to target (no block arguments).
    pub fn goto(&mut self, target: BasicBlockId) -> &mut Self {
        self.set_terminator(Terminator::Goto {
            target,
            args: ThinVec::new(),
        })
    }

    /// Unconditional branch with block arguments.
    pub fn goto_with_args(&mut self, target: BasicBlockId, args: ThinVec<Operand>) -> &mut Self {
        self.set_terminator(Terminator::Goto { target, args })
    }

    /// Conditional branch (no block arguments).
    pub fn if_br(&mut self, condition: Operand, true_target: BasicBlockId, false_target: BasicBlockId) -> &mut Self {
        self.set_terminator(Terminator::If {
            condition,
            true_target,
            true_args: ThinVec::new(),
            false_target,
            false_args: ThinVec::new(),
        })
    }

    /// Conditional branch with block arguments.
    pub fn if_br_with_args(
        &mut self,
        condition: Operand,
        true_target: BasicBlockId,
        true_args: ThinVec<Operand>,
        false_target: BasicBlockId,
        false_args: ThinVec<Operand>,
    ) -> &mut Self {
        self.set_terminator(Terminator::If {
            condition,
            true_target,
            true_args,
            false_target,
            false_args,
        })
    }

    /// Return from the function.
    pub fn ret(&mut self, value: Option<Operand>) -> &mut Self {
        self.set_terminator(Terminator::Return { value })
    }

    /// Diverging call (no return).
    pub fn call(&mut self, callee: Operand, args: ThinVec<Operand>) -> &mut Self {
        self.set_terminator(Terminator::Call {
            callee,
            args,
            destination: None,
            target: None,
            target_args: ThinVec::new(),
        })
    }

    /// Returning call.
    pub fn call_return(
        &mut self,
        callee: Operand,
        args: ThinVec<Operand>,
        destination: Place,
        target: BasicBlockId,
    ) -> &mut Self {
        self.set_terminator(Terminator::Call {
            callee,
            args,
            destination: Some(destination),
            target: Some(target),
            target_args: ThinVec::new(),
        })
    }

    /// Returning call with block arguments on the successor edge.
    pub fn call_return_with_args(
        &mut self,
        callee: Operand,
        args: ThinVec<Operand>,
        destination: Place,
        target: BasicBlockId,
        target_args: ThinVec<Operand>,
    ) -> &mut Self {
        self.set_terminator(Terminator::Call {
            callee,
            args,
            destination: Some(destination),
            target: Some(target),
            target_args,
        })
    }

    /// Unreachable terminator.
    pub fn unreachable(&mut self) -> &mut Self {
        self.set_terminator(Terminator::Unreachable)
    }

    // ── Convenience: build Rvalues ───────────────────────────

    pub fn rv_use(op: Operand) -> Rvalue {
        Rvalue::Use(op)
    }

    pub fn rv_ref(kind: BorrowKind, place: Place) -> Rvalue {
        Rvalue::Ref { region: kind, place }
    }

    pub fn rv_len(place: Place) -> Rvalue {
        Rvalue::Len(place)
    }

    pub fn rv_cast(value: Operand, target_ty: MirTypeId) -> Rvalue {
        Rvalue::Cast { value, target_ty }
    }

    pub fn rv_binary(op: MirBinaryOp, lhs: Operand, rhs: Operand) -> Rvalue {
        Rvalue::BinaryOp { op, lhs, rhs }
    }

    pub fn rv_checked_binary(op: MirBinaryOp, lhs: Operand, rhs: Operand) -> Rvalue {
        Rvalue::CheckedBinaryOp { op, lhs, rhs }
    }

    pub fn rv_unary(op: MirUnaryOp, operand: Operand) -> Rvalue {
        Rvalue::UnaryOp { op, operand }
    }

    pub fn rv_nullary(op: NullaryOp, ty: MirTypeId) -> Rvalue {
        Rvalue::NullaryOp(op, ty)
    }

    pub fn rv_aggregate(kind: AggregateKind, operands: ThinVec<Operand>) -> Rvalue {
        Rvalue::Aggregate(kind, operands)
    }

    // ── Convenience: build Places ────────────────────────────

    pub fn place_local(local: LocalId) -> Place {
        Place::Local(local)
    }

    pub fn place_static(name: NString) -> Place {
        Place::Static(name)
    }

    pub fn place_deref(base: Place) -> Place {
        Place::Deref(Box::new(base))
    }

    pub fn place_field(base: Place, field_name: NString) -> Place {
        Place::Field {
            base: Box::new(base),
            field_name,
        }
    }

    pub fn place_index(base: Place, index: Place) -> Place {
        Place::Index {
            base: Box::new(base),
            index: Box::new(index),
        }
    }

    pub fn place_downcast(base: Place, variant_name: NString) -> Place {
        Place::Downcast {
            base: Box::new(base),
            variant_name,
        }
    }

    // ── Convenience: build Operands ──────────────────────────

    pub fn op_copy(place: Place) -> Operand {
        Operand::Copy(place)
    }

    pub fn op_move(place: Place) -> Operand {
        Operand::Move(place)
    }

    pub fn op_const(lit: MirLiteral) -> Operand {
        Operand::Constant(lit)
    }

    // ── Finish ───────────────────────────────────────────────

    /// Finalize the function: commit all data to the MirStore and return
    /// the `MirFunctionId`.
    pub fn finish_function(self) -> MirFunctionId {
        // Extern functions have no body and thus no blocks/entry block.
        if self.entry_block.is_none() {
            return self.finish_extern_function();
        }

        let entry_block = self.entry_block.unwrap();

        let body = MirFunctionBody {
            locals: self.locals,
            local_ids: self.local_ids,
            entry_block,
            blocks: self.blocks,
            arg_count: self.arg_count,
            statement_spans: self.block_spans,
        };

        let func = MirFunction {
            name: self.name,
            params: self.params,
            return_ty: self.return_ty,
            is_c_variadic: self.is_c_variadic,
            body: Some(body),
        };

        let func_id: MirFunctionId = func.into();

        self.builder.add_function(func_id.clone());
        func_id
    }

    /// Finalize an extern function that has no body (and thus no blocks).
    fn finish_extern_function(self) -> MirFunctionId {
        let func = MirFunction {
            name: self.name,
            params: self.params,
            return_ty: self.return_ty,
            is_c_variadic: self.is_c_variadic,
            body: None,
        };

        let func_id: MirFunctionId = func.into();
        self.builder.add_function(func_id.clone());
        func_id
    }

    /// Convenience: intern a type and return its `MirTypeId`.
    pub fn store_type(&self, ty: MirType) -> MirTypeId {
        ty.into()
    }

    /// Register a string literal as a global constant, returning a unique
    /// name that can be used as `Place::Static(name)`.
    pub fn register_string_global(&mut self, data: ThinVec<u8>) -> NString {
        self.builder.register_string_global(data)
    }
}

// ─────────────────────────────────────────────────────────────
// Re-export MirModule for build_module return type
// ─────────────────────────────────────────────────────────────

use crate::func::MirModule;

#[cfg(test)]
mod tests {
    use super::*;
    use crate::MirType;

    #[test]
    fn builder_records_statement_and_terminator_spans() {
        let store = crate::MirStore::new();
        crate::using_storage(&store, || {
            let mut builder = MirBuilder::new();
            let i32_ty = MirType::I32.into();

            let mut f = builder.start_function("span_map".into(), i32_ty);
            let x = f.new_temp(i32_ty, true);
            f.create_block();

            // Two statements with spans, then a terminator with a span.
            f.set_current_span(Some(SrcPos::new(None, 1, 1, 10)));
            f.push_storage_live(x.clone());
            f.set_current_span(Some(SrcPos::new(None, 1, 5, 14)));
            f.push_assign(Place::Local(x.clone()), Rvalue::Use(Operand::Constant(MirLiteral::I32(7))));
            f.set_current_span(Some(SrcPos::new(None, 1, 9, 18)));
            f.ret(Some(Operand::Copy(Place::Local(x))));
            f.finish_function();

            let module = builder.build_module(PtrSize::U64);
            let func = module.functions[0].borrow().clone();
            let body = func.body.as_ref().expect("function should have a body");

            // One block: two statement entries + one terminator entry.
            assert_eq!(body.blocks.len(), 1);
            assert_eq!(body.statement_spans.len(), 1);
            let spans = &body.statement_spans[0];
            assert_eq!(spans.len(), 3, "statements + terminator entry");
            assert_eq!(spans[0].unwrap().offset.to_u32(), 10);
            assert_eq!(spans[1].unwrap().offset.to_u32(), 14);
            assert_eq!(spans[2].unwrap().offset.to_u32(), 18);

            // Statements pushed without an explicit span record `None`.
            let mut builder = MirBuilder::new();
            let mut f2 = builder.start_function("unspanned".into(), i32_ty);
            let y = f2.new_temp(i32_ty, true);
            f2.create_block();
            f2.push_storage_live(y.clone());
            f2.push_assign(Place::Local(y.clone()), Rvalue::Use(Operand::Constant(MirLiteral::I32(1))));
            f2.ret(None);
            f2.finish_function();

            let module = builder.build_module(PtrSize::U64);
            let func = module.functions[0].borrow().clone();
            let body = func.body.as_ref().expect("function should have a body");
            assert_eq!(body.statement_spans[0].len(), 3);
            assert!(body.statement_spans[0].iter().all(Option::is_none));
        });
    }

    #[test]
    fn empty_spans_are_filtered_out() {
        // `SrcPos::default()` (used by the solver for monomorphized copies)
        // must not be recorded, so no bogus `?:1:1` location is ever printed.
        let store = crate::MirStore::new();
        crate::using_storage(&store, || {
            let mut builder = MirBuilder::new();
            let i32_ty = MirType::I32.into();

            let mut f = builder.start_function("empty_span".into(), i32_ty);
            let x = f.new_temp(i32_ty, true);
            f.create_block();
            f.set_current_span(Some(SrcPos::default()));
            f.push_assign(Place::Local(x.clone()), Rvalue::Use(Operand::Constant(MirLiteral::I32(1))));
            f.ret(None);
            f.finish_function();

            let module = builder.build_module(PtrSize::U64);
            let func = module.functions[0].borrow().clone();
            let body = func.body.as_ref().expect("function should have a body");
            assert_eq!(body.statement_spans[0].len(), 2);
            assert!(body.statement_spans[0].iter().all(Option::is_none));
        });
    }
}
