//! # Liveness Analysis
//!
//! Liveness is the foundation of the non-lexical-lifetime borrow checker.
//! A borrow is live from its creation point until the last use of the local
//! that holds it; computing *where* locals are live lets us kill borrows at
//! exactly the right points instead of holding them for a lexical scope.
//!
//! This module computes:
//!
//! * **Per-statement liveness** — `live_before[block][stmt]` is the set of
//!   locals live just before `stmt` (statement index `N` is the terminator).
//!   The checker uses `live_before[block][stmt + 1]` as the *kill set* after
//!   processing statement `stmt`.
//! * **Per-local use counts** — a mutable borrow whose destination local is
//!   used exactly once, as a direct call argument, is a *two-phase borrow*
//!   (reserved at creation, activated at the call).
//! * **Call-argument use marking** — locals used directly as call arguments.

use crate::check::FunctionData;
use crate::place;
use nitrate_mir::{Operand, Place, Rvalue, Statement, Terminator};

// ─────────────────────────────────────────────────────────────
// A small dense bitset
// ─────────────────────────────────────────────────────────────

/// A dense bitset over local indices, implemented as a fixed-size vector of
/// machine words. Local counts are small, so this is fast and allocation-light.
#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct BitSet {
    words: Vec<u64>,
}

impl BitSet {
    #[must_use]
    pub fn with_capacity(nlocals: usize) -> Self {
        Self { words: vec![0; nlocals.div_ceil(64)] }
    }

    pub fn insert(&mut self, idx: usize) {
        let (word, bit) = (idx / 64, idx % 64);
        if word >= self.words.len() {
            self.words.resize(word + 1, 0);
        }
        self.words[word] |= 1 << bit;
    }

    #[must_use]
    pub fn contains(&self, idx: usize) -> bool {
        let (word, bit) = (idx / 64, idx % 64);
        word < self.words.len() && (self.words[word] & (1 << bit)) != 0
    }

    pub fn union_with(&mut self, other: &BitSet) {
        if other.words.len() > self.words.len() {
            self.words.resize(other.words.len(), 0);
        }
        for (a, b) in self.words.iter_mut().zip(&other.words) {
            *a |= *b;
        }
    }

    pub fn subtract(&mut self, other: &BitSet) {
        for (a, b) in self.words.iter_mut().zip(&other.words) {
            *a &= !*b;
        }
    }
}


// ─────────────────────────────────────────────────────────────
// Liveness result
// ─────────────────────────────────────────────────────────────

/// The results of the liveness pass for one function.
pub struct LivenessInfo {
    /// `live_before[block][stmt]` — locals live just before `stmt`.
    /// `stmt` ranges `0..=num_statements(block)`; the last index is the
    /// terminator.
    pub live_before: Vec<Vec<BitSet>>,
    /// `live_out[block]` — locals live just after the terminator.
    pub live_out: Vec<BitSet>,
    /// Number of uses of each local (by positional index).
    pub use_counts: Vec<usize>,
    /// Locals (by positional index) used directly as a call argument.
    pub call_arg_uses: BitSet,
}

struct UseDef {
    uses: BitSet,
    defs: BitSet,
}

impl UseDef {
    fn new(nlocals: usize) -> Self {
        Self { uses: BitSet::with_capacity(nlocals), defs: BitSet::with_capacity(nlocals) }
    }
}

/// Compute the liveness analysis for the given function snapshot.
pub fn compute_liveness(function: &FunctionData) -> LivenessInfo {
    let nlocals = function.locals.len();
    let nblocks = function.blocks.len();

    let mut use_counts = vec![0usize; nlocals];
    let mut call_arg_uses = BitSet::with_capacity(nlocals);

    // Pre-compute per-statement use/def sets.
    let mut stmt_usedefs: Vec<Vec<UseDef>> = Vec::with_capacity(nblocks);
    for block in &function.blocks {
        let mut defs = Vec::with_capacity(block.statements.len() + 1);
        for stmt in &block.statements {
            let mut ud = UseDef::new(nlocals);
            collect_stmt_usedef(stmt, &mut ud, &mut use_counts, &mut call_arg_uses, function);
            defs.push(ud);
        }
        let mut term_ud = UseDef::new(nlocals);
        collect_term_usedef(&block.terminator, &mut term_ud, &mut use_counts, &mut call_arg_uses, function);
        defs.push(term_ud);
        stmt_usedefs.push(defs);
    }

    // Backward dataflow over the CFG to a fixed point.
    let mut live_in: Vec<BitSet> = vec![BitSet::with_capacity(nlocals); nblocks];
    let mut live_out: Vec<BitSet> = vec![BitSet::with_capacity(nlocals); nblocks];
    let mut live_before: Vec<Vec<BitSet>> = vec![Vec::new(); nblocks];

    loop {
        let mut changed = false;
        for b in 0..nblocks {
            // live_out[b] = union of live_in[succ]
            let mut new_out = BitSet::with_capacity(nlocals);
            for succ in function.successors(b) {
                new_out.union_with(&live_in[succ]);
            }
            if new_out != live_out[b] {
                live_out[b] = new_out;
                changed = true;
            }

            // Backward pass within the block to recompute per-statement liveness.
            let n = function.blocks[b].statements.len();
            let mut per_stmt = vec![BitSet::with_capacity(nlocals); n + 1];
            let mut live = live_out[b].clone();
            for s in (0..=n).rev() {
                // live_before[s] = uses[s] ∪ (live_after[s] − defs[s])
                let mut before = stmt_usedefs[b][s].uses.clone();
                before.union_with(&live);
                before.subtract(&stmt_usedefs[b][s].defs);
                per_stmt[s] = before.clone();
                live = before;
            }
            live_in[b] = live;

            if live_before[b] != per_stmt {
                live_before[b] = per_stmt;
                changed = true;
            }
        }
        if !changed {
            break;
        }
    }

    LivenessInfo { live_before, live_out, use_counts, call_arg_uses }
}


// ─────────────────────────────────────────────────────────────
// Use/def collection helpers
// ─────────────────────────────────────────────────────────────

fn collect_stmt_usedef(
    stmt: &Statement,
    ud: &mut UseDef,
    use_counts: &mut [usize],
    call_arg_uses: &mut BitSet,
    function: &FunctionData,
) {
    match stmt {
        Statement::Assign(dest, rvalue) => {
            if let Place::Local(id) = dest {
                ud.defs.insert(function.idx_of(id));
            }
            collect_rvalue_usedef(rvalue, ud, use_counts, call_arg_uses, function);
        }
        Statement::SetDiscriminant { place, .. } => {
            if let Place::Local(id) = place {
                ud.defs.insert(function.idx_of(id));
            } else {
                // Projection writes still need their base/index locals live.
                collect_place_locals(place, ud, use_counts, function);
            }
        }
        Statement::StorageLive(_) | Statement::StorageDead(_) => {}
    }
}

fn collect_rvalue_usedef(
    rvalue: &Rvalue,
    ud: &mut UseDef,
    use_counts: &mut [usize],
    call_arg_uses: &mut BitSet,
    function: &FunctionData,
) {
    match rvalue {
        Rvalue::Use(op) => collect_operand_usedef(op, ud, use_counts, call_arg_uses, function),
        Rvalue::Ref { .. } => {
            // The borrow target place is not a read; nothing to record.
        }
        Rvalue::Len(place) => collect_place_locals(place, ud, use_counts, function),
        Rvalue::Cast { value, .. } => collect_operand_usedef(value, ud, use_counts, call_arg_uses, function),
        Rvalue::BinaryOp { lhs, rhs, .. } | Rvalue::CheckedBinaryOp { lhs, rhs, .. } => {
            collect_operand_usedef(lhs, ud, use_counts, call_arg_uses, function);
            collect_operand_usedef(rhs, ud, use_counts, call_arg_uses, function);
        }
        Rvalue::UnaryOp { operand, .. } => {
            collect_operand_usedef(operand, ud, use_counts, call_arg_uses, function);
        }
        Rvalue::NullaryOp(..) => {}
        Rvalue::Aggregate(_, operands) => {
            for op in operands {
                collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
            }
        }
    }
}

fn collect_term_usedef(
    term: &Terminator,
    ud: &mut UseDef,
    use_counts: &mut [usize],
    call_arg_uses: &mut BitSet,
    function: &FunctionData,
) {
    match term {
        Terminator::Goto { args, .. } => {
            for op in args {
                collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
            }
        }
        Terminator::If { condition, true_args, false_args, .. } => {
            collect_operand_usedef(condition, ud, use_counts, call_arg_uses, function);
            for op in true_args.iter().chain(false_args.iter()) {
                collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
            }
        }
        Terminator::SwitchInt { discr, targets, otherwise_args, .. } => {
            collect_operand_usedef(discr, ud, use_counts, call_arg_uses, function);
            for (_, _, args) in targets {
                for op in args {
                    collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
                }
            }
            for op in otherwise_args {
                collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
            }
        }
        Terminator::Return { value } => {
            if let Some(op) = value {
                collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
            }
        }
        Terminator::Unreachable => {}
        Terminator::Call { callee, args, destination, target_args, .. } => {
            collect_operand_usedef(callee, ud, use_counts, call_arg_uses, function);
            for op in args {
                collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
                if let Operand::Copy(place) | Operand::Move(place) = op {
                    mark_place_call_arg(place, call_arg_uses, function);
                }
            }
            if let Some(dest) = destination {
                if let Place::Local(id) = dest {
                    ud.defs.insert(function.idx_of(id));
                } else {
                    collect_place_locals(dest, ud, use_counts, function);
                }
            }
            for op in target_args {
                collect_operand_usedef(op, ud, use_counts, call_arg_uses, function);
            }
        }
    }
}

fn collect_operand_usedef(
    op: &Operand,
    ud: &mut UseDef,
    use_counts: &mut [usize],
    _call_arg_uses: &mut BitSet,
    function: &FunctionData,
) {
    match op {
        Operand::Copy(place) | Operand::Move(place) => collect_place_locals(place, ud, use_counts, function),
        Operand::Constant(_) => {}
    }
}

fn mark_place_call_arg(place: &Place, call_arg_uses: &mut BitSet, function: &FunctionData) {
    // Only the root local of a direct place argument is a "call argument use".
    if let Place::Local(id) = place::root_place(place) {
        call_arg_uses.insert(function.idx_of(id));
    }
}

/// Collect every local whose value is needed to evaluate `place` (the root,
/// plus any index operands).
fn collect_place_locals(place: &Place, ud: &mut UseDef, use_counts: &mut [usize], function: &FunctionData) {
    match place {
        Place::Local(id) => {
            let idx = function.idx_of(id);
            ud.uses.insert(idx);
            use_counts[idx] += 1;
        }
        Place::Static(_) => {}
        Place::Deref(base) | Place::Field { base, .. } | Place::Downcast { base, .. } => {
            collect_place_locals(base, ud, use_counts, function);
        }
        Place::Index { base, index } => {
            collect_place_locals(base, ud, use_counts, function);
            collect_place_locals(index, ud, use_counts, function);
        }
    }
}
