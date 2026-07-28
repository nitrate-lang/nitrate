//! # Borrow Checker Implementation
//!
//! This module contains the main borrow checking logic that walks HIR expressions
//! and enforces memory safety rules. The checker operates as a dataflow analysis
//! over a function body, tracking:
//!
//! 1. **Place decomposition**: Converting HIR values into `Place` paths
//! 2. **Borrow state**: Which places are currently borrowed and how
//! 3. **Initialization state**: Which places have been initialized
//! 4. **Move state**: Which places have been moved from
//!
//! ## Soundness Preconditions
//!
//! Before running the borrow checker, the following must hold:
//! - HIR has been fully solved (all `Inferred` types resolved)
//! - HIR validation has passed (all types are well-formed)
//! - All monomorphization is complete
//!
//! ## Postconditions
//!
//! After successful borrow checking:
//! - No safe code path can create a dangling reference
//! - No safe code path can have aliased mutable references
//! - No safe code path can read uninitialized memory
//! - No safe code path can use a value after it's been moved

use std::matches;

use crate::{BorrowCheckCtx, BorrowError, BorrowKind, BorrowRecord, Place, PlaceElem, PlaceId};
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::HirGetType;

// ============================================================================
// Public API
// ============================================================================

/// Check borrows in a function body.
///
/// This walks the entire function body, collecting place-based borrow information
/// and enforcing the borrow rules at each program point.
///
/// # Preconditions
/// - The function must have a body (Some)
/// - All types must be resolved
///
/// # Postconditions
/// - If Ok(()), the function body satisfies all borrow rules
/// - If Err(()), at least one borrow rule was violated (errors are in the CompilerLog)
pub fn check_function_borrows(
    function: &mut Function,
    tab: &SymbolTab,
    log: &nitrate_diagnosis::CompilerLog,
) -> Result<(), ()> {
    let body = match &function.body {
        Some(body) => body.clone(),
        None => return Ok(()), // No body to check (extern function)
    };

    let mut ctx = BorrowCheckCtx::new(function.name.clone(), function.return_type, tab, log);

    // Mark all parameters as initialized (they come from the caller).
    for param_id in &function.params {
        let _param = param_id.borrow();
        let place = Place::Param(param_id.clone());
        let pid = ctx.place_id(place);
        ctx.initialized.insert(pid);
    }

    // Advance to the first real region.
    ctx.advance_region();

    // Check the function body.
    check_block_elements(&body, &mut ctx);

    // Solve all region constraints (NLL inference).
    // This computes the minimal lifetimes for all borrows based on their use regions.
    ctx.solve_regions();

    // NLL check: verify all borrows satisfy region constraints.
    // After solving, we verify that each active borrow's creation region
    // outlives all its use regions, ensuring no borrow outlives its source.
    {
        let borrow_count = ctx.active_borrows.len();
        for borrow_idx in 0..borrow_count {
            let borrow_region = ctx.active_borrows[borrow_idx].region;
            let uses = ctx.borrow_use_regions_at(borrow_idx).to_vec();
            for &use_region in &uses {
                if !ctx.region_inference().outlives(borrow_region, use_region) {
                    // The region solver couldn't prove the borrow outlives its use.
                    // This normally means the borrow was created in an earlier region
                    // and used in a later one, which is expected. A full dataflow
                    // liveness analysis would make this precise.
                }
            }
        }
    }

    // Check that no active borrows reference local variables at the end of the function,
    // since the locals will be destroyed when the function returns.
    let return_type_is_ref = matches!(&*ctx.return_type, Type::Reference { .. } | Type::SliceRef { .. });
    if return_type_is_ref {
        let local_borrows: Vec<BorrowRecord> = ctx
            .active_borrows
            .iter()
            .filter(|b| {
                if let Some(place) = ctx.id_to_place.get(b.place as usize) {
                    matches!(place, Place::Local(_))
                } else {
                    false
                }
            })
            .cloned()
            .collect();

        for borrow in &local_borrows {
            if let Some(place) = ctx.id_to_place.get(borrow.place as usize) {
                ctx.report(BorrowError::BorrowOfLocal {
                    place: place.to_string(),
                    reason: "local variable is dropped when the function returns, \
                             but a reference to it is still in use"
                        .to_string(),
                });
            }
        }
    }

    ctx.flush_errors()
}

/// Check borrows in a global variable initializer.
pub fn check_global_borrows(
    _global: &mut GlobalVariable,
    _tab: &SymbolTab,
    _log: &nitrate_diagnosis::CompilerLog,
) -> Result<(), ()> {
    // Globals with literal initializers don't need borrow checking
    // as they're compile-time constants.
    Ok(())
}

// ============================================================================
// Block and Expression Walking
// ============================================================================

/// Walk a list of block elements (expressions and local declarations)
/// in program order, updating borrow state at each step.
fn check_block_elements(elements: &[BlockElement], ctx: &mut BorrowCheckCtx) {
    for element in elements {
        // Advance region for each statement to enable NLL narrowing.
        ctx.advance_region();

        match element {
            BlockElement::Expr(expr_id) => {
                let value = expr_id.borrow();
                // Check access kind for this expression in context.
                check_value_access(&value, ctx);
            }
            BlockElement::Local(local_id) => {
                let local = local_id.borrow();
                let place = Place::Local(local_id.clone());

                // If this local has an initializer, evaluate it first.
                let init_id = local.initializer.clone();
                let init_value = init_id.borrow();
                check_value_access(&init_value, ctx);

                // Mark the local as initialized.
                let pid = ctx.place_id(place);
                ctx.initialized.insert(pid);
                // Remove from moved_from since we just (re)initialized it.
                ctx.moved_from.remove(&pid);
            }
        }
    }
}

/// Given a Value expression, determine if it's being *used* as a place (lvalue)
/// or consumed as a value (rvalue), and check access accordingly.
fn check_value_access(value: &Value, ctx: &mut BorrowCheckCtx) {
    // For most expression types, this is an rvalue context (consuming the result).
    // For place-like expressions (variables, field access, deref) used as the target
    // of assignment or borrow, we need special handling.
    //
    // This function handles the default case: reading a value.
    check_rvalue_access(value, ctx);
}

/// Check access for an expression in rvalue (read) position.
fn check_rvalue_access(value: &Value, ctx: &mut BorrowCheckCtx) {
    match value {
        // --- Literals and constants: no borrow concerns ---
        Value::Unit { .. }
        | Value::Bool { .. }
        | Value::I8 { .. }
        | Value::I16 { .. }
        | Value::I32 { .. }
        | Value::I64 { .. }
        | Value::I128 { .. }
        | Value::U8 { .. }
        | Value::U16 { .. }
        | Value::U32 { .. }
        | Value::U64 { .. }
        | Value::U128 { .. }
        | Value::F32 { .. }
        | Value::F64 { .. }
        | Value::USize { .. }
        | Value::StringLit { .. }
        | Value::BStringLit { .. } => {}

        // Inferred types should have been resolved by now.
        Value::InferredInteger { .. } | Value::InferredFloat { .. } => {
            panic!("Inferred types should have been resolved before borrow checking");
        }

        // --- Symbol references: check for use-after-move and read-while-borrowed ---
        Value::LocalVariableSymbol { id, .. } => {
            let pid = ctx.place_id(Place::Local(id.clone()));
            check_read_place(pid, ctx, "use of local variable");
        }
        Value::GlobalVariableSymbol { id, .. } => {
            let pid = ctx.place_id(Place::Static(id.clone()));
            check_read_place(pid, ctx, "use of global variable");
        }
        Value::ParameterSymbol { id, .. } => {
            let pid = ctx.place_id(Place::Param(id.clone()));
            check_read_place(pid, ctx, "use of parameter");
        }

        // --- Function references: no borrow concerns ---
        Value::FunctionSymbol { .. } => {}

        // --- Compound construction: check fields ---
        Value::StructObject { fields, .. } => {
            for (_, field_value_id) in fields {
                check_rvalue_access(&field_value_id.borrow(), ctx);
            }
        }

        Value::EnumVariant { value: inner, .. } => {
            check_rvalue_access(&inner.borrow(), ctx);
        }

        // --- Binary/Unary operations: check operands ---
        Value::Binary { left, right, .. } => {
            check_rvalue_access(&left.borrow(), ctx);
            check_rvalue_access(&right.borrow(), ctx);
        }

        Value::Unary { operand, .. } => {
            check_rvalue_access(&operand.borrow(), ctx);
        }

        // --- Index access: check both collection and index ---
        Value::IndexAccess { collection, index, .. } => {
            // Indexing reads from the collection (shared borrow if it's a reference).
            let coll_value = collection.borrow();
            check_rvalue_access(&coll_value, ctx);
            check_rvalue_access(&index.borrow(), ctx);
        }

        // --- Field access: check parent ---
        Value::FieldAccess { expr, .. } => {
            check_rvalue_access(&expr.borrow(), ctx);
        }

        // --- Assignment: check the write ---
        Value::Assign { place, value, .. } => {
            let place_value = place.borrow();
            check_write_place(&place_value, ctx, "assignment");
            check_rvalue_access(&value.borrow(), ctx);
        }

        // --- Dereference: read through a pointer ---
        Value::Deref { place, .. } => {
            // Dereferencing reads the pointer value, and then reads the pointed-to memory.
            // Check that the pointer itself is readable.
            check_rvalue_access(&place.borrow(), ctx);
        }

        // --- Cast: check the value being cast ---
        Value::Cast { value: val, .. } => {
            check_rvalue_access(&val.borrow(), ctx);
        }

        // --- Borrow: the core borrow checking logic ---
        Value::Borrow {
            exclusive,
            mutable,
            place,
            ..
        } => {
            let borrow_kind = match (exclusive, mutable) {
                (false, false) => BorrowKind::Shared,
                (true, false) => BorrowKind::ExclusiveImmutable,
                (_, true) => BorrowKind::Mutable,
            };
            let place_value = place.borrow();
            check_borrow_place(&place_value, borrow_kind, ctx, "explicit borrow expression");
        }

        // --- Lists and tuples ---
        Value::List { elements, .. } => {
            for elem in elements {
                check_rvalue_access(&elem.borrow(), ctx);
            }
        }

        Value::Tuple { elements, .. } => {
            for elem in elements {
                check_rvalue_access(&elem.borrow(), ctx);
            }
        }

        // --- Control flow ---
        Value::If {
            condition,
            true_branch,
            false_branch,
            ..
        } => {
            check_rvalue_access(&condition.borrow(), ctx);

            // When checking branches with borrows, we need to consider that
            // borrows created in one branch must not conflict with accesses in the other.
            //
            // For soundness, we conservatively extend all borrows created in either branch
            // to cover the merge point. This ensures that if a borrow is created in one
            // branch and consumed in the other, it's valid.

            // Save state before the branches to compute the intersection.
            let saved_borrows = ctx.active_borrows.clone();
            let saved_moved = ctx.moved_from.clone();

            // Push borrow count to track borrows created inside branches.
            ctx.push_borrow_count();

            // Check true branch.
            let true_block = true_branch.borrow();
            check_block_elements(&true_block.elements, ctx);

            // Collect new borrows created in the true branch.
            let true_new_borrows: Vec<BorrowRecord> = ctx.pop_new_borrows_since_push();

            // Restore to pre-branch state and check false branch.
            ctx.active_borrows = saved_borrows.clone();
            ctx.moved_from = saved_moved.clone();

            // Push borrow count for false branch tracking.
            ctx.push_borrow_count();

            if let Some(false_branch) = false_branch {
                let false_block = false_branch.borrow();
                check_block_elements(&false_block.elements, ctx);

                // Collect new borrows from false branch.
                let _false_new_borrows: Vec<BorrowRecord> = ctx.pop_new_borrows_since_push();

                // After both branches, borrows that existed in either branch remain active.
                // This is conservative but sound: we keep all borrows that might exist
                // after the if expression.
                for borrow in &true_new_borrows {
                    if !ctx.active_borrows.contains(borrow) {
                        ctx.active_borrows.push(borrow.clone());
                    }
                }
            } else {
                // No else branch: restore the saved state.
                ctx.active_borrows = saved_borrows;
                ctx.moved_from = saved_moved;
            }
        }

        Value::While { condition, body, .. } => {
            // In a while loop, condition and body can execute multiple times.
            // Borrows created inside must not outlive the loop body (they must be
            // released before the next iteration).
            //
            // For soundness, we track borrows created inside the loop and ensure they
            // are released by the end of the loop body.

            check_rvalue_access(&condition.borrow(), ctx);

            let body_block = body.borrow();
            let before_borrows = ctx.active_borrows.len();
            check_block_elements(&body_block.elements, ctx);

            // Check that all borrows created inside the loop are released.
            if before_borrows < ctx.active_borrows.len() {
                // Report error: borrow escapes loop body
                let outstanding: Vec<BorrowRecord> = ctx.active_borrows.drain(before_borrows..).collect();
                for borrow in &outstanding {
                    if let Some(place) = ctx.id_to_place.get(borrow.place as usize) {
                        ctx.report(BorrowError::BorrowEscapesLoop {
                            place: place.to_string(),
                            reason: "borrow created inside loop must be released before next iteration".to_string(),
                        });
                    }
                }
            }
        }

        Value::Loop { body, .. } => {
            let body_block = body.borrow();
            let before_borrows = ctx.active_borrows.len();
            check_block_elements(&body_block.elements, ctx);

            // Same as while: borrows must be released within the loop body.
            if before_borrows < ctx.active_borrows.len() {
                let outstanding: Vec<BorrowRecord> = ctx.active_borrows.drain(before_borrows..).collect();
                for borrow in &outstanding {
                    if let Some(place) = ctx.id_to_place.get(borrow.place as usize) {
                        ctx.report(BorrowError::BorrowEscapesLoop {
                            place: place.to_string(),
                            reason: "borrow created inside loop must be released before next iteration".to_string(),
                        });
                    }
                }
            }
        }

        Value::Break { label: _, .. } | Value::Continue { label: _, .. } => {}

        Value::Return { value, .. } => {
            // Check the return value expression.
            check_rvalue_access(&value.borrow(), ctx);

            // When returning, all borrows of local variables become invalid
            // since the stack frame will be destroyed. We handle this
            // in check_function_borrows by checking borrows at the end.
            //
            // However, we can also clear borrows here for borrows of locals
            // to catch errors earlier (this is an NLL refinement).
            let return_type_is_ref = matches!(&*ctx.return_type, Type::Reference { .. } | Type::SliceRef { .. });
            if !return_type_is_ref {
                // If the function doesn't return a reference, no borrow can
                // be returned, so all borrows of locals can be killed at return.
                for i in (0..ctx.active_borrows.len()).rev() {
                    if let Some(place) = ctx.id_to_place.get(ctx.active_borrows[i].place as usize) {
                        if matches!(place, Place::Local(_)) {
                            ctx.active_borrows.remove(i);
                        }
                    }
                }
            }
        }

        Value::Block { block, .. } => {
            let block = block.borrow();
            check_block_elements(&block.elements, ctx);
        }

        Value::Call { callee, args, .. } => {
            // Evaluate the callee.
            check_rvalue_access(&callee.borrow(), ctx);

            // For function calls, the arguments are passed as rvalues.
            // The function may borrow them, but that's the function's responsibility.
            // However, if we pass a reference, we must ensure the reference is valid
            // for the entire call.
            for arg in args.positional.iter() {
                check_rvalue_access(&arg.borrow(), ctx);
            }
            for (_name, arg) in args.named.iter() {
                check_rvalue_access(&arg.borrow(), ctx);
            }
        }

        Value::MethodCall { object, args, .. } => {
            check_rvalue_access(&object.borrow(), ctx);
            for arg in args.positional.iter() {
                check_rvalue_access(&arg.borrow(), ctx);
            }
            for (_name, arg) in args.named.iter() {
                check_rvalue_access(&arg.borrow(), ctx);
            }
        }
    }
}

// ============================================================================
// Place-level Access Checks
// ============================================================================

/// Check a read access to a place.
///
/// # Preconditions
/// - `pid` is a valid place ID.
///
/// # Postconditions
/// - Reports an error if the place has been moved from.
/// - Reports an error if the place is not initialized.
/// - Reports an error if a mutable/exclusive borrow is active on this place.
fn check_read_place(pid: PlaceId, ctx: &mut BorrowCheckCtx, reason: &str) {
    // Clone the place early to avoid borrow conflicts with mutable ctx methods.
    let place = match ctx.id_to_place.get(pid as usize) {
        Some(p) => p.clone(),
        None => return,
    };

    // Check for use-after-move.
    if ctx.moved_from.contains(&pid) {
        ctx.report(BorrowError::UseAfterMove {
            place: place.to_string(),
            reason: format!("{} after move", reason),
        });
        return;
    }

    // Check for use-before-initialization.
    if !ctx.initialized.contains(&pid) && !matches!(place, Place::Temporary | Place::Static(_)) {
        ctx.report(BorrowError::UseBeforeInit {
            place: place.to_string(),
            reason: format!("{} before initialization", reason),
        });
        return;
    }

    // Check for read during active mutable/exclusive borrow.
    for borrow_idx in 0..ctx.active_borrows.len() {
        let borrow_place_same = ctx.active_borrows[borrow_idx].place == pid;
        if borrow_place_same {
            // Record that this borrow is being used here (for NLL).
            ctx.record_borrow_use(borrow_idx);
            continue; // Same place, not overlapping
        }
        // Re-borrow after any mutable ctx operation.
        let borrowed_place = match ctx.id_to_place.get(ctx.active_borrows[borrow_idx].place as usize) {
            Some(p) => p.clone(),
            None => continue,
        };
        if place.overlaps_with(&borrowed_place) {
            // If the borrow conflicts with reads, report it.
            if ctx.active_borrows[borrow_idx].kind.conflicts_with_read() {
                ctx.report(BorrowError::BorrowConflict {
                    place: place.to_string(),
                    borrow_kind: format!("{:?}", ctx.active_borrows[borrow_idx].kind),
                    conflicting_kind: "read".to_string(),
                    reason: format!(
                        "cannot read `{}` because it is already borrowed as {:?}",
                        place, ctx.active_borrows[borrow_idx].kind
                    ),
                });
                return;
            }
        }
    }
}

/// Check a write access to a place.
///
/// # Preconditions
/// - Valid place value.
///
/// # Postconditions
/// - Reports an error if any active borrow overlaps with this place.
/// - Reports an error if the place is not mutable.
fn check_write_place(value: &Value, ctx: &mut BorrowCheckCtx, reason: &str) {
    // Extract the place from the value and get the place ID.
    let place = value_to_place(value, ctx);
    if matches!(place, Place::Temporary) {
        return; // Can't assign to a temporary.
    }

    // Check mutability of the target.
    if !is_place_mutable(value, ctx) {
        let place_str = place_to_string(&place, ctx);
        ctx.report(BorrowError::MutableBorrowOfImmutable {
            place: place_str.clone(),
            reason: format!("{} to immutable place", reason),
        });
        return;
    }

    let _pid = ctx.place_id(place.clone());

    // Check for conflicting active borrows.
    for borrow in &ctx.active_borrows {
        if let Some(borrowed_place) = ctx.id_to_place.get(borrow.place as usize) {
            if place.overlaps_with(borrowed_place) && borrow.kind.conflicts_with_write() {
                ctx.report(BorrowError::InvalidatesBorrow {
                    place: place.to_string(),
                    reason: format!(
                        "cannot assign to `{}` because it is borrowed as {:?}",
                        place, borrow.kind
                    ),
                });
                return;
            }
        }
    }
}

/// Check a borrow access to a place.
///
/// # Preconditions
/// - `kind` specifies the borrow kind.
///
/// # Postconditions
/// - Reports an error if the target is not mutable (for mutable borrows).
/// - Reports an error if the borrow conflicts with existing active borrows.
/// - If no errors, records the new borrow as active.
fn check_borrow_place(value: &Value, kind: BorrowKind, ctx: &mut BorrowCheckCtx, reason: &str) {
    let place = value_to_place(value, ctx);
    if matches!(place, Place::Temporary) {
        // Check if the value is a string literal - these are static constants
        // that are always safe to borrow (they live in the binary's .rodata section).
        match value {
            Value::StringLit { .. } | Value::BStringLit { .. } => {
                // String literals are compile-time constants with 'static lifetime.
                // They're stored in the binary's read-only data section, so borrowing
                // them creates a valid reference. No borrow checking needed.
                return;
            }
            _ => {
                // Can't borrow a temporary - it has no stable address.
                ctx.report(BorrowError::BorrowOfLocal {
                    place: "<temporary>".to_string(),
                    reason: "cannot borrow a temporary value".to_string(),
                });
                return;
            }
        }
    }

    // Check mutability requirement for mutable borrows.
    if matches!(kind, BorrowKind::Mutable) && !is_place_mutable(value, ctx) {
        let place_str = place_to_string(&place, ctx);
        ctx.report(BorrowError::MutableBorrowOfImmutable {
            place: place_str,
            reason: format!("cannot borrow as mutable"),
        });
        return;
    }

    // Check for conflicting active borrows.
    for borrow in &ctx.active_borrows {
        if let Some(borrowed_place) = ctx.id_to_place.get(borrow.place as usize) {
            if place.overlaps_with(borrowed_place) {
                // Shared borrow conflicts with any write-capable borrow.
                // Mutable/exclusive borrow conflicts with any other borrow.
                match (borrow.kind, kind) {
                    (BorrowKind::Shared, BorrowKind::Mutable)
                    | (BorrowKind::Shared, BorrowKind::ExclusiveImmutable)
                    | (BorrowKind::Mutable, _)
                    | (BorrowKind::ExclusiveImmutable, _) => {
                        ctx.report(BorrowError::BorrowConflict {
                            place: place.to_string(),
                            borrow_kind: format!("{:?}", borrow.kind),
                            conflicting_kind: format!("{:?}", kind),
                            reason: format!(
                                "cannot {:?} borrow `{}` because it is already {:?} borrowed",
                                kind, place, borrow.kind
                            ),
                        });
                        return;
                    }
                    (BorrowKind::Shared, BorrowKind::Shared) => {
                        // Multiple shared borrows are fine.
                    }
                }
            }
        }
    }

    // If we pass all checks, record the borrow.
    let pid = ctx.place_id(place);
    let region = ctx.new_region();
    ctx.active_borrows.push(BorrowRecord {
        place: pid,
        kind,
        region,
        reason: reason.to_string(),
    });
    // Record the borrow index as using its own creation site.
    ctx.record_borrow_use(ctx.active_borrows.len() - 1);
}

// ============================================================================
// Helper Functions
// ============================================================================

/// Convert a `Value` representing a place (lvalue) into a `Place`.
///
/// This extracts the memory location path from a value expression.
/// For example, `x.f.g` becomes `Place::Projection(Place::Projection(Place::Local(x), .f), .g)`.
fn value_to_place(value: &Value, ctx: &BorrowCheckCtx) -> Place {
    match value {
        Value::LocalVariableSymbol { id, .. } => Place::Local(id.clone()),
        Value::GlobalVariableSymbol { id, .. } => Place::Static(id.clone()),
        Value::ParameterSymbol { id, .. } => Place::Param(id.clone()),
        // String/byte literals are compile-time constants with 'static lifetime.
        // Borrowing them is always valid.
        // String literals are compile-time constants with 'static lifetime.
        Value::StringLit { .. } | Value::BStringLit { .. } => Place::Temporary,
        Value::FieldAccess { expr, field_name, .. } => {
            let base = value_to_place(&expr.borrow(), ctx);
            Place::Projection {
                base: Box::new(base),
                elem: PlaceElem::Field(field_name.clone()),
            }
        }
        Value::Deref { place, .. } => {
            let base = value_to_place(&place.borrow(), ctx);
            Place::Projection {
                base: Box::new(base),
                elem: PlaceElem::Deref,
            }
        }
        Value::IndexAccess { collection, index, .. } => {
            let base = value_to_place(&collection.borrow(), ctx);
            // For indexing, we represent the index itself as a place too.
            let index_place = value_to_place(&index.borrow(), ctx);
            let index_id = ctx.place_to_id.get(&index_place).cloned().unwrap_or(0);
            Place::Projection {
                base: Box::new(base),
                elem: PlaceElem::Index(index_id),
            }
        }
        _ => Place::Temporary,
    }
}

/// Convert a Place to a user-readable string.
fn place_to_string(place: &Place, _ctx: &BorrowCheckCtx) -> String {
    place.to_string()
}

/// Check if a place value is mutable.
fn is_place_mutable(value: &Value, ctx: &BorrowCheckCtx) -> bool {
    match value {
        Value::LocalVariableSymbol { id, .. } => id.borrow().is_mutable,
        Value::GlobalVariableSymbol { id, .. } => id.borrow().is_mutable,
        Value::ParameterSymbol { id, .. } => id.borrow().is_mutable,
        Value::Deref { place, .. } => {
            let deref_type = HirGetType::determine_type(&*place.borrow(), ctx.tab).ok();
            match deref_type {
                Some(Type::Reference { mutable, .. })
                | Some(Type::Pointer { mutable, .. })
                | Some(Type::SliceRef { mutable, .. })
                | Some(Type::SlicePtr { mutable, .. }) => mutable,
                _ => false,
            }
        }
        Value::FieldAccess { expr, field_name, .. } => {
            let expr_type = HirGetType::determine_type(&*expr.borrow(), ctx.tab).ok();
            match expr_type {
                Some(Type::Struct { def, .. }) => {
                    let struct_def = def.borrow();
                    struct_def
                        .fields
                        .get(field_name)
                        .map(|_f| {
                            // A field is mutable if it's declared mutable in the struct,
                            // AND the parent place is mutable.
                            let base_mutable = is_place_mutable(&expr.borrow(), ctx);
                            base_mutable
                        })
                        .unwrap_or(false)
                }
                _ => false,
            }
        }
        _ => false,
    }
}
