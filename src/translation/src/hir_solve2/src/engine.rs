//! Inference engine: orchestrates the solving pipeline for functions and globals.
//!
//! The engine runs a fixed-point loop:
//! 1. Walk the expression tree, collecting constraints
//! 2. Solve equality constraints (unification)
//! 3. Rewrite inferred literals and range expressions
//! 4. Monomorphize generic call sites
//! 5. Repeat until no new constraints or monomorphizations are produced

use crate::constraints::ConstraintGraph;
use crate::diagnosis::TypeErr;
use crate::monomorphize::Monomorphizer;
use crate::rewrite;
use crate::walk::{self, NodeTypes};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{BlockElement, Function, GlobalVariable, SymbolTab};
use nitrate_hir_evaluate::Evaluator;
use nitrate_hir_type::HirGetType;
use std::collections::HashSet;

/// Solve type inference for a function body.
///
/// Runs the constraint collection → unification → rewriting → monomorphization
/// pipeline in a fixed-point loop until convergence.
pub(crate) fn solve_function(func: &mut Function, symbol_tab: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    crate::range::ensure_range_structs(symbol_tab);

    let mut graph = ConstraintGraph::new();
    let mut node_types = NodeTypes::new();
    let mut mono = Monomorphizer::new();
    let mut errors: HashSet<TypeErr> = HashSet::new();

    // Resolve UnresolvedArray/UnresolvedRefine types in signature.
    resolve_unresolved_types_in_function(func, symbol_tab, log);

    if func.body.is_none() {
        return Ok(());
    }

    // Fixed-point loop.
    loop {
        let prev_mono_count = mono.counter;

        // Phase 1: Walk the body, collect constraints.
        walk::walk_function(&mut graph, &mut node_types, func, symbol_tab, &mut mono, &mut errors);

        // Phase 2: Solve equality constraints.
        graph.solve_equalities();

        // Collect any unification errors.
        for err in graph.drain_errors() {
            errors.insert(err);
        }

        // Phase 3: Rewrite inferred literals and range expressions.
        if let Some(ref mut body) = func.body {
            rewrite::rewrite_body(body, &mut node_types, &mut graph, symbol_tab, &mut errors);
        }

        // Phase 4: Check for convergence.
        if mono.counter == prev_mono_count {
            break;
        }
    }

    // Resolve UnresolvedArray/UnresolvedRefine types in local variable declarations.
    if let Some(ref mut body) = func.body {
        resolve_unresolved_types_in_body(body, symbol_tab, log);
    }

    // Report accumulated errors.
    for error in &errors {
        log.report(error);
    }

    if errors.is_empty() { Ok(()) } else { Err(()) }
}

/// Solve type inference for a global variable initializer.
pub(crate) fn solve_global(
    global: &mut GlobalVariable,
    symbol_tab: &mut SymbolTab,
    log: &CompilerLog,
) -> Result<(), ()> {
    crate::range::ensure_range_structs(symbol_tab);

    let mut graph = ConstraintGraph::new();
    let mut node_types = NodeTypes::new();
    let mut mono = Monomorphizer::new();
    let mut errors: HashSet<TypeErr> = HashSet::new();

    // Resolve UnresolvedArray/UnresolvedRefine in global type.
    global.ty = resolve_type(&global.ty, symbol_tab, log);

    // Walk and solve.
    walk::walk_global(&mut graph, &mut node_types, global, symbol_tab, &mut mono, &mut errors);
    graph.solve_equalities();

    for err in graph.drain_errors() {
        errors.insert(err);
    }

    // Resolve the initializer's type.
    if global.ty.is_inferred() {
        if let Some(resolved) = node_types.resolve(&global.initializer, &mut graph) {
            global.ty = resolved;
        }
    }

    // Report errors.
    for error in &errors {
        log.report(error);
    }

    if errors.is_empty() { Ok(()) } else { Err(()) }
}

/// Resolve `UnresolvedArray`/`UnresolvedRefine` embedded `ValueId` expressions
/// and replace them with resolved concrete `Type`.
fn resolve_type(ty: &nitrate_hir::TypeId, symbol_tab: &SymbolTab, log: &CompilerLog) -> nitrate_hir::TypeId {
    use nitrate_hir::{LiteralId, Type, TypeId, get_storage};
    use std::ops::Deref;

    let span = ty.span();
    match &*ty.deref() {
        Type::UnresolvedArray { element_type, len, .. } => {
            let mut evaluator = Evaluator::new(log, symbol_tab.arch_ptr_size());
            match evaluator.evaluate_to_literal(&len.borrow()) {
                Ok(lit) => {
                    let len_u32 = nitrate_hir_type::lit_to_u128(&lit)
                        .and_then(|v| u32::try_from(v).ok())
                        .unwrap_or(0);
                    Type::Array {
                        span,
                        element_type: resolve_type(element_type, symbol_tab, log),
                        len: len_u32,
                    }
                    .into()
                }
                Err(_) => ty.clone(),
            }
        }
        Type::UnresolvedRefine { base, min, max, .. } => {
            let mut evaluator = Evaluator::new(log, symbol_tab.arch_ptr_size());
            let min_lit = evaluator
                .evaluate_to_literal(&min.borrow())
                .ok()
                .map(|lit| LiteralId::from(get_storage(|store| store.store_literal(lit))));
            let max_lit = evaluator
                .evaluate_to_literal(&max.borrow())
                .ok()
                .map(|lit| LiteralId::from(get_storage(|store| store.store_literal(lit))));
            let resolved_base = resolve_type(base, symbol_tab, log);

            match (min_lit, max_lit) {
                (Some(min), Some(max)) => Type::Refine {
                    span,
                    base: resolved_base,
                    min,
                    max,
                }
                .into(),
                _ => ty.clone(),
            }
        }
        Type::Array { element_type, .. } => {
            let resolved_elem = resolve_type(element_type, symbol_tab, log);
            if resolved_elem.as_usize() != element_type.as_usize() {
                Type::Array {
                    span,
                    element_type: resolved_elem,
                    len: match &**ty {
                        Type::Array { len, .. } => *len,
                        _ => 0,
                    },
                }
                .into()
            } else {
                ty.clone()
            }
        }
        Type::Parameterized { base, args, .. } => {
            let resolved_base = resolve_type(base, symbol_tab, log);
            let resolved_args: Vec<TypeId> = args
                .positional
                .iter()
                .map(|a| resolve_type(a, symbol_tab, log))
                .collect();
            Type::Parameterized {
                span,
                base: resolved_base,
                args: nitrate_hir::Arguments {
                    positional: resolved_args.into(),
                    named: args.named.clone(),
                },
            }
            .into()
        }
        Type::Tuple { element_types, .. } => {
            let resolved: Vec<TypeId> = element_types
                .iter()
                .map(|et| resolve_type(et, symbol_tab, log))
                .collect();
            Type::Tuple {
                span,
                element_types: resolved.into(),
            }
            .into()
        }
        Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
            ..
        } => {
            let resolved_to = resolve_type(to, symbol_tab, log);
            if resolved_to.as_usize() != to.as_usize() {
                Type::Reference {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: resolved_to,
                }
                .into()
            } else {
                ty.clone()
            }
        }
        Type::SliceRef {
            lifetime,
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let resolved_et = resolve_type(element_type, symbol_tab, log);
            if resolved_et.as_usize() != element_type.as_usize() {
                Type::SliceRef {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: resolved_et,
                }
                .into()
            } else {
                ty.clone()
            }
        }
        Type::Pointer {
            lifetime,
            exclusive,
            mutable,
            to,
            ..
        } => {
            let resolved_to = resolve_type(to, symbol_tab, log);
            if resolved_to.as_usize() != to.as_usize() {
                Type::Pointer {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: resolved_to,
                }
                .into()
            } else {
                ty.clone()
            }
        }
        Type::SlicePtr {
            lifetime,
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let resolved_et = resolve_type(element_type, symbol_tab, log);
            if resolved_et.as_usize() != element_type.as_usize() {
                Type::SlicePtr {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: resolved_et,
                }
                .into()
            } else {
                ty.clone()
            }
        }
        _ => ty.clone(),
    }
}

/// Resolve `UnresolvedArray`/`UnresolvedRefine` types in function parameters and return type.
fn resolve_unresolved_types_in_function(func: &mut Function, symbol_tab: &SymbolTab, log: &CompilerLog) {
    func.return_type = resolve_type(&func.return_type, symbol_tab, log);

    for param_id in &func.params {
        let mut param = param_id.borrow_mut();
        param.ty = resolve_type(&param.ty, symbol_tab, log);
    }
}

/// Resolve `UnresolvedArray`/`UnresolvedRefine` types in local variable types within a body.
fn resolve_unresolved_types_in_body(body: &mut [BlockElement], symbol_tab: &SymbolTab, log: &CompilerLog) {
    for element in body {
        match element {
            BlockElement::Local(local_var) => {
                let mut lv = local_var.borrow_mut();
                lv.ty = resolve_type(&lv.ty, symbol_tab, log);
            }
            _ => {}
        }
    }
}
