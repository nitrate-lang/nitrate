//! # The NLL Borrow Checker Core
//!
//! This module implements the forward dataflow that computes borrow regions
//! and detects conflicts. The overall structure follows the design laid out in
//! the crate documentation:
//!
//! 1. A [`FunctionData`] snapshot clones the function body into owned data so
//!    the checker never contends with `RefCell`s or TLS borrows.
//! 2. [`liveness::compute_liveness`] precomputes per-statement liveness.
//! 3. [`BorrowChecker::run`] walks the CFG forward with a worklist. The state
//!    at each point is the set of active [`BorrowRecord`]s (each with a set of
//!    *holders* — places storing the borrow value), plus the moved-from and
//!    possibly-uninitialized place sets.
//! 4. Kills are applied after every statement from the precomputed liveness:
//!    a holder is dropped when its root local is not live on exit, when it is
//!    overwritten by an assignment, or when its storage dies.

use crate::diagnosis::BorrowError;
use crate::liveness::{self, BitSet, LivenessInfo};
use crate::place;
use nitrate_diagnosis::CompilerLog;
use nitrate_mir::MirFunction;
use nitrate_mir::MirModule;
use nitrate_mir::{BasicBlockId, BorrowKind, LocalId, MirStore, MirTypeId, Operand, Place, Rvalue, Statement, Terminator};
use nitrate_nstring::NString;
use nitrate_tree::SrcPos;
use std::collections::{HashMap, HashSet};
use thin_vec::ThinVec;

/// A compact, fully-owned snapshot of a MIR function body.
///
/// All handles (`LocalId`, `BasicBlockId`, `MirTypeId`) are `Copy` and may be
/// dereferenced through the TLS `MirStore` at any time; cloning the blocks here
/// means the checker holds no `RefCell` borrows.
pub(crate) struct FunctionData {
    pub name: NString,
    /// Local declarations, positional (index = position in `body.locals`).
    pub locals: Vec<nitrate_mir::LocalDecl>,
    /// `LocalId` for each positional local.
    pub local_ids: Vec<LocalId>,
    /// Number of leading locals that are function parameters.
    pub arg_count: usize,
    /// Basic blocks, positional (index = order in `body.blocks`).
    pub blocks: Vec<nitrate_mir::BasicBlock>,
    /// Index of the entry block.
    pub entry: usize,
    /// Per-statement source map, parallel to `blocks`: `statement_spans[b][s]`
    /// is the position of block `b`'s `s`-th statement; the final entry of
    /// each inner vector is the block's terminator position.
    pub statement_spans: Vec<Vec<Option<SrcPos>>>,
    id_to_idx: HashMap<BasicBlockId, usize>,
    local_id_to_idx: HashMap<LocalId, usize>,
}

impl FunctionData {
    /// Build a snapshot of a MIR function's body.
    pub fn snapshot(function: &MirFunction, store: &MirStore) -> Option<Self> {
        let body = function.body.as_ref()?;
        let id_to_idx: HashMap<BasicBlockId, usize> =
            body.blocks.iter().enumerate().map(|(i, id)| (*id, i)).collect();
        let blocks = body.blocks.iter().map(|id| store[id].borrow().clone()).collect();
        let entry = *id_to_idx.get(&body.entry_block)?;
        let local_id_to_idx: HashMap<LocalId, usize> =
            body.local_ids.iter().enumerate().map(|(i, id)| (*id, i)).collect();
        Some(Self {
            name: function.name.clone(),
            locals: body.locals.iter().cloned().collect(),
            local_ids: body.local_ids.iter().copied().collect(),
            arg_count: body.arg_count as usize,
            blocks,
            entry,
            statement_spans: body.statement_spans.clone(),
            id_to_idx,
            local_id_to_idx,
        })
    }

    /// Positional index of a local.
    pub fn idx_of(&self, id: &LocalId) -> usize {
        *self.local_id_to_idx.get(id).unwrap_or_else(|| panic!("local not found in function {:?}", self.name))
    }

    /// The type of a local, if it exists.
    pub fn local_ty(&self, id: &LocalId) -> Option<MirTypeId> {
        self.local_id_to_idx.get(id).and_then(|i| self.locals.get(*i)).map(|l| l.ty.clone())
    }

    /// Whether the local is declared mutable.
    pub fn local_is_mutable(&self, id: &LocalId) -> bool {
        self.local_id_to_idx.get(id).is_some_and(|i| self.locals.get(*i).is_some_and(|l| l.mutable))
    }

    /// Diagnostic display name for a local.
    pub fn local_display_name(&self, id: &LocalId) -> String {
        format!("_{}", id.as_usize())
    }

    /// Successor block indices of a block.
    pub fn successors(&self, b: usize) -> Vec<usize> {
        self.blocks[b].successors().iter().map(|id| self.id_to_idx[id]).collect()
    }

    /// Map a `BasicBlockId` to its positional index.
    pub fn block_idx(&self, id: &BasicBlockId) -> usize {
        self.id_to_idx[id]
    }
}


/// A precise program point: block index and statement index (the terminator is
/// statement index `N` where `N` is the block's statement count).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
struct Location {
    block: usize,
    stmt: usize,
}

/// A single active borrow.
#[derive(Debug, Clone)]
struct BorrowRecord {
    idx: usize,
    kind: BorrowKind,
    /// The place whose memory is borrowed.
    source: Place,
    /// Places currently storing this borrow's value. The borrow lives as long
    /// as any holder lives.
    holders: Vec<Place>,
    /// Two-phase reservation (only for `BorrowKind::Mutable`).
    two_phase: bool,
    /// Whether a two-phase borrow has been activated (exclusivity enforced).
    activated: bool,
}

impl BorrowRecord {
    fn kind_str(&self) -> &'static str {
        match self.kind {
            BorrowKind::Shared => "shared",
            BorrowKind::Mutable => "mutable",
        }
    }
}

/// The dataflow state at a program point.
#[derive(Debug, Clone, Default)]
struct State {
    /// Active borrows, in creation order.
    borrows: Vec<BorrowRecord>,
    /// Places moved from (on some path reaching this point).
    moved: HashSet<Place>,
    /// Places possibly uninitialized (on some path reaching this point).
    uninit: HashSet<Place>,
}

/// The NLL borrow checker for one function.
pub(crate) struct BorrowChecker<'a> {
    function: FunctionData,
    module: &'a MirModule,
    log: &'a CompilerLog,
    liveness: LivenessInfo,
    /// Per-block entry states (the merge points of the worklist).
    entry_states: Vec<State>,
    worklist: Vec<usize>,
    in_worklist: Vec<bool>,
    next_borrow_idx: usize,
    pub errors: Vec<BorrowError>,
    /// Deduplication keys for already-reported errors.
    reported: HashSet<String>,
}

impl<'a> BorrowChecker<'a> {
    pub fn new(function: FunctionData, module: &'a MirModule, log: &'a CompilerLog) -> Self {
        let nblocks = function.blocks.len();
        let liveness = liveness::compute_liveness(&function);
        let entry_states = vec![State::default(); nblocks];
        let in_worklist = vec![false; nblocks];
        Self {
            function,
            module,
            log,
            liveness,
            entry_states,
            worklist: Vec::new(),
            in_worklist,
            next_borrow_idx: 0,
            errors: Vec::new(),
            reported: HashSet::new(),
        }
    }
}

impl<'a> BorrowChecker<'a> {


    /// Run the borrow check to a fixed point and collect all errors.
    pub fn run(&mut self) {
        let entry = self.function.entry;
        // The entry state marks every non-parameter local as possibly
        // uninitialized; parameters arrive initialized from the caller.
        let mut init = State::default();
        for (i, id) in self.function.local_ids.iter().enumerate() {
            if i >= self.function.arg_count {
                init.uninit.insert(Place::Local(*id));
            }
        }
        self.entry_states[entry] = init;
        self.push_worklist(entry);

        while let Some(b) = self.worklist.pop() {
            self.in_worklist[b] = false;
            let exit = self.process_block(b, self.entry_states[b].clone());
            let edges = self.successor_edges(b);
            for (succ, _edge_args) in edges {
                // Mark the target block's argument locals as initialized: they
                // receive their values on this edge.
                let succ_args = self.function.blocks[succ].arg_local_ids.clone();
                let mut merged = self.merge_state(&self.entry_states[succ], &exit);
                if let Some(merged) = merged.as_mut() {
                    for arg_local in &succ_args {
                        let p = Place::Local(*arg_local);
                        merged.moved.retain(|m| !(m == &p || place::is_prefix(&p, m)));
                        merged.uninit.retain(|u| !(u == &p || place::is_prefix(&p, u)));
                    }
                }
                if let Some(merged) = merged {
                    self.entry_states[succ] = merged;
                    self.push_worklist(succ);
                }
            }
        }
    }

    fn push_worklist(&mut self, b: usize) {
        if !self.in_worklist[b] {
            self.in_worklist[b] = true;
            self.worklist.push(b);
        }
    }

    /// Process all statements and the terminator of a block, returning the
    /// exit state. Kills (liveness-based, assignment-based, storage-based) are
    /// applied after each statement and after the terminator.
    fn process_block(&mut self, b: usize, mut state: State) -> State {
        // Clone statements to avoid holding an immutable borrow of `self`
        // across the mutable `process_statement`/`apply_kills` calls.
        let statements = self.function.blocks[b].statements.clone();
        let n = statements.len();
        for (i, stmt) in statements.iter().enumerate() {
            let live_after = self.liveness.live_before[b][i + 1].clone();
            self.process_statement(stmt, Location { block: b, stmt: i }, &mut state);
            self.apply_kills(&mut state, &live_after);
        }
        let live_after = self.liveness.live_out[b].clone();
        self.process_terminator(
            &self.function.blocks[b].terminator.clone(),
            Location { block: b, stmt: n },
            &mut state,
        );
        // Propagate borrow values flowing into successor block arguments
        // BEFORE the liveness kill, so the successor's argument locals carry
        // the borrow region into the next block.
        self.propagate_edge_holders(b, &mut state);
        self.apply_kills(&mut state, &live_after);
        state
    }

    /// For every edge out of block `b`, extend each borrow's holder set with
    /// the successor's argument locals when a reference-typed value that holds
    /// the borrow is passed as a block argument.
    fn propagate_edge_holders(&mut self, b: usize, state: &mut State) {
        let edges = self.successor_edges(b);
        for (succ, edge_args) in edges {
            let succ_arg_ids = self.function.blocks[succ].arg_local_ids.clone();
            let ref_typed: Vec<bool> = edge_args
                .iter()
                .map(|op| match op {
                    Operand::Copy(p) | Operand::Move(p) => {
                        place::is_reference_typed(p, &self.function, self.module)
                    }
                    Operand::Constant(_) => false,
                })
                .collect();
            for ((op, arg_local), is_ref) in edge_args.iter().zip(succ_arg_ids.iter()).zip(ref_typed.iter()) {
                if let Operand::Copy(p) | Operand::Move(p) = op {
                    if *is_ref {
                        let dest = Place::Local(*arg_local);
                        self.propagate_holders(p, &dest, state);
                    }
                }
            }
        }
    }

    /// The outgoing edges of a block: each successor index paired with the
    /// operands passed to that successor's block arguments.
    fn successor_edges(&self, b: usize) -> Vec<(usize, ThinVec<Operand>)> {
        let term = &self.function.blocks[b].terminator;
        let mut edges = Vec::new();
        match term {
            Terminator::Goto { target, args } => {
                edges.push((self.function.block_idx(target), args.clone()));
            }
            Terminator::If { true_target, true_args, false_target, false_args, .. } => {
                edges.push((self.function.block_idx(true_target), true_args.clone()));
                edges.push((self.function.block_idx(false_target), false_args.clone()));
            }
            Terminator::SwitchInt { targets, otherwise, otherwise_args, .. } => {
                for (_, bb, args) in targets {
                    edges.push((self.function.block_idx(bb), args.clone()));
                }
                edges.push((self.function.block_idx(otherwise), otherwise_args.clone()));
            }
            Terminator::Call { target: Some(t), target_args, .. } => {
                edges.push((self.function.block_idx(t), target_args.clone()));
            }
            Terminator::Call { target: None, .. }
            | Terminator::Return { .. }
            | Terminator::Unreachable => {}
        }
        edges
    }

    /// Merge `exit` into `entry`, returning `Some` if the entry state changed.
    /// Borrows are merged by index with holder-union; moved/uninit sets are
    /// unioned.
    fn merge_state(&self, entry: &State, exit: &State) -> Option<State> {
        let mut changed = false;
        let mut result = entry.clone();

        for eb in &exit.borrows {
            match result.borrows.iter_mut().find(|rb| rb.idx == eb.idx) {
                Some(rb) => {
                    for h in &eb.holders {
                        if !rb.holders.contains(h) {
                            rb.holders.push(h.clone());
                            changed = true;
                        }
                    }
                }
                None => {
                    result.borrows.push(eb.clone());
                    changed = true;
                }
            }
        }
        for p in &exit.moved {
            if result.moved.insert(p.clone()) {
                changed = true;
            }
        }
        for p in &exit.uninit {
            if result.uninit.insert(p.clone()) {
                changed = true;
            }
        }
        if changed { Some(result) } else { None }
    }

    /// Drop holders whose root local is no longer live, then drop borrows
    /// with no remaining holders.
    fn apply_kills(&self, state: &mut State, live_after: &BitSet) {
        for br in &mut state.borrows {
            br.holders.retain(|h| {
                match place::root_place(h) {
                    Place::Local(id) => live_after.contains(self.function.idx_of(id)),
                    Place::Static(_) => true,
                    Place::Deref(_) | Place::Field { .. } | Place::Index { .. } | Place::Downcast { .. } => {
                        unreachable!("root_place strips all projections")
                    }
                }
            });
        }
        state.borrows.retain(|br| !br.holders.is_empty());
    }

    /// Drop holders overwritten by a whole/local assignment to `dest`.
    fn kill_holders_overwritten(&mut self, dest: &Place, state: &mut State) {
        for br in &mut state.borrows {
            br.holders.retain(|h| !(h == dest || place::is_prefix(dest, h)));
        }
        state.borrows.retain(|br| !br.holders.is_empty());
    }

    /// Drop borrows whose holders or sources are rooted at a local whose
    /// storage is being (de)allocated.
    fn kill_borrows_at_local(&mut self, id: &LocalId, state: &mut State) {
        for br in &mut state.borrows {
            br.holders.retain(|h| !matches!(place::root_place(h), Place::Local(holder) if *holder == *id));
        }
        state.borrows.retain(|br| {
            !br.holders.is_empty() && !matches!(place::root_place(&br.source), Place::Local(src) if *src == *id)
        });
    }


    // ─────────────────────────────────────────────────────────
    // Statement and terminator processing
    // ─────────────────────────────────────────────────────────

    fn process_statement(&mut self, stmt: &Statement, loc: Location, state: &mut State) {
        match stmt {
            Statement::Assign(dest, rvalue) => {
                self.write_place(dest, state, loc);
                self.kill_holders_overwritten(dest, state);
                self.process_rvalue(rvalue, dest, state, loc);
                self.mark_initialized(dest, state);
            }
            Statement::SetDiscriminant { place, .. } => {
                self.write_place(place, state, loc);
                self.kill_holders_overwritten(place, state);
                self.mark_initialized(place, state);
            }
            Statement::StorageLive(id) => {
                // Fresh storage: the local (and anything under it) is
                // possibly-uninitialized, and any borrow touching its memory
                // is invalid.
                state.uninit.insert(Place::Local(*id));
                self.kill_borrows_at_local(id, state);
            }
            Statement::StorageDead(id) => {
                state.uninit.insert(Place::Local(*id));
                self.kill_borrows_at_local(id, state);
            }
        }
    }

    fn process_rvalue(&mut self, rvalue: &Rvalue, dest: &Place, state: &mut State, loc: Location) {
        match rvalue {
            Rvalue::Use(op) => {
                self.process_operand(op, state, loc);
                // Borrow-value propagation: copying a reference-typed value
                // from a holder extends the borrow's region to the copy.
                if let Operand::Copy(p) | Operand::Move(p) = op {
                    let is_reference = place::is_reference_typed(p, &self.function, self.module);
                    if is_reference {
                        self.propagate_holders(p, dest, state);
                    }
                }
            }
            Rvalue::Ref { region, place } => {
                self.borrow_place(*region, place, dest, state, loc);
            }
            Rvalue::Len(place) => {
                self.read_place(place, state, loc, false);
            }
            Rvalue::Cast { value, .. } => {
                self.process_operand(value, state, loc);
            }
            Rvalue::BinaryOp { lhs, rhs, .. } | Rvalue::CheckedBinaryOp { lhs, rhs, .. } => {
                self.process_operand(lhs, state, loc);
                self.process_operand(rhs, state, loc);
            }
            Rvalue::UnaryOp { operand, .. } => {
                self.process_operand(operand, state, loc);
            }
            Rvalue::NullaryOp(..) => {}
            Rvalue::Aggregate(_, operands) => {
                for op in operands {
                    self.process_operand(op, state, loc);
                }
            }
        }
    }

    fn process_operand(&mut self, op: &Operand, state: &mut State, loc: Location) {
        match op {
            Operand::Constant(_) => {}
            Operand::Copy(place) => self.read_place(place, state, loc, false),
            Operand::Move(place) => self.read_place(place, state, loc, true),
        }
    }


    fn process_terminator(&mut self, term: &Terminator, loc: Location, state: &mut State) {
        match term {
            Terminator::Goto { args, .. } => {
                for op in args {
                    self.process_operand(op, state, loc);
                }
            }
            Terminator::If { condition, true_args, false_args, .. } => {
                self.process_operand(condition, state, loc);
                for op in true_args.iter().chain(false_args.iter()) {
                    self.process_operand(op, state, loc);
                }
            }
            Terminator::SwitchInt { discr, targets, otherwise_args, .. } => {
                self.process_operand(discr, state, loc);
                for (_, _, args) in targets {
                    for op in args {
                        self.process_operand(op, state, loc);
                    }
                }
                for op in otherwise_args {
                    self.process_operand(op, state, loc);
                }
            }
            Terminator::Return { value } => {
                if let Some(op) = value {
                    self.process_operand(op, state, loc);
                    if let Operand::Copy(p) | Operand::Move(p) = op {
                        self.check_returned_borrow(p, state, loc);
                    }
                }
            }
            Terminator::Unreachable => {}
            Terminator::Call { callee, args, destination, target_args, .. } => {
                self.process_operand(callee, state, loc);
                for op in args {
                    self.process_operand(op, state, loc);
                }
                // Activate two-phase borrows that are direct call arguments.
                self.activate_two_phase_borrows(args, state, loc);
                if let Some(dest) = destination {
                    self.write_place(dest, state, loc);
                    self.kill_holders_overwritten(dest, state);
                    self.mark_initialized(dest, state);
                }
                for op in target_args {
                    self.process_operand(op, state, loc);
                }
            }
        }
    }


    // ─────────────────────────────────────────────────────────
    // Access helpers: reads, writes, borrows
    // ─────────────────────────────────────────────────────────

    /// Check a read (or move, when `move_out` is true) of a place.
    fn read_place(&mut self, place: &Place, state: &mut State, loc: Location, move_out: bool) {
        // 1. Initialization / move-state checks.
        if let Some(moved) = self.find_moved(place, state) {
            self.report(
                BorrowError::UseAfterMove {
                    span: None,
                    place: place::place_to_string(place, &self.function),
                    reason: format!(
                        "`{}` was moved from earlier on this path",
                        place::place_to_string(&moved, &self.function)
                    ),
                },
                loc,
            );
        } else if let Some(uninit) = self.find_uninit(place, state) {
            self.report(
                BorrowError::UseBeforeInit {
                    span: None,
                    place: place::place_to_string(place, &self.function),
                    reason: format!(
                        "`{}` is not initialized on this path",
                        place::place_to_string(&uninit, &self.function)
                    ),
                },
                loc,
            );
        }

        // 2. Borrow conflicts.
        for br in &state.borrows {
            if place::overlaps(place, &br.source) {
                if move_out {
                    // Moving deinitializes the borrowed memory — conflicts
                    // with borrows of any kind.
                    self.report(
                        BorrowError::MoveWhileBorrowed {
                            span: None,
                            place: place::place_to_string(place, &self.function),
                            borrow_kind: br.kind_str().to_string(),
                            reason: "moving deinitializes the borrowed memory".to_string(),
                        },
                        loc,
                    );
                } else if br.kind == BorrowKind::Mutable && br.activated {
                    self.report(
                        BorrowError::ReadWhileMutablyBorrowed {
                            span: None,
                            place: place::place_to_string(place, &self.function),
                            borrow_kind: "mutable".to_string(),
                            reason: "reading while a mutable borrow is active".to_string(),
                        },
                        loc,
                    );
                }
            }
        }

        // 3. A move deinitializes the place.
        if move_out {
            state.moved.insert(place.clone());
            state.uninit.insert(place.clone());
        }
    }


    /// Check a write to a place (assignment destination, discriminant, call
    /// destination). Writes conflict with borrows of any kind and may not
    /// target a place whose parent was moved.
    fn write_place(&mut self, place: &Place, state: &State, loc: Location) {
        for br in &state.borrows {
            if place::overlaps(place, &br.source) {
                self.report(
                    BorrowError::WriteWhileBorrowed {
                        span: None,
                        place: place::place_to_string(place, &self.function),
                        borrow_kind: br.kind_str().to_string(),
                        reason: "assigning while a borrow of the same memory is active".to_string(),
                    },
                    loc,
                );
            }
        }
        // Writing into a moved-from parent is an error; writing the moved
        // place itself is a re-initialization and is allowed.
        let chain = place::init_chain(place);
        for ancestor in chain.iter().skip(1) {
            if state.moved.contains(ancestor) {
                self.report(
                    BorrowError::AssignToMoved {
                        span: None,
                        place: place::place_to_string(place, &self.function),
                        reason: format!(
                            "parent `{}` was moved from",
                            place::place_to_string(ancestor, &self.function)
                        ),
                    },
                    loc,
                );
                break;
            }
        }
    }

    /// Create a borrow of `place` of the given kind, stored in `dest`.
    fn borrow_place(&mut self, kind: BorrowKind, place: &Place, dest: &Place, state: &mut State, loc: Location) {
        // 1. Initialization checks — borrowing uninitialized or moved memory
        //    creates a dangling reference.
        if let Some(moved) = self.find_moved(place, state) {
            self.report(
                BorrowError::BorrowOfMoved {
                    span: None,
                    place: place::place_to_string(place, &self.function),
                    reason: format!("`{}` was moved from earlier", place::place_to_string(&moved, &self.function)),
                },
                loc,
            );
        } else if let Some(uninit) = self.find_uninit(place, state) {
            self.report(
                BorrowError::BorrowOfUninit {
                    span: None,
                    place: place::place_to_string(place, &self.function),
                    reason: format!("`{}` is not initialized", place::place_to_string(&uninit, &self.function)),
                },
                loc,
            );
        }

        // 2. Mutability check.
        if kind == BorrowKind::Mutable && !place::is_place_mutable(place, &self.function, self.module) {
            self.report(
                BorrowError::MutableBorrowOfImmutable {
                    span: None,
                    place: place::place_to_string(place, &self.function),
                    reason: "the target is not declared mutable".to_string(),
                },
                loc,
            );
        }

        // 3. Conflict detection with existing active borrows.
        let two_phase = kind == BorrowKind::Mutable && self.is_two_phase(dest);
        for br in &state.borrows {
            if !place::overlaps(place, &br.source) {
                continue;
            }
            match kind {
                BorrowKind::Mutable => {
                    // A new mutable borrow (even a reservation) conflicts with
                    // any existing borrow of overlapping memory.
                    self.report(
                        BorrowError::MutableBorrowConflict {
                            span: None,
                            place: place::place_to_string(place, &self.function),
                            borrow_kind: br.kind_str().to_string(),
                            reason: "a mutable borrow cannot coexist with any other borrow".to_string(),
                        },
                        loc,
                    );
                }
                BorrowKind::Shared => {
                    // Shared borrows coexist with shared borrows and with
                    // *reserved* two-phase mutable borrows, but not with an
                    // activated mutable borrow.
                    if br.kind == BorrowKind::Mutable && br.activated {
                        self.report(
                            BorrowError::SharedBorrowConflict {
                                span: None,
                                place: place::place_to_string(place, &self.function),
                                borrow_kind: "mutable".to_string(),
                                reason: "a shared borrow cannot coexist with an active mutable borrow".to_string(),
                            },
                            loc,
                        );
                    }
                }
            }
        }

        // 4. Record the borrow.
        let idx = self.next_borrow_idx;
        self.next_borrow_idx += 1;
        state.borrows.push(BorrowRecord {
            idx,
            kind,
            source: place.clone(),
            holders: vec![dest.clone()],
            two_phase,
            activated: !two_phase,
        });
    }


    /// Activate any two-phase borrows whose holders appear directly in the
    /// call's argument list. Activation enforces exclusivity: any other active
    /// borrow of overlapping memory at this point is a conflict.
    fn activate_two_phase_borrows(&mut self, args: &[Operand], state: &mut State, loc: Location) {
        let arg_places: Vec<Place> = args
            .iter()
            .filter_map(|op| match op {
                Operand::Copy(p) | Operand::Move(p) => Some(p.clone()),
                Operand::Constant(_) => None,
            })
            .collect();

        let to_activate: Vec<usize> = state
            .borrows
            .iter()
            .filter(|br| br.two_phase && !br.activated)
            .filter(|br| arg_places.iter().any(|p| br.holders.contains(p)))
            .map(|br| br.idx)
            .collect();

        for idx in to_activate {
            if let Some(br) = state.borrows.iter_mut().find(|br| br.idx == idx) {
                br.activated = true;
            }
            // Enforce exclusivity against every other active borrow.
            let source = state.borrows.iter().find(|br| br.idx == idx).map(|br| br.source.clone());
            if let Some(source) = source {
                let others: Vec<BorrowRecord> =
                    state.borrows.iter().filter(|br| br.idx != idx).cloned().collect();
                for other in others {
                    if place::overlaps(&source, &other.source) {
                        self.report(
                            BorrowError::TwoPhaseActivationConflict {
                                span: None,
                                place: place::place_to_string(&source, &self.function),
                                borrow_kind: other.kind_str().to_string(),
                                reason: "the two-phase mutable borrow was activated while another borrow of the same memory was still active".to_string(),
                            },
                            loc,
                        );
                    }
                }
            }
        }
    }

    /// Check whether returning `place` (the return operand's place) returns a
    /// borrow of memory that dies with this frame.
    fn check_returned_borrow(&mut self, place: &Place, state: &State, loc: Location) {
        for br in &state.borrows {
            if br.holders.contains(place) && !place::is_escapable(&br.source) {
                self.report(
                    BorrowError::BorrowOfLocalEscape {
                        span: None,
                        place: place::place_to_string(&br.source, &self.function),
                        reason: "the referenced local dies when the function returns".to_string(),
                    },
                    loc,
                );
            }
        }
    }

    /// Find a moved-from ancestor-or-self of `place`, if any.
    fn find_moved(&self, place: &Place, state: &State) -> Option<Place> {
        state.moved.iter().find(|m| place::is_prefix(m, place)).cloned()
    }

    /// Find a possibly-uninitialized ancestor-or-self of `place`, if any.
    fn find_uninit(&self, place: &Place, state: &State) -> Option<Place> {
        place::init_chain(place).iter().find(|anc| state.uninit.contains(*anc)).cloned()
    }

    /// Mark a place (and everything reachable under it) as initialized and
    /// not moved-from.
    fn mark_initialized(&mut self, place: &Place, state: &mut State) {
        state.moved.retain(|m| !(m == place || place::is_prefix(place, m)));
        state.uninit.retain(|u| !(u == place || place::is_prefix(place, u)));
    }

    /// Extend `dest`'s holder membership for every borrow currently held by
    /// `src` (value propagation through copies).
    fn propagate_holders(&mut self, src: &Place, dest: &Place, state: &mut State) {
        for br in &mut state.borrows {
            if br.holders.contains(src) && !br.holders.contains(dest) {
                br.holders.push(dest.clone());
            }
        }
    }

    /// A `&mut` borrow is two-phase when its destination local is used exactly
    /// once, as a direct call argument. Only then can the reservation phase be
    /// meaningfully distinguished from activation.
    fn is_two_phase(&self, dest: &Place) -> bool {
        if let Place::Local(id) = dest {
            let idx = self.function.idx_of(id);
            self.liveness.use_counts[idx] == 1 && self.liveness.call_arg_uses.contains(idx)
        } else {
            false
        }
    }



    // ─────────────────────────────────────────────────────────
    // Reporting
    // ─────────────────────────────────────────────────────────

    /// Source position of the given program point, if one was recorded during
    /// HIR→MIR lowering. Returns `None` for synthetic statements or MIR built
    /// directly by tests.
    fn span_at(&self, loc: Location) -> Option<SrcPos> {
        self.function
            .statement_spans
            .get(loc.block)
            .and_then(|spans| spans.get(loc.stmt).copied())
            .flatten()
    }

    /// Report an error to the log (deduplicated) and collect it.
    fn report(&mut self, error: BorrowError, loc: Location) {
        // Attach the source position of the offending statement/terminator so
        // the diagnostic carries `file:line:col` context.
        let error = error.with_span(self.span_at(loc));
        let key = format!("{loc:?}|{error}");
        if self.reported.insert(key) {
            self.log.report(&error);
            self.errors.push(error);
        }
    }
}
