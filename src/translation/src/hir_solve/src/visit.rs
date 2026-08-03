use crate::constraints::{TypeConstraint, is_arithmetic_op, is_comparison_or_logical_op, propagate_to_children};
use crate::solver::Solver;
use crate::{constraints::NodeAction, diagnosis::TypeErr};
use nitrate_hir::{BlockElement, BlockId, FunctionId, Type, TypeId, Value, ValueId};
use nitrate_hir_type::HirGetType;
use nitrate_tree::SrcPos;
use smallvec::SmallVec;
use std::{matches, unreachable};

impl<'m> Solver<'m> {
    fn type_contains_any_generic_param(ty: &Type) -> bool {
        match ty {
            Type::GenericParam { .. } => true,
            Type::Array { element_type, .. } => Self::type_contains_any_generic_param(element_type),
            Type::Tuple { element_types, .. } => {
                element_types.iter().any(|et| Self::type_contains_any_generic_param(et))
            }
            Type::Reference { to, .. } | Type::Pointer { to, .. } => Self::type_contains_any_generic_param(to),
            Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
                Self::type_contains_any_generic_param(element_type)
            }
            Type::Refine { base, .. } => Self::type_contains_any_generic_param(base),
            _ => false,
        }
    }

    pub(crate) fn visit(&mut self, e: &ValueId) {
        let action = {
            let current_value = e.borrow();
            self.determine_action(&current_value, e)
        };
        match action {
            NodeAction::Replace(new_value) => {
                e.replace(new_value);
            }
            NodeAction::NoChange => self.visit_children(e),
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
                let lv = local_var.borrow();
                let ty = lv.ty;
                let is_inferred = ty.is_inferred();

                if let Some(init_id) = &lv.initializer {
                    if is_inferred {
                        if let Ok(determined_ty) = init_id.borrow().determine_type(self.m) {
                            local_var.borrow_mut().ty = determined_ty.into();
                        }
                    } else {
                        self.add_constraint(init_id, TypeConstraint::Equal(ty));
                    }

                    self.visit(init_id);

                    if let Ok(new_ty) = init_id.borrow().determine_type(self.m) {
                        let mut lv = local_var.borrow_mut();
                        let old_ty = lv.ty.clone();
                        let old_is_parameterized = matches!(&*old_ty, Type::Parameterized { .. });
                        let new_is_struct = matches!(&new_ty, Type::Struct { .. });
                        let new_type_id: TypeId = new_ty.into();
                        let should_update = old_is_parameterized && new_is_struct;
                        let old_is_generic_struct = matches!(&*old_ty, Type::Struct { .. });
                        let new_is_different_struct = new_is_struct && old_ty != new_type_id;
                        if should_update || (old_is_generic_struct && new_is_different_struct) {
                            lv.ty = new_type_id;
                        }
                    }
                }
            }
        }
    }

    pub(crate) fn determine_action(&mut self, value: &Value, id: &ValueId) -> NodeAction {
        match value {
            Value::InferredInteger { value, .. } => self.solve_inferred_integer(id, **value),
            Value::InferredFloat { value, .. } => self.solve_inferred_float(id, *value),
            Value::Range { .. } => self.solve_range(id),
            _ => NodeAction::NoChange,
        }
    }

    pub(crate) fn solve_range(&mut self, id: &ValueId) -> NodeAction {
        let (span, start, end, inclusive) = {
            let value = &*id.borrow();
            if let Value::Range {
                span,
                start,
                end,
                inclusive,
            } = value
            {
                (*span, start.clone(), end.clone(), *inclusive)
            } else {
                return NodeAction::NoChange;
            }
        };

        let has_start = start.is_some();
        let has_end = end.is_some();

        let replacement =
            crate::range::make_range_struct_object(self.m, span, start, end, inclusive, has_start, has_end);

        NodeAction::Replace(replacement)
    }

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
            | Value::InferredFloat { .. } => 0,

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
            Value::Break { .. } | Value::Continue { .. } => 16,
            Value::Return { .. } => 17,
            Value::Block { .. } => 18,
            Value::Call { .. } => 19,
            Value::MethodCall { .. } => 20,
            Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => 21,
            Value::Range { .. } => 22,
        }
    }

    pub(crate) fn visit_children(&mut self, e: &ValueId) {
        let tag = {
            let v = e.borrow();
            Self::classify_value(&v)
        };
        match tag {
            0 => {}
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
            16 => {}
            17 => self.visit_return(e),
            18 => self.visit_block_value(e),
            19 => self.visit_call(e),
            20 => self.visit_method_call(e),
            21 => {}
            22 => self.visit_range(e),
            _ => panic!("unhandled Value variant in visit_children"),
        }
    }

    pub(crate) fn visit_struct_object(&mut self, e: &ValueId) {
        let (struct_def, has_generics, fields, span) = {
            let v = e.borrow();
            let Value::StructObject { struct_def, fields, .. } = &*v else {
                unreachable!()
            };
            let has_generics = struct_def.borrow().generics.is_some();
            (struct_def.clone(), has_generics, fields.clone(), v.span())
        };

        if has_generics {
            for (_name, field_value) in &fields {
                self.visit(field_value);
            }

            let subst = self
                .infer_generic_args_from_struct_fields(&struct_def, &fields)
                .or_else(|| self.infer_generic_args_from_constraints(e, &struct_def));

            if let Some(subst) = subst {
                let mono_struct_id = self.monomorphize_struct(&struct_def, &subst);
                {
                    let mut original = e.borrow_mut();
                    if let Value::StructObject { struct_def: sd, .. } = &mut *original {
                        *sd = mono_struct_id;
                    }
                }
                self.apply_struct_field_constraints(e);
            } else {
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
                self.apply_struct_field_constraints(e);
            }
        } else {
            self.apply_struct_field_constraints(e);
        }
    }

    pub(crate) fn apply_struct_field_constraints(&mut self, e: &ValueId) {
        let value = e.borrow().clone();
        let Value::StructObject { struct_def, fields, .. } = &value else {
            return;
        };
        let struct_def_b = struct_def.borrow();
        let is_generic = struct_def_b.generics.is_some();
        for (field_name, field_value) in fields {
            if let Some(field) = struct_def_b.fields.get(field_name) {
                if is_generic && Self::type_contains_any_generic_param(&field.ty) {
                    self.visit(field_value);
                    continue;
                }
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
                            if let Some(bnds) = crate::bounds::extract_bounds_from_type(&result_ty) {
                                let comp_min = bnds.lo.max(0) as u128;
                                let comp_max = bnds.hi;
                                self.errors.insert(TypeErr::OperationResultOutOfRefinementBounds {
                                    span,
                                    refinement_type: result_ty,
                                    computed_min: comp_min,
                                    computed_max: comp_max,
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

                if !left_inferred && !right_inferred && left_ty != right_ty && is_arithmetic_op(op) {
                    self.errors.insert(TypeErr::AmbiguousType {
                        span,
                        description: "binary operation has operands of different types".into(),
                    });
                }

                if left_ty == right_ty && !left_inferred && is_arithmetic_op(op) {
                    self.add_constraint(e, TypeConstraint::Equal(left_ty));
                } else if left_inferred && right_inferred && is_arithmetic_op(op) {
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
                    if let Some(bnds) = crate::bounds::extract_bounds_from_type(&result_ty) {
                        let comp_min = bnds.lo.max(0) as u128;
                        let comp_max = bnds.hi;
                        self.errors.insert(TypeErr::OperationResultOutOfRefinementBounds {
                            span,
                            refinement_type: result_ty,
                            computed_min: comp_min,
                            computed_max: comp_max,
                        });
                    }
                }
            }
        }
    }

    pub(crate) fn visit_range(&mut self, e: &ValueId) {
        let value = &*e.borrow();
        let Value::Range { start, end, .. } = value else {
            unreachable!()
        };

        if let Some(start) = start {
            self.visit(start);
        }
        if let Some(end) = end {
            self.visit(end);
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
                span: SrcPos::default(),
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
                span: SrcPos::default(),
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
                span: SrcPos::default(),
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

        let has_named_args = !args.named.is_empty();

        if let Some(ref func_id) = callee_func_id
            && let Some(subst) = self.infer_generic_args_from_call(func_id, &args.positional)
        {
            let mono_id = self.monomorphize_function(func_id, &subst);
            callee.replace(Value::FunctionSymbol {
                span: SrcPos::default(),
                id: mono_id,
            });
        } else if let Some(ref func_id) = callee_func_id
            && has_named_args
            && let Some(subst) = self.infer_generic_args_from_call_named(func_id, args)
        {
            let mono_id = self.monomorphize_function(func_id, &subst);
            callee.replace(Value::FunctionSymbol {
                span: SrcPos::default(),
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
        let (object_id, method_name_result, args_result, span, obj_type) = {
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
            (object.clone(), method_name.clone(), args.clone(), *span, obj_type)
        };
        let method_id_opt: Option<FunctionId> =
            obj_type.and_then(|obj_type| self.m.get_method(&obj_type, &method_name_result).cloned());

        if let Some(method_id) = method_id_opt {
            let is_generic = {
                let mf = method_id.borrow();
                mf.generics.is_some() && mf.generics.as_ref().is_some_and(|g| !g.is_empty())
            };
            if is_generic {
                let mut args_with_self = args_result.clone();
                args_with_self.positional.insert(0, object_id.clone());
                let subst = self
                    .infer_generic_args_from_call(&method_id, &args_with_self.positional)
                    .or_else(|| {
                        if !args_result.named.is_empty() {
                            self.infer_generic_args_from_call_named(&method_id, &args_with_self)
                        } else {
                            None
                        }
                    });
                if let Some(subst) = subst {
                    let mono_id = self.monomorphize_function(&method_id, &subst);
                    e.replace(Value::Call {
                        span: SrcPos::default(),
                        callee: ValueId::from(Value::FunctionSymbol {
                            span: SrcPos::default(),
                            id: mono_id,
                        }),
                        args: args_with_self,
                    });
                    self.visit(e);
                    return;
                }
            } else {
                // Non-generic method: replace MethodCall with Call + FunctionSymbol,
                // passing the object as the first positional argument (self).
                // If the method takes &self (reference), auto-borrow the receiver.
                let mf = method_id.borrow();
                let first_param_is_ref = mf.params.first().map_or(false, |pid| {
                    matches!(&*pid.borrow().ty, Type::Reference { .. } | Type::SliceRef { .. })
                });

                let self_arg = if first_param_is_ref {
                    ValueId::from(Value::Borrow {
                        span: SrcPos::default(),
                        exclusive: false,
                        mutable: false,
                        place: object_id.clone(),
                    })
                } else {
                    object_id.clone()
                };

                let mut args_with_self = args_result.clone();
                args_with_self.positional.insert(0, self_arg);

                for (i, arg) in args_result.positional.iter().enumerate() {
                    if let Some(param) = mf.params.get(i + 1) {
                        self.add_constraint(arg, TypeConstraint::Equal(param.borrow().ty));
                    }
                }
                for (name, arg) in &args_result.named {
                    if let Some(param) = mf.params.iter().find(|p| p.borrow().name == *name) {
                        self.add_constraint(arg, TypeConstraint::Equal(param.borrow().ty));
                    }
                }
                drop(mf);
                e.replace(Value::Call {
                    span: SrcPos::default(),
                    callee: ValueId::from(Value::FunctionSymbol {
                        span: SrcPos::default(),
                        id: method_id,
                    }),
                    args: args_with_self,
                });
                self.visit(e);
                return;
            }
        } else if let Some(recv_type) = obj_type {
            self.errors.insert(TypeErr::MethodNotFound {
                span,
                method_name: method_name_result.to_string(),
                receiver_type: recv_type,
            });
        }

        self.visit(&object_id);
        for arg in &args_result.positional {
            self.visit(arg);
        }
        for (_name, arg) in &args_result.named {
            self.visit(arg);
        }
    }
}
