//! Main solver module: fixed-point constraint propagation and type inference.
//!
//! The `Solver` struct maintains per-expression type constraints, a mutable
//! symbol table reference, accumulated error state, the current function's
//! return type, monomorphization counters, and deduplication caches.
//!
//! # Architecture
//!
//! The solver uses a fixed-point iteration loop for each function/global:
//! repeatedly visiting all block elements until constraint accumulation
//! reaches a steady state. This ensures transitive constraint propagation,
//! nested monomorphization detection, and inference variable resolution.

use crate::bounds::extract_bounds_from_type;
use crate::constraints::TypeConstraint;
use crate::diagnosis::TypeErr;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    BlockElement, Function, FunctionId, GlobalVariable, PtrSize, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_hir_get_type::HirGetType;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::{HashMap, HashSet};
use std::hash::{Hash, Hasher};
use std::ops::Deref;

#[path = "visit.rs"]
mod solver_visit;

/// A compact cache key for monomorphization that avoids heap allocation.
///
/// Instead of `Vec<(u32, TypeId)>` which allocates and sorts every lookup,
/// we hash the (index, type_id) pairs directly into a single u64.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub(super) struct MonoCacheKey(u64);

impl MonoCacheKey {
    pub(super) fn new(original_id: usize, subst_type_args: &[(u32, TypeId)]) -> Self {
        let mut state = std::collections::hash_map::DefaultHasher::new();
        original_id.hash(&mut state);
        let mut sorted: Vec<(u32, u64)> = subst_type_args.iter().map(|(k, v)| (*k, v.as_usize() as u64)).collect();
        sorted.sort_by_key(|(k, _)| *k);
        for (k, v) in &sorted {
            k.hash(&mut state);
            v.hash(&mut state);
        }
        MonoCacheKey(state.finish())
    }
}

/// The core solver struct that manages type constraint propagation.
///
/// Contains per-expression constraints, symbol table, error accumulation,
/// function return type tracking, and monomorphization infrastructure.
pub(crate) struct Solver<'m> {
    /// Maps each ValueId to a set of type constraints it must satisfy.
    pub(super) constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    /// Mutable reference to the global symbol table.
    pub(super) m: &'m mut SymbolTab,
    /// Accumulated type errors (HashSet for deduplication).
    pub(super) errors: HashSet<TypeErr>,
    /// The return type of the function currently being solved.
    pub(super) function_return_type: Option<TypeId>,
    /// Counter for naming monomorphized functions/structs.
    pub(super) mono_counter: u32,
    /// Cache for monomorphized functions: compact hash key -> FunctionId.
    pub(super) mono_cache: HashMap<MonoCacheKey, FunctionId>,
    /// Cache for monomorphized structs: compact hash key -> StructDefId.
    pub(super) struct_mono_cache: HashMap<MonoCacheKey, crate::monomorphize::StructMonoCacheValue>,
    /// Worklist of ValueIds that need re-visiting on the next iteration.
    /// When a value is changed (e.g. monomorphized or a constraint added),
    /// it's added here so we don't re-visit all elements.
    worklist: HashSet<ValueId>,
}

type Bounds = (i128, i128);

impl<'m> Solver<'m> {
    pub(crate) fn new(m: &'m mut SymbolTab) -> Self {
        Self {
            constraints: HashMap::new(),
            m,
            errors: HashSet::new(),
            function_return_type: None,
            mono_counter: 0,
            mono_cache: HashMap::new(),
            struct_mono_cache: HashMap::new(),
            worklist: HashSet::new(),
        }
    }

    /// Report that an integer literal is outside the range of its target type.
    fn report_out_of_range(&mut self, span: ByteSpan, integer: u128, target_type: TypeId) {
        self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
            span,
            value: integer,
            target_type,
        });
    }

    /// Add a value to the worklist for re-visiting.
    pub(super) fn add_to_worklist(&mut self, id: &ValueId) {
        self.worklist.insert(id.clone());
    }

    /// Mark all block elements as needing re-visit.
    fn add_all_elements_to_worklist(&mut self, body: &[BlockElement]) {
        for element in body {
            match element {
                BlockElement::Expr(expr_id) => {
                    self.worklist.insert(expr_id.clone());
                }
                BlockElement::Local(local_var) => {
                    let lv = local_var.borrow();
                    self.worklist.insert(lv.initializer.clone());
                }
            }
        }
    }

    /// Get effective value bounds for a ValueId, accounting for constraints.
    pub(super) fn get_effective_bounds(&self, id: &ValueId) -> Option<Bounds> {
        let own_bounds = {
            let value = id.borrow();
            match &*value {
                Value::LocalVariableSymbol { id, .. } => extract_bounds_from_type(id.borrow().ty.deref()),
                Value::GlobalVariableSymbol { id, .. } => extract_bounds_from_type(id.borrow().ty.deref()),
                Value::ParameterSymbol { id, .. } => extract_bounds_from_type(id.borrow().ty.deref()),
                Value::I8 { .. } => Some((-128, 127)),
                Value::I16 { .. } => Some((-32768, 32_767)),
                Value::I32 { .. } => Some((-2_147_483_648, 2_147_483_647)),
                Value::I64 { .. } => Some((-9_223_372_036_854_775_808, 9_223_372_036_854_775_807)),
                Value::I128 { .. } => Some((i128::MIN, i128::MAX)),
                Value::U8 { .. } => Some((0, 255)),
                Value::U16 { .. } => Some((0, 65535)),
                Value::U32 { .. } => Some((0, 4_294_967_295)),
                Value::U64 { .. } => Some((0, 18_446_744_073_709_551_615)),
                Value::U128 { .. } => Some((0, i128::MAX)),
                Value::USize { .. } => Some((0, 18_446_744_073_709_551_615)),
                Value::InferredInteger { value: v, .. } => Some((**v as i128, **v as i128)),
                _ => None,
            }
        };

        if let Some(constraints) = self.constraints.get(id) {
            let mut effective_bounds = own_bounds;
            for constraint in constraints {
                let constraint_ty = constraint.type_id();
                if let Some(bounds) = extract_bounds_from_type(&constraint_ty) {
                    effective_bounds = Some(match effective_bounds {
                        Some((cur_min, cur_max)) => {
                            (std::cmp::max(cur_min, bounds.0), std::cmp::min(cur_max, bounds.1))
                        }
                        None => bounds,
                    });
                }
            }
            return effective_bounds;
        }
        own_bounds
    }

    /// Determine the concrete target type from a set of constraints.
    /// Returns the most specific (narrowest) compatible integer/float type.
    fn find_common_integer_type<'a>(
        constraints: impl Iterator<Item = &'a TypeConstraint>,
        value: u128,
    ) -> Option<(TypeId, bool)> {
        let mut best_ty: Option<TypeId> = None;
        let has_error = false;

        for constraint in constraints {
            let ty = constraint.type_id();
            let effective_ty = match &*ty {
                Type::Refine { base, .. } => *base,
                _ => ty,
            };

            if !effective_ty.is_integer_primitive() {
                // Non-integer constraint - not usable for resolution
                continue;
            }

            match &*effective_ty {
                Type::I8 { .. }
                | Type::I16 { .. }
                | Type::I32 { .. }
                | Type::I64 { .. }
                | Type::I128 { .. }
                | Type::U8 { .. }
                | Type::U16 { .. }
                | Type::U32 { .. }
                | Type::U64 { .. }
                | Type::U128 { .. }
                | Type::USize { .. } => {}
                _ => continue,
            }

            // Pick the widest type that can hold the value
            // This prefers signed over unsigned (like Rust)
            let fits = match &*effective_ty {
                Type::I8 { .. } => value <= 127,
                Type::I16 { .. } => value <= 32767,
                Type::I32 { .. } => value <= 2147483647,
                Type::I64 { .. } => value <= 9223372036854775807,
                Type::U8 { .. } => value <= 255,
                Type::U16 { .. } => value <= 65535,
                Type::U32 { .. } => value <= 4294967295,
                Type::U64 { .. } => value <= 18446744073709551615,
                Type::I128 { .. } => value <= 170141183460469231731687303715884105727,
                Type::U128 { .. } => true,
                Type::USize { .. } => true,
                _ => true,
            };

            if fits {
                // Prefer signed over unsigned when both fit
                match (best_ty, &*effective_ty) {
                    // If no best yet, take this one
                    (None, _) => best_ty = Some(effective_ty),
                    // Prefer signed over unsigned
                    _ if effective_ty.is_signed_primitive() && !best_ty.unwrap().is_signed_primitive() => {
                        best_ty = Some(effective_ty);
                    }
                    // Prefer wider over narrower
                    _ if Self::type_bit_width(&effective_ty) > Self::type_bit_width(&best_ty.unwrap()) => {
                        best_ty = Some(effective_ty);
                    }
                    _ => {}
                }
            }

            // If any constraint is a refinement, we still check it
            if let Type::Refine { .. } = &*ty {
                // Refinement bound check happens separately
            }
        }

        Some((best_ty?, has_error))
    }

    fn type_bit_width(ty: &Type) -> u32 {
        match ty {
            Type::I8 { .. } | Type::U8 { .. } => 8,
            Type::I16 { .. } | Type::U16 { .. } => 16,
            Type::I32 { .. } | Type::U32 { .. } => 32,
            Type::I64 { .. } | Type::U64 { .. } | Type::USize { .. } => 64,
            Type::I128 { .. } | Type::U128 { .. } => 128,
            _ => 0,
        }
    }

    /// Try to resolve an `InferredInteger` value based on accumulated constraints.
    /// Returns `Replace` with the concrete integer value, or `NoChange` if unresolved.
    fn solve_inferred_integer(&mut self, id: &ValueId, value: u128) -> crate::constraints::NodeAction {
        let constraints: Vec<TypeConstraint> = self
            .constraints
            .get(id)
            .cloned()
            .unwrap_or_default()
            .into_iter()
            .collect();

        if constraints.is_empty() {
            return crate::constraints::NodeAction::NoChange;
        }

        let span = id.borrow().span();
        let mut check_errors: Vec<(TypeId, i128)> = Vec::new();
        let mut has_non_integer = false;
        let mut unsatisfiable_ty: Option<TypeId> = None;

        // Phase 1: Validate constraints - check refinement bounds and non-integer errors
        for constraint in &constraints {
            let ty = constraint.type_id();
            if let Type::Refine { .. } = &*ty {
                let bounds_check = crate::bounds::check_literal_against_refinement(value as i128, &ty);
                if !bounds_check {
                    check_errors.push((ty, value as i128));
                }
            }
            let effective_ty = match &*ty {
                Type::Refine { base, .. } => *base,
                _ => ty,
            };
            if !effective_ty.is_integer_primitive() {
                has_non_integer = true;
                unsatisfiable_ty = Some(ty);
            }
        }

        // Report errors
        if has_non_integer {
            if let Some(unsat_ty) = unsatisfiable_ty {
                self.errors.insert(TypeErr::IntegerLiteralUnsatisfiable {
                    span,
                    value,
                    unsatisfiable_type: unsat_ty,
                });
            }
            return crate::constraints::NodeAction::NoChange;
        }

        for (refinement_ty, val) in &check_errors {
            self.errors.insert(TypeErr::IntegerLiteralOutOfRefinementBounds {
                span,
                value: *val as u128,
                refinement_type: *refinement_ty,
            });
        }

        // Phase 2: Find the best (widest fitting) common type across all constraints
        let (best_ty, _has_error) = Self::find_common_integer_type(constraints.iter(), value)
            .unwrap_or((TypeId::from(Type::I32 { span }), false));

        // Phase 3: Build the concrete value
        match &*best_ty {
            Type::I8 { .. } => match i8::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::I8 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::I16 { .. } => match i16::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::I16 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::I32 { .. } => match i32::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::I32 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::I64 { .. } => match i64::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::I64 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::I128 { .. } => match i128::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::I128 {
                    span,
                    value: Box::new(v),
                }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::U8 { .. } => match u8::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::U8 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::U16 { .. } => match u16::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::U16 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::U32 { .. } => match u32::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::U32 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::U64 { .. } => match u64::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::U64 { span, value: v }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::U128 { .. } => match u128::try_from(value) {
                Ok(v) => crate::constraints::NodeAction::Replace(Value::U128 {
                    span,
                    value: Box::new(v),
                }),
                Err(_) => {
                    self.report_out_of_range(span, value, best_ty);
                    crate::constraints::NodeAction::NoChange
                }
            },
            Type::USize { .. } => match self.m.arch_ptr_size() {
                PtrSize::U32 => match u32::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::USize {
                        span,
                        bits: 32,
                        value: u64::from(v),
                    }),
                    Err(_) => {
                        self.report_out_of_range(span, value, best_ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                PtrSize::U64 => match u64::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::USize {
                        span,
                        bits: 64,
                        value: v,
                    }),
                    Err(_) => {
                        self.report_out_of_range(span, value, best_ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
            },
            _ => crate::constraints::NodeAction::NoChange,
        }
    }

    /// Try to resolve an `InferredFloat` value based on accumulated constraints.
    fn solve_inferred_float(&mut self, id: &ValueId, value: OrderedFloat<f64>) -> crate::constraints::NodeAction {
        let span = id.borrow().span();
        if let Some(constraints) = self.constraints.get(id) {
            let mut best_ty: Option<TypeId> = None;
            let mut has_non_float = false;
            let mut unsatisfiable_ty: Option<TypeId> = None;

            for constraint in constraints {
                let ty = constraint.type_id();
                if !ty.is_float_primitive() {
                    has_non_float = true;
                    unsatisfiable_ty = Some(ty);
                    continue;
                }
                // Pick the widest float type (F64 > F32)
                if let Some(current_best) = best_ty {
                    let current_is_f64 = matches!(&*current_best, Type::F64 { .. });
                    let this_is_f64 = matches!(&*ty, Type::F64 { .. });
                    if this_is_f64 && !current_is_f64 {
                        best_ty = Some(ty);
                    }
                } else {
                    best_ty = Some(ty);
                }
            }

            if has_non_float {
                if let Some(unsat_ty) = unsatisfiable_ty {
                    self.errors.insert(TypeErr::FloatLiteralUnsatisfiable {
                        span,
                        value,
                        unsatisfiable_type: unsat_ty,
                    });
                }
                return crate::constraints::NodeAction::NoChange;
            }

            if let Some(best) = best_ty {
                return match &*best {
                    Type::F32 { .. } => crate::constraints::NodeAction::Replace(Value::F32 {
                        span,
                        value: (*value as f32).into(),
                    }),
                    Type::F64 { .. } => crate::constraints::NodeAction::Replace(Value::F64 { span, value }),
                    _ => unreachable!(),
                };
            }
        }
        crate::constraints::NodeAction::NoChange
    }

    /// Solve all constraints for a function.
    /// Uses worklist-based fixed-point iteration: only re-visits changed values.
    fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        if let Some(body) = &mut function.body {
            self.function_return_type = Some(function.return_type);

            // Initially visit all elements
            self.add_all_elements_to_worklist(body);

            loop {
                // Drain the worklist: visit only elements that were queued
                let pending: Vec<ValueId> = self.worklist.drain().collect();
                if pending.is_empty() {
                    break;
                }

                let prev_len = self.constraints.len();
                let prev_mono_count = self.mono_counter;

                for value_id in &pending {
                    // Find which block element this value belongs to and visit it
                    self.visit(value_id);
                }

                if self.constraints.len() == prev_len && self.mono_counter == prev_mono_count {
                    break;
                }
            }

            // Final pass: resolve any remaining InferredInteger/InferredFloat values
            // by defaulting to i32/f64 (like Rust's default literal types)
            self.finalize_inferred_literals(body);
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }

    /// Recursively default remaining InferredInteger → i32 and InferredFloat → f64
    /// This walks through the entire value tree to catch any nested inferred values.
    fn finalize_inferred_literals(&mut self, body: &mut [BlockElement]) {
        for element in body.iter_mut() {
            match element {
                BlockElement::Expr(expr_id) => {
                    self.finalize_value_recursive(expr_id);
                }
                BlockElement::Local(local_var) => {
                    let lv = local_var.borrow();
                    self.finalize_value_recursive(&lv.initializer);
                }
            }
        }
    }

    /// Recursively finalize a value and all its children.
    fn finalize_value_recursive(&mut self, value_id: &ValueId) {
        // First, try to resolve this value itself
        let span = value_id.borrow().span();
        if matches!(
            &*value_id.borrow(),
            Value::InferredInteger { .. } | Value::InferredFloat { .. }
        ) {
            // Check if there are still unsolved constraints
            if let Some(_constraints) = self.constraints.get(value_id) {
                // If there are constraints, re-try solving
                self.visit(value_id);
            }
        }

        let was_resolved = !matches!(
            &*value_id.borrow(),
            Value::InferredInteger { .. } | Value::InferredFloat { .. }
        );

        // If it's still inferred and has no constraints, emit a diagnostic
        if !was_resolved {
            let current_value = value_id.borrow();
            match &*current_value {
                Value::InferredInteger { value: v, .. } => {
                    // Default to i32
                    let action = match i32::try_from(**v) {
                        Ok(val) => crate::constraints::NodeAction::Replace(Value::I32 { span, value: val }),
                        Err(_) => match i64::try_from(**v) {
                            Ok(val) => crate::constraints::NodeAction::Replace(Value::I64 { span, value: val }),
                            Err(_) => match u64::try_from(**v) {
                                Ok(val) => crate::constraints::NodeAction::Replace(Value::U64 { span, value: val }),
                                Err(_) => crate::constraints::NodeAction::Replace(Value::U128 {
                                    span,
                                    value: Box::new(**v),
                                }),
                            },
                        },
                    };
                    if let crate::constraints::NodeAction::Replace(new_value) = action {
                        value_id.replace(new_value);
                    }
                }
                Value::InferredFloat { value: v, .. } => {
                    value_id.replace(Value::F64 { span, value: *v });
                }
                _ => {}
            }
            return;
        }

        // Then recurse into children
        let value = value_id.borrow().clone();
        match &value {
            Value::Block { block, .. } => {
                self.finalize_inferred_literals(&mut block.borrow_mut().elements);
            }
            Value::StructObject { fields, .. } => {
                for (_, field_value) in fields {
                    self.finalize_value_recursive(field_value);
                }
            }
            Value::EnumVariant { value: v, .. } => {
                self.finalize_value_recursive(v);
            }
            Value::Binary { left, right, .. } => {
                self.finalize_value_recursive(left);
                self.finalize_value_recursive(right);
            }
            Value::Unary { operand, .. } => {
                self.finalize_value_recursive(operand);
            }
            Value::IndexAccess { collection, index, .. } => {
                self.finalize_value_recursive(collection);
                self.finalize_value_recursive(index);
            }
            Value::FieldAccess { expr, .. } => {
                self.finalize_value_recursive(expr);
            }
            Value::Assign { place, value: v, .. } => {
                self.finalize_value_recursive(place);
                self.finalize_value_recursive(v);
            }
            Value::Deref { place, .. } => {
                self.finalize_value_recursive(place);
            }
            Value::Cast { value: v, .. } => {
                self.finalize_value_recursive(v);
            }
            Value::Borrow { place, .. } => {
                self.finalize_value_recursive(place);
            }
            Value::List { elements, .. } => {
                for element in elements {
                    self.finalize_value_recursive(element);
                }
            }
            Value::Tuple { elements, .. } => {
                for element in elements {
                    self.finalize_value_recursive(element);
                }
            }
            Value::If {
                condition,
                true_branch,
                false_branch,
                ..
            } => {
                self.finalize_value_recursive(condition);
                self.finalize_inferred_literals(&mut true_branch.borrow_mut().elements);
                if let Some(false_branch) = false_branch {
                    self.finalize_inferred_literals(&mut false_branch.borrow_mut().elements);
                }
            }
            Value::While { condition, body, .. } => {
                self.finalize_value_recursive(condition);
                self.finalize_inferred_literals(&mut body.borrow_mut().elements);
            }
            Value::Loop { body, .. } => {
                self.finalize_inferred_literals(&mut body.borrow_mut().elements);
            }
            Value::Return { value: v, .. } => {
                self.finalize_value_recursive(v);
            }
            Value::Call { callee, args, .. } => {
                self.finalize_value_recursive(callee);
                for arg in &args.positional {
                    self.finalize_value_recursive(arg);
                }
                for (_, arg) in &args.named {
                    self.finalize_value_recursive(arg);
                }
            }
            Value::MethodCall { object, args, .. } => {
                self.finalize_value_recursive(object);
                for arg in &args.positional {
                    self.finalize_value_recursive(arg);
                }
                for (_, arg) in &args.named {
                    self.finalize_value_recursive(arg);
                }
            }
            _ => {}
        }
    }

    /// Solve all constraints for a global variable.
    fn solve_global_variable(&mut self, g: &mut GlobalVariable, log: &CompilerLog) -> Result<(), ()> {
        loop {
            let prev_len = self.constraints.len();
            if g.ty.is_inferred() {
                if let Ok(ty) = g.initializer.borrow().determine_type(self.m) {
                    g.ty = ty.into();
                }
            } else {
                let value = g.initializer.clone();
                let ty = g.ty;
                self.add_constraint(&value, TypeConstraint::Equal(ty));
            }
            self.visit(&g.initializer);
            if self.constraints.len() == prev_len {
                break;
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }
}

/// Public entry point: resolve all type constraints for a function.
pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut solver = Solver::new(m);
    solver.solve_function(function, log)
}

/// Public entry point: resolve all type constraints for a global variable.
pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut solver = Solver::new(m);
    solver.solve_global_variable(global, log)
}
