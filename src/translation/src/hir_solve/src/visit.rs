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
use smallvec::SmallVec;
use std::unreachable;

impl<'m> Solver<'m> {
    /// Main entry: visit a value node, determine action, then recurse.
    pub(crate) fn visit(&mut self, e: &ValueId) {
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

    pub(crate) fn visit_block(&mut self, block: &BlockId) {
        for element in &mut block.borrow_mut().elements {
            self.visit_block_element(element);
        }
    }

    pub(crate) fn visit_block_element(&mut self, element: &mut BlockElement) {
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
                // Visit the initializer first, which may monomorphize structs and resolve
                // inferred literal types
                self.visit(&local_var.borrow().initializer);
                // Re-check the type of the initializer after visiting, since it may have been
                // monomorphized (e.g. Pair { first: 1_i32, second: 2_i32 } -> Pair::<i32>)
                // Also handle the case where the declared type was Parameterized (e.g. Pair<i32>)
                // and the initializer was monomorphized to a concrete Struct type
                {
                    let init_id = local_var.borrow().initializer.clone();
                    if let Ok(new_ty) = init_id.borrow().determine_type(self.m) {
                        let mut lv = local_var.borrow_mut();
                        let old_ty = lv.ty.clone();
                        // Check if the initializer's type has been monomorphized to a different struct.
                        // The initializer's Struct def may have been replaced with a monomorphized copy.
                        let old_is_parameterized = matches!(&*old_ty, Type::Parameterized { .. });
                        let new_type_id: TypeId = new_ty.clone().into();
                        // Update if: old was Parameterized -> now Struct,
                        // OR old was generic Struct -> now monomorphized Struct (different def)
                        let should_update = old_is_parameterized && matches!(&new_ty, Type::Struct { .. });
                        let old_is_generic_struct = matches!(&*old_ty, Type::Struct { .. });
                        let new_is_different_struct = matches!(&new_ty, Type::Struct { .. }) && old_ty != new_type_id;
                        if should_update || (old_is_generic_struct && new_is_different_struct) {
                            lv.ty = new_type_id;
                        }
                    }
                }
            }
        }
    }

    // ── Determine Action ──────────────────────────────────────────────────

    pub(crate) fn determine_action(&mut self, value: &Value, id: &ValueId) -> crate::constraints::NodeAction {
        match value {
            Value::InferredInteger { value, .. } => self.solve_inferred_integer(id, **value),
            Value::InferredFloat { value, .. } => self.solve_inferred_float(id, *value),
            _ => crate::constraints::NodeAction::NoChange,
        }
    }

    // ── Constraint helpers (used by both visit.rs and solver.rs) ──────────

    pub(crate) fn add_constraint(&mut self, id: &ValueId, constraint: TypeConstraint) {
        let changed = self.constraints.entry(id.clone()).or_default().insert(constraint);
        if changed {
            self.constraint_version = self.constraint_version.wrapping_add(1);
            self.add_to_worklist(id);
        }
    }

    pub(crate) fn add_constraints(&mut self, id: &ValueId, constraints: impl IntoIterator<Item = TypeConstraint>) {
        let entry = self.constraints.entry(id.clone()).or_default();
        let mut changed = false;
        for c in constraints {
            if entry.insert(c) {
                changed = true;
            }
        }
        if changed {
            self.constraint_version = self.constraint_version.wrapping_add(1);
            self.add_to_worklist(id);
        }
    }

    // ── Visit Children (dispatched by variant) ────────────────────────────

    /// Classify a Value variant into a handler tag without holding the borrow.
    /// This allows per-variant handlers to freely borrow the ValueId (including mutably).
    pub(crate) fn classify_value(value: &Value) -> u8 {
        match value {
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
            | Value::InferredFloat { .. } => 0, // Leaf

            Value::StructObject { .. } => 1,
            Value::EnumVariant { .. } => 2,
            Value::Binary { .. } => 3,
            Value::Unary { .. } => 4,
            Value::IndexAccess { .. } => 5,
            Value::FieldAccess { .. } => 6,
            Value::Assign { .. } => 7,
            Value::Deref { .. } => 8,
            Value::Cast { .. } => 9,
            Value::Borrow { .. } => 10,
            Value::List { .. } => 11,
            Value::Tuple { .. } => 12,
            Value::If { .. } => 13,
            Value::While { .. } => 14,
            Value::Loop { .. } => 15,
            Value::Break { .. } | Value::Continue { .. } => 16, // BreakContinue
            Value::Return { .. } => 17,
            Value::Block { .. } => 18,
            Value::Call { .. } => 19,
            Value::MethodCall { .. } => 20,
            Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => 21, // Symbol
        }
    }

    pub(crate) fn visit_children(&mut self, e: &ValueId) {
        // Determine the variant discriminant without holding a borrow on e,
        // so that per-variant handlers can freely borrow e (including mutably).
        let tag = {
            let v = e.borrow();
            Self::classify_value(&v)
        };
        match tag {
            0 => {} // Leaf
            1 => self.visit_struct_object(e),
            2 => self.visit_enum_variant(e),
            3 => self.visit_binary(e),
            4 => self.visit_unary(e),
            5 => self.visit_index_access(e),
            6 => self.visit_field_access(e),
            7 => self.visit_assign(e),
            8 => self.visit_deref(e),
            9 => self.visit_cast(e),
            10 => self.visit_borrow(e),
            11 => self.visit_list(e),
            12 => self.visit_tuple(e),
            13 => self.visit_if(e),
            14 => self.visit_while(e),
            15 => self.visit_loop(e),
            16 => {} // BreakContinue
            17 => self.visit_return(e),
            18 => self.visit_block_value(e),
            19 => self.visit_call(e),
            20 => self.visit_method_call(e),
            21 => {} // Symbol
            _ => panic!("unhandled Value variant in visit_children"),
        }
    }

    // ── Per-variant handlers ──────────────────────────────────────────────

    pub(crate) fn visit_struct_object(&mut self, e: &ValueId) {
        // Extract needed data in a narrow scope to avoid holding an immutable
        // borrow across the potential mutable borrow in the monomorphization path.
        let (struct_def, has_generics, fields, span) = {
            let v = e.borrow();
            let Value::StructObject { struct_def, fields, .. } = &*v else {
                unreachable!()
            };
            let has_generics = struct_def.borrow().generics.is_some();
            (struct_def.clone(), has_generics, fields.clone(), v.span())
        };

        if has_generics {
            // First try to infer generic args from concrete field values
            let subst = self
                .infer_generic_args_from_struct_fields(&struct_def, &fields)
                // If field-based inference failed, try to extract concrete type args
                // from parent constraints (e.g., from type annotations like `let x: Pair<i32>`)
                .or_else(|| self.infer_generic_args_from_constraints(e, &struct_def));

            if let Some(subst) = subst {
                let mono_struct_id = self.monomorphize_struct(&struct_def, &subst);
                {
                    let mut original = e.borrow_mut();
                    if let Value::StructObject { struct_def: sd, .. } = &mut *original {
                        *sd = mono_struct_id;
                    }
                }
            } else {
                // Could not infer generic args - report error and unbound params
                let (generic_name, unbound_params) = {
                    let sd = struct_def.borrow();
                    let unbound: Vec<String> = sd
                        .generics
                        .as_ref()
                        .map(|g| g.keys().map(|k| k.to_string()).collect())
                        .unwrap_or_default();
                    (sd.name.to_string(), unbound)
                };
                self.errors.insert(TypeErr::CannotInferTypeArgs {
                    span,
                    generic_name: generic_name.clone(),
                    reason: format!(
                        "cannot determine type arguments for struct `{}` from field values or context",
                        generic_name
                    ),
                });
                for param_name in &unbound_params {
                    self.errors.insert(TypeErr::UnboundGenericParam {
                        span,
                        param_name: param_name.clone(),
                        generic_name: generic_name.clone(),
                    });
                }
            }

            // After optional monomorphization, apply constraints and visit fields
            self.apply_struct_field_constraints(e);
        } else {
            self.apply_struct_field_constraints(e);
        }
    }

    /// Apply field type constraints and visit all fields of a struct object.
    /// Shared helper to avoid duplicated code between generic and non-generic paths.
    pub(crate) fn apply_struct_field_constraints(&mut self, e: &ValueId) {
        let value = e.borrow().clone();
        let Value::StructObject { struct_def, fields, .. } = &value else {
            return;
        };
        let struct_def_b = struct_def.borrow();
        for (field_name, field_value) in fields {
            if let Some(field) = struct_def_b.fields.get(field_name) {
                self.add_constraint(field_value, TypeConstraint::Equal(field.ty));
                self.visit(field_value);
            }
        }
    }

    pub(crate) fn visit_enum_variant(&mut self, e: &ValueId) {
        let value = &*e.borrow();
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

    pub(crate) fn visit_binary(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Binary { left, op, right, .. } = value else {
            unreachable!()
        };
        let span = value.span();

        let parent_constraints: SmallVec<[TypeConstraint; 2]> = self
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

                // If both operands are concrete and different types, that's ambiguous
                if !left_inferred && !right_inferred && left_ty != right_ty && is_arithmetic_op(op) {
                    self.errors.insert(TypeErr::AmbiguousType {
                        span,
                        description: "binary operation has operands of different types".into(),
                    });
                }

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

    pub(crate) fn visit_unary(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Unary { op, operand, .. } = value else {
            unreachable!()
        };
        let span = value.span();

        if let Some(constraints) = self.constraints.get(e).cloned() {
            self.add_constraints(operand, constraints.clone());
        }
        self.visit(operand);

        let parent_constraints: SmallVec<[TypeConstraint; 2]> = self
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

    pub(crate) fn visit_index_access(&mut self, e: &ValueId) {
        let value = &*e.borrow();
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

    pub(crate) fn visit_field_access(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::FieldAccess { expr, .. } = value else {
            unreachable!()
        };
        self.visit(expr);
    }

    pub(crate) fn visit_assign(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Assign { place, value: v, .. } = value else {
            unreachable!()
        };
        if let Ok(place_type) = place.borrow().determine_type(self.m) {
            self.add_constraint(v, TypeConstraint::Equal(place_type.into()));
        }
        self.visit(place);
        self.visit(v);
    }

    pub(crate) fn visit_deref(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Deref { place, .. } = value else {
            unreachable!()
        };
        self.visit(place);
    }

    pub(crate) fn visit_cast(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Cast {
            value: v, target_type, ..
        } = value
        else {
            unreachable!()
        };
        self.add_constraint(v, TypeConstraint::Equal(*target_type));
        self.visit(v);
    }

    pub(crate) fn visit_borrow(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Borrow { place, .. } = value else {
            unreachable!()
        };
        self.visit(place);
    }

    pub(crate) fn visit_list(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::List { elements, .. } = value else {
            unreachable!()
        };

        if let Some(parent_constraints) = self.constraints.get(e).cloned() {
            for element in elements {
                let element_constraints: SmallVec<[TypeConstraint; 2]> = parent_constraints
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

    pub(crate) fn visit_tuple(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Tuple { elements, .. } = value else {
            unreachable!()
        };
        for element in elements {
            self.visit(element);
        }
    }

    pub(crate) fn visit_if(&mut self, e: &ValueId) {
        let value = &*e.borrow();
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
                        if !matches!(&t, Type::Never { .. }) && !matches!(&f, Type::Never { .. }) {
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

    pub(crate) fn visit_while(&mut self, e: &ValueId) {
        let value = &*e.borrow();
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

    pub(crate) fn visit_loop(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Loop { body, .. } = value else {
            unreachable!()
        };
        self.visit_block(body);
    }

    pub(crate) fn visit_return(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Return { value: v, .. } = value else {
            unreachable!()
        };
        if let Some(ret_type) = self.function_return_type {
            self.add_constraint(v, TypeConstraint::Equal(ret_type));
        }
        self.visit(v);
    }

    pub(crate) fn visit_block_value(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Block { block, .. } = value else {
            unreachable!()
        };
        for element in &mut block.borrow_mut().elements {
            self.visit_block_element(element);
        }
    }

    pub(crate) fn visit_call(&mut self, e: &ValueId) {
        let value = &*e.borrow();
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

        // Check for named args in generic inference (#11)
        let has_named_args = !args.named.is_empty();

        if let Some(ref func_id) = callee_func_id
            && let Some(subst) = self.infer_generic_args_from_call(func_id, &args.positional)
        {
            let mono_id = self.monomorphize_function(func_id, &subst);
            callee.replace(Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: mono_id,
            });
        } else if let Some(ref func_id) = callee_func_id
            && has_named_args
            && let Some(subst) = self.infer_generic_args_from_call_named(func_id, args)
        {
            let mono_id = self.monomorphize_function(func_id, &subst);
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
            // Also add constraints for named args by looking up param names
            for (name, arg) in &args.named {
                if let Some(param) = func.params.iter().find(|p| p.borrow().name == *name) {
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

    pub(crate) fn visit_method_call(&mut self, e: &ValueId) {
        let value = &*e.borrow();
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
                // Try positional inference first, then named inference fallback
                let subst = self
                    .infer_generic_args_from_call(&method_id, &args.positional)
                    .or_else(|| {
                        if !args.named.is_empty() {
                            // Construct a synthetic call to use named arg inference
                            self.infer_generic_args_from_call_named(&method_id, args)
                        } else {
                            None
                        }
                    });
                if let Some(subst) = subst {
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
                // Add constraints for named args too
                for (name, arg) in &args.named {
                    if let Some(param) = mf.params.iter().find(|p| p.borrow().name == *name) {
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
