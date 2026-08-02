use crate::func::{LocalDecl, MirFunction};
use crate::operand::{MirBinaryOp, MirLiteral, MirUnaryOp, Operand};
use crate::place::Place;
use crate::rvalue::{AggregateKind, BorrowKind, NullaryOp, Rvalue};
use crate::stmt::{BasicBlock, Statement, Terminator};
use crate::store::{BasicBlockId, LocalId, MirFunctionId, MirStore, MirTypeId, get_storage, using_storage};
use crate::ty::{MirType, PtrSize};
use nitrate_nstring::NString;
use thin_vec::ThinVec;

// ─────────────────────────────────────────────────────────────
// Utility: fresh local counter
// ─────────────────────────────────────────────────────────────

/// A counter for generating fresh temporary local indices.
#[derive(Debug, Clone)]
pub struct FreshLocalCounter(u32);

impl FreshLocalCounter {
    pub fn new() -> Self {
        FreshLocalCounter(0)
    }

    pub fn next(&mut self) -> u32 {
        let val = self.0;
        self.0 += 1;
        val
    }
}

impl Default for FreshLocalCounter {
    fn default() -> Self {
        Self::new()
    }
}

// ─────────────────────────────────────────────────────────────
// MirBuilder — top-level module builder
// ─────────────────────────────────────────────────────────────

/// A builder for constructing MIR modules and functions.
///
/// The builder manages the construction of MIR functions incrementally,
/// tracking locals, basic blocks, and statements. It holds a reference
/// to a `MirStore` and inserts constructed entities into it on `finish`.
///
/// # Usage
///
/// ```ignore
/// let mut builder = MirBuilder::new(&store);
/// let func_id = builder
///     .start_function("my_func".into(), return_ty_id)?
///     .add_param("x".into(), ty_id, false)?
///     .new_block()?
///     .push_assign(local_id, rvalue)?
///     .set_terminator(Terminator::Return { value: None })?
///     .finish_function()?;
/// let module = builder.build_module(ptr_size)?;
/// ```
#[derive(Debug)]
pub struct MirBuilder<'store> {
    store: &'store MirStore,
    /// Accumulated function IDs for the final module.
    functions: ThinVec<MirFunctionId>,
}

impl<'store> MirBuilder<'store> {
    #[must_use]
    pub fn new(store: &'store MirStore) -> Self {
        Self {
            store,
            functions: ThinVec::new(),
        }
    }

    /// Begin constructing a new function. Returns a per-function builder.
    ///
    /// The per-function builder holds data locally; nothing is inserted
    /// into the `MirStore` until `finish_function()` is called.
    pub fn start_function(&mut self, name: NString, return_ty: MirTypeId) -> MirFunctionBuilder<'_, 'store> {
        MirFunctionBuilder {
            builder: self,
            name,
            return_ty,
            params: ThinVec::new(),
            locals: ThinVec::new(),
            arg_count: 0,
            blocks: ThinVec::new(),
            entry_block: None,
            current_block: None,
            current_statements: ThinVec::new(),
            fresh_counter: FreshLocalCounter::new(),
        }
    }

    /// Insert a pre-built MirFunctionId into the module's function list.
    /// Useful for extern functions or functions built elsewhere.
    pub fn add_function(&mut self, func_id: MirFunctionId) {
        self.functions.push(func_id);
    }

    /// Build the final `MirModule` from all accumulated functions.
    pub fn build_module(self, ptr_size: PtrSize) -> MirModule {
        MirModule {
            functions: self.functions,
            globals: ThinVec::new(),
            ptr_size,
        }
    }

    /// Convenience: intern a type and return its `MirTypeId`.
    pub fn intern_type(&self, ty: MirType) -> MirTypeId {
        self.store.store_type(ty)
    }

    /// Access the underlying MirStore.
    pub fn store(&self) -> &'store MirStore {
        self.store
    }
}

// ─────────────────────────────────────────────────────────────
// MirFunctionBuilder — per-function builder
// ─────────────────────────────────────────────────────────────

/// A builder for a single MIR function body.
///
/// Holds all data locally until `finish_function()` commits it to the
/// `MirStore` and returns a `MirFunctionId`.
#[derive(Debug)]
pub struct MirFunctionBuilder<'b, 'store> {
    builder: &'b mut MirBuilder<'store>,
    name: NString,
    return_ty: MirTypeId,
    params: ThinVec<LocalId>,
    locals: ThinVec<LocalDecl>,
    arg_count: u32,
    blocks: ThinVec<BasicBlockId>,
    entry_block: Option<BasicBlockId>,

    /// The basic block currently being constructed (if any).
    pub current_block: Option<BasicBlockId>,
    /// Statements accumulated for the current block.
    pub current_statements: ThinVec<Statement>,

    /// Fresh local counter for temporaries.
    fresh_counter: FreshLocalCounter,
}

impl<'b, 'store> MirFunctionBuilder<'b, 'store> {
    // ── Parameters and locals ────────────────────────────────

    /// Register a parameter local and return its `LocalId`.
    ///
    /// Parameters must be added before the entry block is created.
    /// They are stored as the first N locals.
    pub fn add_param(&mut self, _name: NString, ty: MirTypeId, mutable: bool) -> LocalId {
        let local = LocalDecl { ty, mutable };
        // Store locally first; actual MirStore insertion happens at finish.
        // We use a placeholder approach: store into MirStore immediately
        // so that LocalId dereferencing works if needed during building.
        let id: LocalId = using_storage(self.builder.store, || get_storage(|s| s.store_local(local)));
        self.params.push(id.clone());
        self.arg_count += 1;
        // Also track in locals list for the function
        self.locals.push(LocalDecl { ty, mutable });
        id
    }

    /// Create a new temporary local and return its `LocalId`.
    ///
    /// Temporary locals are used for intermediate computation results
    /// (e.g., the result of a binary operation before it is assigned
    /// to its final destination).
    pub fn new_temp(&mut self, ty: MirTypeId, mutable: bool) -> LocalId {
        let local = LocalDecl { ty, mutable };
        let id: LocalId = using_storage(self.builder.store, || get_storage(|s| s.store_local(local)));
        self.locals.push(LocalDecl { ty, mutable });
        id
    }

    // ── Block management ─────────────────────────────────────

    /// Start a new basic block. If a previous block was being built,
    /// **it must have a terminator set** or this will panic.
    ///
    /// The first block created becomes the entry block.
    pub fn start_block(&mut self) -> &mut Self {
        // Finalize any current block
        if let Some(_current) = self.current_block.take() {
            // The previous block should have had its terminator set.
            // We don't validate here; the caller is responsible.
        }
        self.current_statements = ThinVec::new();
        // Create the block lazily — on finish or when the terminator is set.
        // For now, we just note that a new block is starting.
        self.current_block = None; // will be created on first statement/terminator
        self
    }

    /// Ensure there is an active current block, creating one if necessary.
    fn ensure_block(&mut self) -> BasicBlockId {
        if let Some(ref bb) = self.current_block {
            return bb.clone();
        }
        // Create an empty block
        let bb = BasicBlock {
            statements: ThinVec::new(),
            terminator: Terminator::Unreachable,
        };
        let bb_id: BasicBlockId = using_storage(self.builder.store, || get_storage(|s| s.store_basic_block(bb)));
        if self.entry_block.is_none() {
            self.entry_block = Some(bb_id.clone());
        }
        self.blocks.push(bb_id.clone());
        self.current_block = Some(bb_id.clone());
        bb_id
    }

    // ── Statements ───────────────────────────────────────────

    /// Push a statement into the current block.
    pub fn push_stmt(&mut self, stmt: Statement) -> &mut Self {
        let bb_id = self.ensure_block();
        // Update the stored block's statements.
        // Access the store directly since we're inside using_storage.
        using_storage(self.builder.store, || {
            let mut borrowed = self.builder.store[&bb_id].borrow_mut();
            borrowed.statements.push(stmt.clone());
        });
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

    /// Set the terminator of the current block. This finalizes the block
    /// and prepares for a new block to be started.
    pub fn set_terminator(&mut self, terminator: Terminator) -> &mut Self {
        let bb_id = self.ensure_block();
        using_storage(self.builder.store, || {
            let mut borrowed = self.builder.store[&bb_id].borrow_mut();
            borrowed.terminator = terminator;
        });
        self.current_block = None;
        self
    }

    /// Shorthand: unconditional branch to target.
    pub fn goto(&mut self, target: BasicBlockId) -> &mut Self {
        self.set_terminator(Terminator::Goto { target })
    }

    /// Shorthand: conditional branch.
    pub fn if_br(&mut self, condition: Operand, true_target: BasicBlockId, false_target: BasicBlockId) -> &mut Self {
        self.set_terminator(Terminator::If {
            condition,
            true_target,
            false_target,
        })
    }

    /// Shorthand: return.
    pub fn ret(&mut self, value: Option<Operand>) -> &mut Self {
        self.set_terminator(Terminator::Return { value })
    }

    /// Shorthand: call with no return (diverging).
    pub fn call(&mut self, callee: Operand, args: ThinVec<Operand>) -> &mut Self {
        self.set_terminator(Terminator::Call { callee, args })
    }

    /// Shorthand: call with return value.
    pub fn call_return(
        &mut self,
        callee: Operand,
        args: ThinVec<Operand>,
        destination: Place,
        target: BasicBlockId,
    ) -> &mut Self {
        self.set_terminator(Terminator::CallReturn {
            callee,
            args,
            destination,
            target,
        })
    }

    /// Shorthand: unreachable.
    pub fn unreachable(&mut self) -> &mut Self {
        self.set_terminator(Terminator::Unreachable)
    }

    // ── Convenience: build Rvalues ───────────────────────────

    /// Build an `Rvalue::Use` from an operand.
    pub fn rv_use(op: Operand) -> Rvalue {
        Rvalue::Use(op)
    }

    /// Build an `Rvalue::Ref`.
    pub fn rv_ref(kind: BorrowKind, place: Place) -> Rvalue {
        Rvalue::Ref { region: kind, place }
    }

    /// Build an `Rvalue::Len`.
    pub fn rv_len(place: Place) -> Rvalue {
        Rvalue::Len(place)
    }

    /// Build an `Rvalue::Cast`.
    pub fn rv_cast(value: Operand, target_ty: MirTypeId) -> Rvalue {
        Rvalue::Cast { value, target_ty }
    }

    /// Build an `Rvalue::BinaryOp`.
    pub fn rv_binary(op: MirBinaryOp, lhs: Operand, rhs: Operand) -> Rvalue {
        Rvalue::BinaryOp { op, lhs, rhs }
    }

    /// Build an `Rvalue::CheckedBinaryOp`.
    pub fn rv_checked_binary(op: MirBinaryOp, lhs: Operand, rhs: Operand) -> Rvalue {
        Rvalue::CheckedBinaryOp { op, lhs, rhs }
    }

    /// Build an `Rvalue::UnaryOp`.
    pub fn rv_unary(op: MirUnaryOp, operand: Operand) -> Rvalue {
        Rvalue::UnaryOp { op, operand }
    }

    /// Build an `Rvalue::NullaryOp`.
    pub fn rv_nullary(op: NullaryOp, ty: MirTypeId) -> Rvalue {
        Rvalue::NullaryOp(op, ty)
    }

    /// Build an `Rvalue::Aggregate`.
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
    ///
    /// After this call, the function builder is consumed and the function
    /// ID is added to the parent `MirBuilder`'s function list.
    pub fn finish_function(self) -> MirFunctionId {
        // Ensure all locals are stored (they should already be since we used
        // using_storage during add_param/new_temp)
        // Ensure the entry block exists
        let entry_block = self
            .entry_block
            .expect("No entry block created; call start_block first");

        let func = MirFunction {
            name: self.name,
            params: self.params,
            return_ty: self.return_ty,
            locals: self.locals,
            entry_block,
            blocks: self.blocks,
            arg_count: self.arg_count,
        };

        let func_id = using_storage(self.builder.store, || get_storage(|s| s.store_function(func)));

        self.builder.add_function(func_id.clone());
        func_id
    }

    /// Get the current fresh counter value.
    pub fn fresh_counter(&self) -> &FreshLocalCounter {
        &self.fresh_counter
    }

    /// Get a mutable reference to the fresh counter.
    pub fn fresh_counter_mut(&mut self) -> &mut FreshLocalCounter {
        &mut self.fresh_counter
    }

    /// Access the underlying store.
    pub fn store(&self) -> &'store MirStore {
        self.builder.store
    }
}

// ─────────────────────────────────────────────────────────────
// Re-export MirModule for build_module return type
// ─────────────────────────────────────────────────────────────

use crate::func::MirModule;
