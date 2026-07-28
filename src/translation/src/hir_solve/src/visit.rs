//! Per-variant visitor handlers for the HIR expression graph.
//!
//! Each Value variant gets its own `visit_*` handler method, factored out
//! from the monolithic `visit_children` in the original code. This module
//! implements constraint propagation, bounds checking, and generic inference
//! for every expression kind.

use crate::constraints::{TypeConstraint, is_arithmetic_op, is_comparison_or_logical_op, propagate_to_children};
use crate::diagnosis::TypeErr;
use crate::solver::Solver;
use nitrate_hir::{BlockElement, BlockId, FunctionId, Type, TypeId, Value, ValueId};
use nitrate_hir_get_type::HirGetType;
use nitrate_tree::ByteSpan;
use std::collections::HashSet;
use std::unreachable;

impl<'m> Solver<'m> {
    /// Main entry: visit a value node, determine action, then recurse.
    pub(super) fn visit(&mut self, e: &ValueId) {
        let action = {
            let current_value = e.borrow();
            self.determine_action(&current_value, e)
        };
        match action {
            crate::constraints::NodeAction::Replace(new_value) => {
                e.replace(new_value);
            }
            crate::constraints::NodeAction::NoChange => self.visit_children(e),
        }
    }

    pub(super) fn visit_block(&mut self, block: &BlockId) {
        for element in &mut block.borrow_mut().elements {
            self.visit_block_element(element);
        }
    }

    pub(super) fn visit_block_element(&mut self, element: &mut BlockElement) {
        match element {
            BlockElement::Expr(e) => self.visit(e),
            BlockElement::Local(local_var) => {
                let (is_inferred, init_id) = {
                    let lv = local_var.borrow();
                    (lv.ty.is_inferred(), lv.initializer.clone())
                };
                if is_inferred {
                    if let Ok(ty) = init_id.borrow().determine_type(self.m) {
                        local_var.borrow_mut().ty = ty.into();
                    }
                } else {
                    let ty = local_var.borrow().ty;
                    self.add_constraint(&init_id, TypeConstraint::Equal(ty));
                }
                self.visit(&local_var.borrow().initializer);
            }
        }
    }

    // ── Determine Action ──────────────────────────────────────────────────

    fn determine_action(&mut self, value: &Value, id: &ValueId) -> crate::constraints::NodeAction {
        match value {
            Value::InferredInteger { value, .. } => self.solve_inferred_integer(id, **value),
            Value::InferredFloat { value, .. } => self.solve_inferred_float(id, *value),
            _ => crate::constraints::NodeAction::NoChange,
        }
    }

    // ── Constraint helpers (used by both visit.rs and solver.rs) ──────────

    pub(super) fn add_constraint(&mut self, id: &ValueId, constraint: TypeConstraint) {
        self.constraints.entry(id.clone()).or_default().insert(constraint);
    }

    pub(super) fn add_constraints(&mut self, id: &ValueId, constraints: impl IntoIterator<Item = TypeConstraint>) {
        self.constraints.entry(id.clone()).or_default().extend(constraints);
    }

    // ── Visit Children (dispatched by variant) ────────────────────────────

    fn visit_children(&mut self, e: &ValueId) {
        let value = e.borrow().clone();
        match &value {
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
            | Value::BStringLit { .. }
            | Value::InferredInteger { .. }
            | Value::InferredFloat { .. } => {}

            Value::StructObject { .. } => self.visit_struct_object(e, &value),
            Value::EnumVariant { .. } => self.visit_enum_variant(e, &value),
            Value::Binary { .. } => self.visit_binary(e, &value),
            Value::Unary { .. } => self.visit_unary(e, &value),
            Value::IndexAccess { .. } => self.visit_index_access(e, &value),
            Value::FieldAccess { .. } => self.visit_field_access(e, &value),
            Value::Assign { .. } => self.visit_assign(e, &value),
            Value::Deref { .. } => self.visit_deref(e, &value),
            Value::Cast { .. } => self.visit_cast(e, &value),
            Value::Borrow { .. } => self.visit_borrow(e, &value),
            Value::List { .. } => self.visit_list(e, &value),
            Value::Tuple { .. } => self.visit_tuple(e, &value),
            Value::If { .. } => self.visit_if(e, &value),
            Value::While { .. } => self.visit_while(e, &value),
            Value::Loop { .. } => self.visit_loop(e, &value),
            Value::Break { .. } | Value::Continue { .. } => {}
            Value::Return { .. } => self.visit_return(e, &value),
            Value::Block { .. } => self.visit_block_value(e, &value),
            Value::Call { .. } => self.visit_call(e, &value),
            Value::MethodCall { .. } => self.visit_method_call(e, &value),
            Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => {}
        }
    }

    // ── Per-variant handlers ──────────────────────────────────────────────

    fn visit_struct_object(&mut self, e: &ValueId, value: &Value) {
        let Value::StructObject { struct_def, fields, .. } = value else {
            unreachable!()
        };
        let has_generics = struct_def.borrow().generics.is_some();

        if has_generics {
            if let Some(subst) = self.infer_generic_args_from_struct_fields(struct_def, fields) {
                let mono_struct_id = self.monomorphize_struct(struct_def, &subst);
                {
                    let mut original = e.borrow_mut();
                    if let Value::StructObject { struct_def: sd, .. } = &mut *original {
                        *sd = mono_struct_id;
                    }
                }
                let _ = value;
                let updated = e.borrow();
                if let Value::StructObject {
                    struct_def: sd,
                    fields: flds,
                    ..
                } = &*updated
                {
                    let struct_def_b = sd.borrow();
                    for (field_name, field_value) in flds {
                        if let Some(field) = struct_def_b.fields.get(field_name) {
                            self.add_constraint(field_value, TypeConstraint::Equal(field.ty));
                            self.visit(field_value);
                        }
                    }
                }
            } else {
                for (_, field_value) in fields {
                    self.visit(field_value);
                }
            }
        } else {
            let struct_def_b = struct_def.borrow();
            for (field_name, field_value) in fields {
                if let Some(field) = struct_def_b.fields.get(field_name) {
                    self.add_constraint(field_value, TypeConstraint::Equal(field.ty));
                    self.visit(field_value);
                }
            }
        }
    }

    fn visit_enum_variant(&mut self, _e: &ValueId, value: &Value) {
        let Value::EnumVariant {
            enum_def,
            variant,
            value: inner_value,
            ..
        } = value
        else {
            unreachable!()
        };
        let variant_type = enum_def
            .borrow()
            .variants
            .iter()
            .find(|item| item.name == *variant)
            .expect("variant not present")
            .ty;
        self.add_constraint(inner_value, TypeConstraint::Equal(variant_type));
        self.visit(inner_value);
    }

    fn visit_binary(&mut self, e: &ValueId, value: &Value) {
        let Value::Binary { left, op, right, .. } = value else {
            unreachable!()
        };
        let span = value.span();

        let parent_constraints: Vec<TypeConstraint> = self
            .constraints
            .get(e)
            .cloned()
            .unwrap_or_default()
            .into_iter()
            .collect();
        for c in &parent_constraints {
            let result_ty = c.type_id();
            if is_arithmetic_op(op) {
                if let (Some(lb), Some(rb)) = (self.get_effective_bounds(left), self.get_effective_bounds(right)) {
                    if let Some(res) = crate::bounds::compute_binary_bounds(op, lb, rb) {
                        if !crate::bounds::check_bounds_against_constraint(res, &result_ty) {
                            let bounds = crate::bounds::extract_bounds_from_type(&result_ty);
                            if let Some((comp_min, comp_max)) = bounds {
                                self.errors.insert(TypeErr::OperationResultOutOfRefinementBounds {
                                    span,
                                    refinement_type: result_ty,
                                    computed_min: comp_min.max(0) as u128,
                                    computed_max: comp_max.max(0) as u128,
                                });
                            }
                        }
                    }
                }
            }
        }

        if let Some(parent_constraints) = self.constraints.get(e).cloned() {
            let child_constraints = propagate_to_children(&parent_constraints);
            self.add_constraints(left, child_constraints.clone());
            self.add_constraints(right, child_constraints);
        }

        if let Ok(left_type) = left.borrow().determine_type(self.m) {
            if let Ok(right_type) = right.borrow().determine_type(self.m) {
                let left_ty: TypeId = left_type.clone().into();
                let right_ty: TypeId = right_type.clone().into();
                let left_inferred = left_type.is_inferred();
                let right_inferred = right_type.is_inferred();
                if left_ty == right_ty && !left_inferred && is_arithmetic_op(op) {
                    self.add_constraint(e, TypeConstraint::Equal(left_ty));
                }
                if is_comparison_or_logical_op(op) {
                    self.add_constraint(e, TypeConstraint::eq_type(Type::Bool { span }));
                    if !left_inferred {
                        self.add_constraint(right, TypeConstraint::Equal(left_ty));
                    }
                    if !right_inferred {
                        self.add_constraint(left, TypeConstraint::Equal(right_ty));
                    }
                }
            }
        }

        self.visit(left);
        self.visit(right);
    }

    fn visit_unary(&mut self, e: &ValueId, value: &Value) {
        let Value::Unary { op, operand, .. } = value else {
            unreachable!()
        };
        let span = value.span();

        if let Some(constraints) = self.constraints.get(e).cloned() {
            self.add_constraints(operand, constraints.clone());
        }
        self.visit(operand);

        let parent_constraints: Vec<TypeConstraint> = self
            .constraints
            .get(e)
            .cloned()
            .unwrap_or_default()
            .into_iter()
            .collect();
        for c in &parent_constraints {
            let result_ty = c.type_id();
            if let Some(ob) = self.get_effective_bounds(operand) {
                let res = crate::bounds::compute_unary_bounds(op, ob);
                if !crate::bounds::check_bounds_against_constraint(res, &result_ty) {
                    let bounds = crate::bounds::extract_bounds_from_type(&result_ty);
                    if let Some((comp_min, comp_max)) = bounds {
                        self.errors.insert(TypeErr::OperationResultOutOfRefinementBounds {
                            span,
                            refinement_type: result_ty,
                            computed_min: comp_min.max(0) as u128,
                            computed_max: comp_max.max(0) as u128,
                        });
                    }
                }
            }
        }
    }

    fn visit_index_access(&mut self, e: &ValueId, value: &Value) {
        let Value::IndexAccess { collection, index, .. } = value else {
            unreachable!()
        };
        self.add_constraint(
            index,
            TypeConstraint::eq_type(Type::USize {
                span: ByteSpan::default(),
            }),
        );

        if let Some(parent_constraints) = self.constraints.get(e).cloned()
            && let Ok(collection_type) = collection.borrow().determine_type(self.m)
        {
            let element_type_id = match &collection_type {
                Type::Array { element_type, .. }
                | Type::SliceRef { element_type, .. }
                | Type::SlicePtr { element_type, .. } => Some(*element_type),
                _ => None,
            };
            if let Some(element_type_id) = element_type_id {
                self.add_constraint(e, TypeConstraint::Equal(element_type_id));
                if let Value::List { elements, .. } = &*collection.borrow() {
                    for element in elements {
                        self.add_constraints(element, parent_constraints.clone());
                    }
                }
            }
        }
        self.visit(collection);
        self.visit(index);
    }

    fn visit_field_access(&mut self, _e: &ValueId, value: &Value) {
        let Value::FieldAccess { expr, .. } = value else {
            unreachable!()
        };
        self.visit(expr);
    }

    fn visit_assign(&mut self, _e: &ValueId, value: &Value) {
        let Value::Assign { place, value: v, .. } = value else {
            unreachable!()
        };
        if let Ok(place_type) = place.borrow().determine_type(self.m) {
            self.add_constraint(v, TypeConstraint::Equal(place_type.into()));
        }
        self.visit(place);
        self.visit(v);
    }

    fn visit_deref(&mut self, _e: &ValueId, value: &Value) {
        let Value::Deref { place, .. } = value else {
            unreachable!()
        };
        self.visit(place);
    }

    fn visit_cast(&mut self, _e: &ValueId, value: &Value) {
        let Value::Cast {
            value: v, target_type, ..
        } = value
        else {
            unreachable!()
        };
        self.add_constraint(v, TypeConstraint::Equal(*target_type));
        self.visit(v);
    }

    fn visit_borrow(&mut self, _e: &ValueId, value: &Value) {
        let Value::Borrow { place, .. } = value else {
            unreachable!()
        };
        self.visit(place);
    }

    fn visit_list(&mut self, e: &ValueId, value: &Value) {
        let Value::List { elements, .. } = value else {
            unreachable!()
        };

        if let Some(parent_constraints) = self.constraints.get(e).cloned() {
            for element in elements {
                let element_constraints: HashSet<TypeConstraint> = parent_constraints
                    .iter()
                    .filter_map(|c| {
                        let ty = c.type_id();
                        match &*ty {
                            Type::Array { element_type, .. }
                            | Type::SliceRef { element_type, .. }
                            | Type::SlicePtr { element_type, .. } => Some(TypeConstraint::Equal(*element_type)),
                            Type::Refine { base, .. } => match &**base {
                                Type::Array { element_type, .. }
                                | Type::SliceRef { element_type, .. }
                                | Type::SlicePtr { element_type, .. } => Some(TypeConstraint::Equal(*element_type)),
                                _ => Some(c.clone()),
                            },
                            _ => Some(c.clone()),
                        }
                    })
                    .collect();
                if !element_constraints.is_empty() {
                    self.add_constraints(element, element_constraints);
                }
            }
        }

        let concrete_element = elements.iter().find(|el| {
            !matches!(
                &*el.borrow(),
                Value::InferredInteger { .. } | Value::InferredFloat { .. }
            )
        });
        if let Some(concrete_element) = concrete_element
            && let Ok(concrete_type) = concrete_element.borrow().determine_type(self.m)
            && !concrete_type.is_inferred()
        {
            let concrete_type_id: TypeId = concrete_type.into();
            for element in elements.iter() {
                if matches!(
                    &*element.borrow(),
                    Value::InferredInteger { .. } | Value::InferredFloat { .. }
                ) {
                    self.add_constraint(element, TypeConstraint::Equal(concrete_type_id));
                }
            }
        }

        for element in elements {
            self.visit(element);
        }
    }

    fn visit_tuple(&mut self, _e: &ValueId, value: &Value) {
        let Value::Tuple { elements, .. } = value else {
            unreachable!()
        };
        for element in elements {
            self.visit(element);
        }
    }

    fn visit_if(&mut self, _e: &ValueId, value: &Value) {
        let Value::If {
            condition,
            true_branch,
            false_branch,
            ..
        } = value
        else {
            unreachable!()
        };
        let span = value.span();

        self.add_constraint(
            condition,
            TypeConstraint::eq_type(Type::Bool {
                span: ByteSpan::default(),
            }),
        );
        self.visit(condition);
        self.visit_block(true_branch);

        if let Some(false_branch) = false_branch {
            self.visit_block(false_branch);
            let true_type = true_branch.borrow().determine_type(self.m).ok();
            let false_type = false_branch.borrow().determine_type(self.m).ok();
            match (true_type, false_type) {
                (Some(t), Some(f)) if t != f => {
                    if !t.is_inferred() && !f.is_inferred() {
                        let t_never = matches!(&t, Type::Never { .. });
                        let f_never = matches!(&f, Type::Never { .. });
                        if !t_never && !f_never {
                            self.errors.insert(TypeErr::MismatchedBranchTypes {
                                span,
                                true_type: t.into(),
                                false_type: f.into(),
                            });
                        }
                    }
                }
                _ => {}
            }
        }
    }

    fn visit_while(&mut self, _e: &ValueId, value: &Value) {
        let Value::While { condition, body, .. } = value else {
            unreachable!()
        };
        self.add_constraint(
            condition,
            TypeConstraint::eq_type(Type::Bool {
                span: ByteSpan::default(),
            }),
        );
        self.visit(condition);
        self.visit_block(body);
    }

    fn visit_loop(&mut self, _e: &ValueId, value: &Value) {
        let Value::Loop { body, .. } = value else {
            unreachable!()
        };
        self.visit_block(body);
    }

    fn visit_return(&mut self, _e: &ValueId, value: &Value) {
        let Value::Return { value: v, .. } = value else {
            unreachable!()
        };
        if let Some(ret_type) = self.function_return_type {
            self.add_constraint(v, TypeConstraint::Equal(ret_type));
        }
        self.visit(v);
    }

    fn visit_block_value(&mut self, _e_id: &ValueId, value: &Value) {
        let Value::Block { block, .. } = value else {
            unreachable!()
        };
        for element in &mut block.borrow_mut().elements {
            self.visit_block_element(element);
        }
    }

    fn visit_call(&mut self, _e: &ValueId, value: &Value) {
        let Value::Call { callee, args, .. } = value else {
            unreachable!()
        };

        let callee_func_id: Option<FunctionId> = match &*callee.borrow() {
            Value::FunctionSymbol { id, .. } => {
                let func = id.borrow();
                if func.generics.is_some() && func.generics.as_ref().is_some_and(|g| !g.is_empty()) {
                    Some(id.clone())
                } else {
                    None
                }
            }
            _ => None,
        };

        if let Some(func_id) = callee_func_id
            && let Some(subst) = self.infer_generic_args_from_call(&func_id, &args.positional)
        {
            let mono_id = self.monomorphize_function(&func_id, &subst);
            callee.replace(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: mono_id,
            });
        }

        self.visit(callee);

        if let Value::FunctionSymbol { id, .. } = &*callee.borrow() {
            let func = id.borrow();
            for (i, arg) in args.positional.iter().enumerate() {
                if let Some(param) = func.params.get(i) {
                    self.add_constraint(arg, TypeConstraint::Equal(param.borrow().ty));
                }
            }
        }

        for arg in &args.positional {
            self.visit(arg);
        }
        for (_name, arg) in &args.named {
            self.visit(arg);
        }
    }

    fn visit_method_call(&mut self, e: &ValueId, value: &Value) {
        let Value::MethodCall {
            object,
            method_name,
            args,
            span,
        } = value
        else {
            unreachable!()
        };

        let obj_type: Option<TypeId> = object.borrow().determine_type(self.m).ok().map(|ty| ty.into());
        let method_id_opt: Option<FunctionId> =
            obj_type.and_then(|obj_type| self.m.get_method(&obj_type, method_name).cloned());

        if let Some(method_id) = method_id_opt {
            let is_generic = {
                let mf = method_id.borrow();
                mf.generics.is_some() && mf.generics.as_ref().is_some_and(|g| !g.is_empty())
            };
            if is_generic {
                if let Some(subst) = self.infer_generic_args_from_call(&method_id, &args.positional) {
                    let mono_id = self.monomorphize_function(&method_id, &subst);
                    e.replace(Value::Call {
                        span: ByteSpan::default(),
                        callee: ValueId::from(Value::FunctionSymbol {
                            span: ByteSpan::default(),
                            id: mono_id,
                        }),
                        args: args.clone(),
                    });
                    self.visit(e);
                    return;
                }
            } else {
                let mf = method_id.borrow();
                for (i, arg) in args.positional.iter().enumerate() {
                    if let Some(param) = mf.params.get(i) {
                        self.add_constraint(arg, TypeConstraint::Equal(param.borrow().ty));
                    }
                }
            }
        } else if let Some(recv_type) = obj_type {
            self.errors.insert(TypeErr::MethodNotFound {
                span: *span,
                method_name: method_name.to_string(),
                receiver_type: recv_type,
            });
        }

        self.visit(object);
        for arg in &args.positional {
            self.visit(arg);
        }
        for (_name, arg) in &args.named {
            self.visit(arg);
        }
    }
}
