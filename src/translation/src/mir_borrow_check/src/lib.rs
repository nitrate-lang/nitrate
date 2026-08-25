#![forbid(unsafe_code)]
//! # Nitrate MIR Borrow Checker
//!
//! A comprehensive non-lexical-lifetime (NLL) borrow checker that operates on
//! the MIR (Mid-level IR). It enforces memory safety for safe code by checking
//! the ownership rules — aliasing XOR mutation, no dangling references, and no
//! use of moved/uninitialized values — using a liveness-based dataflow
//! analysis.
//!
//! ## Why MIR?
//!
//! The HIR is a tree-structured representation where expression nesting hides
//! the actual order of evaluation and the precise lifetime of temporaries. MIR
//! flattens expressions into a control-flow graph of basic blocks with flat
//! statements, explicit temporaries, `StorageLive`/`StorageDead` markers, and
//! `Copy`/`Move` operands. These are exactly the ingredients NLL needs:
//!
//! * **Liveness** of locals (computed on the CFG) gives borrow regions: a
//!   borrow lives from its creation point to the last use of the local that
//!   holds it — no lexical scope required.
//! * **Explicit temporaries** make borrow values first-class, so borrow
//!   *holders* can be tracked through copies and block arguments.
//! * **Control-flow edges** make two-phase borrows (reservation then
//!   activation at a call) precise and loop-carried borrows (which must not
//!   escape the loop) detectable.
//!
//! ## Algorithm
//!
//! 1. **Liveness** (`liveness` module): per-statement backward dataflow,
//!    computing the set of live locals at every statement boundary.
//! 2. **Borrow regions** (`check` module): a forward dataflow over the CFG
//!    maintains the set of active borrows. Each borrow carries a set of
//!    *holders* — the places currently storing its value. A holder dies when
//!    (a) its local is no longer live (NLL last-use), (b) its local is
//!    reassigned, or (c) its storage is dead. When all holders die, the
//!    borrow dies.
//! 3. **Conflict detection**: at every statement, every read, write, move, and
//!    borrow creation is checked against the active borrows using the place
//!    overlap relation (`Deref` boundaries delimit memory regions).
//! 4. **Two-phase borrows**: a `&mut` borrow whose destination local is used
//!    exactly once — as a direct call argument — is *reserved* at creation
//!    (reads and shared borrows of the place are allowed) and *activated* at
//!    the call (exclusivity enforced against every other active borrow).
//! 5. **Escaping borrows**: a borrow live at a `return` whose source is a
//!    local (not a static or a reborrow) is rejected.
//! 6. **Move/init tracking**: `Move` operands deinitialize their place; reads
//!    of moved-from or never-initialized places are rejected.
//!
//! ## Soundness notes
//!
//! The analysis is a *may* (union) dataflow, so it over-approximates borrow
//! activity and moved/uninitialized state: it may reject valid programs but
//! never accepts memory-unsafe ones. Known over-approximations/limitations:
//!
//! * Drop-liveness is not modeled (MIR has no drops), so a borrow dies at its
//!   last use even if the holding local's lexical scope extends further. This
//!   is sound (unused references are dead) and strictly more permissive than
//!   lexical borrow checking.
//! * Points-to aliasing through copied references is not tracked (place-based
//!   analysis); full soundness there requires move-only `&mut T`, which the
//!   MIR lowering does not yet enforce.
//! * A place that is a suffix of a moved place is tracked as moved; partial
//!   moves of individual fields are tracked per-place, but reading a whole
//!   struct after one field was moved is not rejected (the MIR emits no moves
//!   today).

mod check;
pub mod diagnosis;
mod liveness;
mod place;

#[cfg(test)]
mod tests;

use nitrate_diagnosis::CompilerLog;
use nitrate_mir::MirFunction;
use nitrate_mir::MirModule;

/// Check every function in the MIR module for borrow violations.
///
/// All diagnostics are reported to `log`. Returns `Ok(())` if no violations
/// were found, `Err(())` otherwise.
///
/// **Requires**: the `MirStore` must be installed in TLS (via
/// `nitrate_mir::using_storage`) so that `MirTypeId` dereferences resolve.
pub fn check_mir_module(module: &MirModule, log: &CompilerLog) -> Result<(), ()> {
    let mut had_errors = false;
    for func_id in &module.functions {
        let func = nitrate_mir::get_storage(|_store| (*func_id).borrow().clone());
        if func.body.is_some() && check_mir_function(&func, module, log).is_err() {
            had_errors = true;
        }
    }
    if had_errors { Err(()) } else { Ok(()) }
}

/// Check a single MIR function for borrow violations.
///
/// **Requires**: the `MirStore` must be installed in TLS (via
/// `nitrate_mir::using_storage`) so that `MirTypeId` dereferences resolve.
pub fn check_mir_function(function: &MirFunction, module: &MirModule, log: &CompilerLog) -> Result<(), ()> {
    let errors = collect_errors(function, module);
    for error in &errors {
        log.report(error);
    }
    if errors.is_empty() { Ok(()) } else { Err(()) }
}

/// Run the borrow checker on a single function and return the violations
/// without reporting them to a log.
///
/// **Requires**: the `MirStore` must be installed in TLS (via
/// `nitrate_mir::using_storage`) so that `MirTypeId` dereferences resolve.
pub fn collect_errors(function: &MirFunction, module: &MirModule) -> Vec<diagnosis::BorrowError> {
    if function.body.is_none() {
        // Extern functions have no body to check.
        return Vec::new();
    }
    let data = nitrate_mir::get_storage(|store| check::FunctionData::snapshot(function, store))
        .expect("failed to snapshot MIR function body for borrow checking");
    let log = CompilerLog::default();
    let mut checker = check::BorrowChecker::new(data, module, &log);
    checker.run();
    checker.errors
}
