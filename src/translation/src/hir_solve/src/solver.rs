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
    Function, FunctionId, GlobalVariable, PtrSize, StructDefId, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_hir_get_type::HirGetType;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::{HashMap, HashSet};
use std::ops::Deref;

#[path = "visit.rs"]
mod solver_visit;

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
    /// Cache for monomorphized functions: (func_usize, sorted_type_args) -> FunctionId.
    pub(super) mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>,
    /// Cache for monomorphized structs: (struct_usize, sorted_type_args) -> StructDefId.
    pub(super) struct_mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), StructDefId>,
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

        let span = id.borrow().span();

        for constraint in &constraints {
            let ty = constraint.type_id();
            if let Type::Refine { .. } = &*ty {
                let bounds_check = crate::bounds::check_literal_against_refinement(value as i128, &ty);
                if !bounds_check {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRefinementBounds {
                        span,
                        value,
                        refinement_type: ty,
                    });
                }
            }
            let effective_ty = match &*ty {
                Type::Refine { base, .. } => *base,
                _ => ty,
            };
            if !effective_ty.is_integer_primitive() {
                self.errors.insert(TypeErr::IntegerLiteralUnsatisfiable {
                    span,
                    value,
                    unsatisfiable_type: ty,
                });
                break;
            }
            return match &*effective_ty {
                Type::I8 { .. } => match i8::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::I8 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::I16 { .. } => match i16::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::I16 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::I32 { .. } => match i32::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::I32 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::I64 { .. } => match i64::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::I64 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::I128 { .. } => match i128::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::I128 {
                        span,
                        value: Box::new(v),
                    }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::U8 { .. } => match u8::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::U8 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::U16 { .. } => match u16::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::U16 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::U32 { .. } => match u32::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::U32 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::U64 { .. } => match u64::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::U64 { span, value: v }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
                        crate::constraints::NodeAction::NoChange
                    }
                },
                Type::U128 { .. } => match u128::try_from(value) {
                    Ok(v) => crate::constraints::NodeAction::Replace(Value::U128 {
                        span,
                        value: Box::new(v),
                    }),
                    Err(_) => {
                        self.report_out_of_range(span, value, ty);
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
                            self.report_out_of_range(span, value, ty);
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
                            self.report_out_of_range(span, value, ty);
                            crate::constraints::NodeAction::NoChange
                        }
                    },
                },
                _ => return crate::constraints::NodeAction::NoChange,
            };
        }
        crate::constraints::NodeAction::NoChange
    }

    /// Try to resolve an `InferredFloat` value based on accumulated constraints.
    fn solve_inferred_float(&mut self, id: &ValueId, value: OrderedFloat<f64>) -> crate::constraints::NodeAction {
        let span = id.borrow().span();
        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                let ty = constraint.type_id();
                if !ty.is_float_primitive() {
                    self.errors.insert(TypeErr::FloatLiteralUnsatisfiable {
                        span,
                        value,
                        unsatisfiable_type: ty,
                    });
                    break;
                }
                return match &*ty {
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
    /// Uses fixed-point iteration: repeats until no new constraints are generated.
    fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        if let Some(body) = &mut function.body {
            self.function_return_type = Some(function.return_type);
            loop {
                let prev_len = self.constraints.len();
                for element in body.iter_mut() {
                    self.visit_block_element(element);
                }
                // Also check for monomorphization progress
                let prev_mono_count = self.mono_counter;
                if self.constraints.len() == prev_len && self.mono_counter == prev_mono_count {
                    break;
                }
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
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
