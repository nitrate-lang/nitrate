use crate::bounds::{Bounds, extract_bounds_from_type};
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
use std::ops::Deref;

pub(crate) const MAX_MONO_DEPTH: u32 = 64;

#[path = "visit.rs"]
mod solver_visit;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub(super) struct MonoCacheKey(u64);

impl MonoCacheKey {
    pub(super) fn new(original_id: usize, subst_type_args: &[(u32, TypeId)]) -> Self {
        let mut hash: u64 = 0xcbf29ce484222325;
        hash ^= original_id as u64;
        hash = hash.wrapping_mul(0x100000001b3);
        for (k, v) in subst_type_args {
            hash ^= *k as u64;
            hash = hash.wrapping_mul(0x100000001b3);
            hash ^= v.as_usize() as u64;
            hash = hash.wrapping_mul(0x100000001b3);
        }
        MonoCacheKey(hash)
    }
}

pub(crate) struct Solver<'m> {
    pub(super) constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    pub(super) m: &'m mut SymbolTab,
    pub(super) errors: HashSet<TypeErr>,
    pub(super) function_return_type: Option<TypeId>,
    pub(super) mono_counter: u32,
    pub(super) mono_cache: HashMap<MonoCacheKey, FunctionId>,
    pub(super) struct_mono_cache: HashMap<MonoCacheKey, crate::monomorphize::StructMonoCacheValue>,
    worklist: HashSet<ValueId>,
    constraint_version: u64,
    pub(super) mono_depth: u32,
    pub(super) mono_in_progress: HashSet<MonoCacheKey>,
}

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
            constraint_version: 0,
            mono_depth: 0,
            mono_in_progress: HashSet::new(),
        }
    }

    pub(crate) fn report_out_of_range(&mut self, span: ByteSpan, integer: u128, target_type: TypeId) {
        self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
            span,
            value: integer,
            target_type,
        });
    }

    pub(crate) fn add_to_worklist(&mut self, id: &ValueId) {
        self.worklist.insert(id.clone());
    }

    pub(crate) fn add_all_elements_to_worklist(&mut self, body: &[BlockElement]) {
        for element in body {
            match element {
                BlockElement::Expr(expr_id) => {
                    self.worklist.insert(expr_id.clone());
                }
                BlockElement::Local(local_var) => {
                    let lv = local_var.borrow();
                    if let Some(init) = &lv.initializer {
                        self.worklist.insert(init.clone());
                    }
                }
            }
        }
    }

    pub(crate) fn get_effective_bounds(&self, id: &ValueId) -> Option<Bounds> {
        let own_bounds = {
            let value = id.borrow();
            match &*value {
                Value::LocalVariableSymbol { id, .. } => extract_bounds_from_type(id.borrow().ty.deref()),
                Value::GlobalVariableSymbol { id, .. } => extract_bounds_from_type(id.borrow().ty.deref()),
                Value::ParameterSymbol { id, .. } => extract_bounds_from_type(id.borrow().ty.deref()),
                Value::I8 { .. } => Some(Bounds::signed(-128, 127)),
                Value::I16 { .. } => Some(Bounds::signed(-32768, 32_767)),
                Value::I32 { .. } => Some(Bounds::signed(-2_147_483_648, 2_147_483_647)),
                Value::I64 { .. } => Some(Bounds::signed(-9_223_372_036_854_775_808, 9_223_372_036_854_775_807)),
                Value::I128 { .. } => Some(Bounds::signed(i128::MIN, i128::MAX)),
                Value::U8 { .. } => Some(Bounds::unsigned(0, 255)),
                Value::U16 { .. } => Some(Bounds::unsigned(0, 65535)),
                Value::U32 { .. } => Some(Bounds::unsigned(0, 4_294_967_295)),
                Value::U64 { .. } => Some(Bounds::unsigned(0, 18_446_744_073_709_551_615)),
                Value::U128 { .. } => Some(Bounds::unsigned(0, u128::MAX)),
                Value::USize { .. } => Some(Bounds::unsigned(0, 18_446_744_073_709_551_615)),
                Value::InferredInteger { value: v, .. } => Some(Bounds::new(**v as i128, **v as u128)),
                _ => None,
            }
        };

        if let Some(constraints) = self.constraints.get(id) {
            let mut effective_bounds = own_bounds;
            for constraint in constraints {
                let constraint_ty = constraint.type_id();
                if let Some(bounds) = extract_bounds_from_type(&constraint_ty) {
                    effective_bounds = Some(match effective_bounds {
                        Some(cur) => Bounds::new(std::cmp::max(cur.lo, bounds.lo), std::cmp::min(cur.hi, bounds.hi)),
                        None => bounds,
                    });
                }
            }
            return effective_bounds;
        }
        own_bounds
    }

    pub(crate) fn find_common_integer_type<'a>(
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
                match (best_ty, &*effective_ty) {
                    (None, _) => best_ty = Some(effective_ty),
                    _ if effective_ty.is_signed_primitive() && !best_ty.unwrap().is_signed_primitive() => {
                        best_ty = Some(effective_ty);
                    }
                    _ if Self::type_bit_width(&effective_ty) > Self::type_bit_width(&best_ty.unwrap()) => {
                        best_ty = Some(effective_ty);
                    }
                    _ => {}
                }
            }

            if let Type::Refine { .. } = &*ty {}
        }

        Some((best_ty?, has_error))
    }

    pub(crate) fn type_bit_width(ty: &Type) -> u32 {
        match ty {
            Type::I8 { .. } | Type::U8 { .. } => 8,
            Type::I16 { .. } | Type::U16 { .. } => 16,
            Type::I32 { .. } | Type::U32 { .. } => 32,
            Type::I64 { .. } | Type::U64 { .. } | Type::USize { .. } => 64,
            Type::I128 { .. } | Type::U128 { .. } => 128,
            _ => 0,
        }
    }

    pub(crate) fn solve_inferred_integer(&mut self, id: &ValueId, value: u128) -> crate::constraints::NodeAction {
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

        for constraint in &constraints {
            let ty = constraint.type_id();
            if let Type::Refine { .. } = &*ty {
                let bounds_check = crate::bounds::check_literal_against_refinement(value, &ty);
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

        let (best_ty, _has_error) = Self::find_common_integer_type(constraints.iter(), value)
            .unwrap_or((TypeId::from(Type::I32 { span }), false));

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

    pub(crate) fn solve_inferred_float(
        &mut self,
        id: &ValueId,
        value: OrderedFloat<f64>,
    ) -> crate::constraints::NodeAction {
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

    pub(crate) fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        if let Some(body) = &mut function.body {
            self.function_return_type = Some(function.return_type);

            self.add_all_elements_to_worklist(body);

            loop {
                let pending: Vec<ValueId> = self.worklist.drain().collect();
                if pending.is_empty() {
                    break;
                }

                let prev_version = self.constraint_version;
                let prev_mono_count = self.mono_counter;

                for value_id in &pending {
                    self.visit(value_id);
                }

                if self.constraint_version == prev_version && self.mono_counter == prev_mono_count {
                    break;
                }
            }

            self.finalize_inferred_literals(body);
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }

    pub(crate) fn finalize_inferred_literals(&mut self, body: &mut [BlockElement]) {
        for element in body.iter_mut() {
            match element {
                BlockElement::Expr(expr_id) => {
                    self.finalize_value_recursive(expr_id);
                }
                BlockElement::Local(local_var) => {
                    let lv = local_var.borrow();
                    if let Some(init) = &lv.initializer {
                        self.finalize_value_recursive(init);
                    }
                }
            }
        }
    }

    pub(crate) fn finalize_value_recursive(&mut self, value_id: &ValueId) {
        let span = value_id.borrow().span();
        let is_inferred = matches!(
            &*value_id.borrow(),
            Value::InferredInteger { .. } | Value::InferredFloat { .. }
        );

        if is_inferred {
            if self.constraints.contains_key(value_id) {
                self.visit(value_id);
            }
        }

        let inferred_info = {
            let v = value_id.borrow();
            if matches!(&*v, Value::InferredInteger { .. } | Value::InferredFloat { .. }) {
                match &*v {
                    Value::InferredInteger { value, .. } => Some(InferredInfo::Int(**value)),
                    Value::InferredFloat { value, .. } => Some(InferredInfo::Float(value.0)),
                    _ => None,
                }
            } else {
                None
            }
        };

        if let Some(info) = inferred_info {
            let has_reported_error = self.constraints.get(value_id).map_or(false, |constraints| {
                constraints.iter().any(|c| {
                    let ty = c.type_id();
                    let effective_ty = match &*ty {
                        Type::Refine { base, .. } => *base,
                        _ => ty,
                    };
                    match &info {
                        InferredInfo::Int(_) => !effective_ty.is_integer_primitive(),
                        InferredInfo::Float(_) => !effective_ty.is_float_primitive(),
                    }
                })
            });

            if !has_reported_error {
                match info {
                    InferredInfo::Int(val) => {
                        let new_value = match i32::try_from(val) {
                            Ok(v) => Value::I32 { span, value: v },
                            Err(_) => match i64::try_from(val) {
                                Ok(v) => Value::I64 { span, value: v },
                                Err(_) => match u64::try_from(val) {
                                    Ok(v) => Value::U64 { span, value: v },
                                    Err(_) => Value::U128 {
                                        span,
                                        value: Box::new(val),
                                    },
                                },
                            },
                        };
                        value_id.replace(new_value);
                    }
                    InferredInfo::Float(val) => {
                        value_id.replace(Value::F64 {
                            span,
                            value: ordered_float::OrderedFloat(val),
                        });
                    }
                }
            }
            return;
        }

        let children: Option<Vec<ValueId>> = {
            let v = value_id.borrow();
            match &*v {
                Value::Block { block, .. } => {
                    self.finalize_inferred_literals(&mut block.borrow_mut().elements);
                    None
                }
                Value::StructObject { fields, .. } => Some(fields.iter().map(|(_, v)| v.clone()).collect()),
                Value::EnumVariant { value: v, .. } => Some(vec![v.clone()]),
                Value::Binary { left, right, .. } => Some(vec![left.clone(), right.clone()]),
                Value::Unary { operand, .. } => Some(vec![operand.clone()]),
                Value::IndexAccess { collection, index, .. } => Some(vec![collection.clone(), index.clone()]),
                Value::FieldAccess { expr, .. } => Some(vec![expr.clone()]),
                Value::Assign { place, value: val, .. } => Some(vec![place.clone(), val.clone()]),
                Value::Deref { place, .. } => Some(vec![place.clone()]),
                Value::Cast { value: val, .. } => Some(vec![val.clone()]),
                Value::Borrow { place, .. } => Some(vec![place.clone()]),
                Value::List { elements, .. } => Some(elements.iter().cloned().collect()),
                Value::Tuple { elements, .. } => Some(elements.iter().cloned().collect()),
                Value::If {
                    condition,
                    true_branch,
                    false_branch,
                    ..
                } => {
                    self.finalize_inferred_literals(&mut true_branch.borrow_mut().elements);
                    if let Some(fb) = false_branch {
                        self.finalize_inferred_literals(&mut fb.borrow_mut().elements);
                    }
                    Some(vec![condition.clone()])
                }
                Value::While { condition, body, .. } => {
                    self.finalize_inferred_literals(&mut body.borrow_mut().elements);
                    Some(vec![condition.clone()])
                }
                Value::Loop { body, .. } => {
                    self.finalize_inferred_literals(&mut body.borrow_mut().elements);
                    None
                }
                Value::Return { value: val, .. } => Some(vec![val.clone()]),
                Value::Call { callee, args, .. } => {
                    let mut ids = vec![callee.clone()];
                    ids.extend(args.positional.iter().cloned());
                    ids.extend(args.named.iter().map(|(_, v)| v.clone()));
                    Some(ids)
                }
                Value::MethodCall { object, args, .. } => {
                    let mut ids = vec![object.clone()];
                    ids.extend(args.positional.iter().cloned());
                    ids.extend(args.named.iter().map(|(_, v)| v.clone()));
                    Some(ids)
                }
                _ => None,
            }
        };

        if let Some(children) = children {
            for child in &children {
                self.finalize_value_recursive(child);
            }
        }
    }

    pub(crate) fn solve_global_variable(&mut self, g: &mut GlobalVariable, log: &CompilerLog) -> Result<(), ()> {
        loop {
            let prev_version = self.constraint_version;
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
            if self.constraint_version == prev_version {
                break;
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }
}

enum InferredInfo {
    Int(u128),
    Float(f64),
}

pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    crate::range::ensure_range_structs(m);
    let mut solver = Solver::new(m);
    solver.solve_function(function, log)
}

pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    crate::range::ensure_range_structs(m);
    let mut solver = Solver::new(m);
    solver.solve_global_variable(global, log)
}
