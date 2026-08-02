use crate::bounds::*;
use crate::constraints::*;
use crate::diagnosis::TypeErr;
use crate::monomorphize::*;
use crate::range::*;
use crate::substitution::Substitution;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    Arguments, BlockElement, Function, FunctionId, GlobalVariable, LiteralId, StructDef, StructDefId, SymbolTab, Type,
    TypeId, Value, ValueId, get_storage,
};
use nitrate_hir_evaluate::Evaluator;
use nitrate_hir_type::HirGetType;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::{BTreeMap, HashMap, HashSet};
use std::matches;
use std::ops::Deref;
use std::vec;

struct Solver<'a> {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    symbol_tab: &'a mut SymbolTab,
    errors: HashSet<TypeErr>,
    function_return_type: Option<TypeId>,
    mono_counter: u32,
    mono_cache: HashMap<MonoCacheKey, FunctionId>,
    struct_mono_cache: HashMap<MonoCacheKey, StructDefId>,
    worklist: HashSet<ValueId>,
    constraint_version: u64,
    mono_depth: u32,
    mono_in_progress: HashSet<MonoCacheKey>,
}

impl<'a> Solver<'a> {
    fn new(symbol_tab: &'a mut SymbolTab) -> Self {
        Self {
            constraints: HashMap::new(),
            symbol_tab,
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

    fn add_constraint(&mut self, id: &ValueId, constraint: TypeConstraint) {
        if self.constraints.entry(id.clone()).or_default().insert(constraint) {
            self.constraint_version = self.constraint_version.wrapping_add(1);
            self.worklist.insert(id.clone());
        }
    }

    fn add_all_elements_to_worklist(&mut self, body: &[BlockElement]) {
        for element in body {
            match element {
                BlockElement::Expr(id) => {
                    self.worklist.insert(id.clone());
                }
                BlockElement::Local(lv) => {
                    if let Some(init) = &lv.borrow().initializer {
                        self.worklist.insert(init.clone());
                    }
                }
            }
        }
    }

    fn get_effective_bounds(&self, id: &ValueId) -> Option<Bounds> {
        get_effective_bounds_impl(self, id)
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

    fn find_common_integer_type(constraints: &[TypeConstraint], value: u128) -> Option<TypeId> {
        let mut best: Option<TypeId> = None;
        for c in constraints {
            let ty = c.type_id();
            let eff = match &*ty {
                Type::Refine { base, .. } => *base,
                _ => ty,
            };
            if !eff.is_integer_primitive() {
                continue;
            }
            let fits = match &*eff {
                Type::I8 { .. } => value <= 127,
                Type::I16 { .. } => value <= 32767,
                Type::I32 { .. } => value <= 2147483647,
                Type::I64 { .. } => value <= 9223372036854775807,
                Type::U8 { .. } => value <= 255,
                Type::U16 { .. } => value <= 65535,
                Type::U32 { .. } => value <= 4294967295,
                Type::U64 { .. } => value <= 18446744073709551615,
                Type::I128 { .. } => value <= 170141183460469231731687303715884105727,
                _ => true,
            };
            if fits {
                match (best, &*eff) {
                    (None, _) => best = Some(eff),
                    _ if eff.is_signed_primitive() && !best.unwrap().is_signed_primitive() => best = Some(eff),
                    _ if Self::type_bit_width(&eff) > Self::type_bit_width(&best.unwrap()) => best = Some(eff),
                    _ => {}
                }
            }
        }
        best
    }

    fn solve_inferred_integer(&mut self, id: &ValueId, value: u128) -> NodeAction {
        let constraints: Vec<TypeConstraint> = self
            .constraints
            .get(id)
            .cloned()
            .unwrap_or_default()
            .into_iter()
            .collect();
        if constraints.is_empty() {
            return NodeAction::NoChange;
        }
        let span = id.borrow().span();
        let mut check_errors = Vec::new();
        let mut has_non_integer = false;
        for c in &constraints {
            let ty = c.type_id();
            if let Type::Refine { .. } = &*ty {
                if !check_literal_against_refinement(value, &ty) {
                    check_errors.push((ty, value as i128));
                }
            }
            let eff = match &*ty {
                Type::Refine { base, .. } => *base,
                _ => ty,
            };
            if !eff.is_integer_primitive() {
                has_non_integer = true;
            }
        }
        if has_non_integer {
            return NodeAction::NoChange;
        }
        for (refinement_ty, val) in &check_errors {
            self.errors.insert(TypeErr::IntegerLiteralOutOfRefinementBounds {
                span,
                value: *val as u128,
                refinement_type: *refinement_ty,
            });
        }
        let best =
            Self::find_common_integer_type(&constraints, value).unwrap_or_else(|| TypeId::from(Type::I32 { span }));
        match &*best {
            Type::I8 { .. } => i8::try_from(value)
                .map(|v| NodeAction::Replace(Value::I8 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::I16 { .. } => i16::try_from(value)
                .map(|v| NodeAction::Replace(Value::I16 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::I32 { .. } => i32::try_from(value)
                .map(|v| NodeAction::Replace(Value::I32 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::I64 { .. } => i64::try_from(value)
                .map(|v| NodeAction::Replace(Value::I64 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::I128 { .. } => i128::try_from(value)
                .map(|v| {
                    NodeAction::Replace(Value::I128 {
                        span,
                        value: Box::new(v),
                    })
                })
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::U8 { .. } => u8::try_from(value)
                .map(|v| NodeAction::Replace(Value::U8 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::U16 { .. } => u16::try_from(value)
                .map(|v| NodeAction::Replace(Value::U16 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::U32 { .. } => u32::try_from(value)
                .map(|v| NodeAction::Replace(Value::U32 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::U64 { .. } => u64::try_from(value)
                .map(|v| NodeAction::Replace(Value::U64 { span, value: v }))
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            Type::U128 { .. } => NodeAction::Replace(Value::U128 {
                span,
                value: Box::new(value),
            }),
            Type::USize { .. } => u64::try_from(value)
                .map(|v| {
                    NodeAction::Replace(Value::USize {
                        span,
                        bits: 64,
                        value: v,
                    })
                })
                .unwrap_or_else(|_| {
                    self.errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: best,
                    });
                    NodeAction::NoChange
                }),
            _ => NodeAction::NoChange,
        }
    }

    fn solve_inferred_float(&mut self, id: &ValueId, value: OrderedFloat<f64>) -> NodeAction {
        let span = id.borrow().span();
        let constraints: Vec<TypeConstraint> = self
            .constraints
            .get(id)
            .cloned()
            .unwrap_or_default()
            .into_iter()
            .collect();
        if constraints.is_empty() {
            return NodeAction::NoChange;
        }
        let mut best: Option<TypeId> = None;
        let mut has_non_float = false;
        for c in &constraints {
            let ty = c.type_id();
            if !ty.is_float_primitive() {
                has_non_float = true;
                continue;
            }
            if let Some(cur) = best {
                if matches!(&*ty, Type::F64 { .. }) && !matches!(&*cur, Type::F64 { .. }) {
                    best = Some(ty);
                }
            } else {
                best = Some(ty);
            }
        }
        if has_non_float {
            return NodeAction::NoChange;
        }
        if let Some(b) = best {
            return match &*b {
                Type::F32 { .. } => NodeAction::Replace(Value::F32 {
                    span,
                    value: OrderedFloat(*value as f32),
                }),
                Type::F64 { .. } => NodeAction::Replace(Value::F64 { span, value }),
                _ => unreachable!(),
            };
        }
        NodeAction::NoChange
    }

    fn solve_range(&mut self, id: &ValueId) -> NodeAction {
        let v = id.borrow();
        let (span, start_v, end_v, inclusive) = match &*v {
            Value::Range {
                span,
                start,
                end,
                inclusive,
            } => (*span, start.is_some(), end.is_some(), *inclusive),
            _ => return NodeAction::NoChange,
        };
        let start = match &*v {
            Value::Range { start, .. } => start.clone(),
            _ => None,
        };
        let end = match &*v {
            Value::Range { end, .. } => end.clone(),
            _ => None,
        };
        drop(v);
        NodeAction::Replace(make_range_struct_object(
            self.symbol_tab,
            span,
            start,
            end,
            inclusive,
            start_v,
            end_v,
        ))
    }

    // ── Visitor ──────────────────────────────────────────────────

    fn visit(&mut self, e: &ValueId) {
        let action = {
            let cv = e.borrow();
            self.determine_action(&cv, e)
        };
        match action {
            NodeAction::Replace(v) => {
                e.replace(v);
            }
            NodeAction::NoChange => self.visit_children(e),
        }
    }

    fn determine_action(&mut self, value: &Value, id: &ValueId) -> NodeAction {
        match value {
            Value::InferredInteger { value, .. } => self.solve_inferred_integer(id, **value),
            Value::InferredFloat { value, .. } => self.solve_inferred_float(id, *value),
            Value::Range { .. } => self.solve_range(id),
            _ => NodeAction::NoChange,
        }
    }

    fn visit_children(&mut self, e: &ValueId) {
        let tag = {
            let v = e.borrow();
            classify_value(&v)
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
            _ => {}
        }
    }

    fn visit_block(&mut self, block: &nitrate_hir::BlockId) {
        for element in &mut block.borrow_mut().elements {
            self.visit_block_element(element);
        }
    }

    fn visit_block_element(&mut self, element: &mut BlockElement) {
        match element {
            BlockElement::Expr(e) => self.visit(e),
            BlockElement::Local(local_var) => {
                let lv = local_var.borrow();
                let ty = lv.ty;
                let is_inferred = ty.is_inferred();
                if let Some(init_id) = &lv.initializer {
                    if is_inferred {
                        if let Ok(d) = init_id.borrow().determine_type(self.symbol_tab) {
                            local_var.borrow_mut().ty = d.into();
                        }
                    } else {
                        self.add_constraint(init_id, TypeConstraint::Equal(ty));
                    }
                    self.visit(init_id);
                    if let Ok(new_ty) = init_id.borrow().determine_type(self.symbol_tab) {
                        let mut lv = local_var.borrow_mut();
                        if matches!(&*lv.ty, Type::Parameterized { .. }) && matches!(&new_ty, Type::Struct { .. }) {
                            lv.ty = new_ty.into();
                        }
                    }
                }
            }
        }
    }

    fn visit_struct_object(&mut self, e: &ValueId) {
        let (struct_def, has_generics, fields, span) = {
            let v = e.borrow();
            let Value::StructObject { struct_def, fields, .. } = &*v else {
                return;
            };
            (
                struct_def.clone(),
                struct_def.borrow().generics.is_some(),
                fields.clone(),
                v.span(),
            )
        };
        if has_generics {
            for (_, fv) in &fields {
                self.visit(fv);
            }
            let subst = self
                .infer_generic_args_from_struct_fields(&struct_def, &fields)
                .or_else(|| self.infer_generic_args_from_constraints(e, &struct_def));
            if let Some(subst) = subst {
                let mono_id = self.monomorphize_struct(&struct_def, &subst);
                {
                    let mut orig = e.borrow_mut();
                    if let Value::StructObject { struct_def: sd, .. } = &mut *orig {
                        *sd = mono_id;
                    }
                }
                self.apply_struct_field_constraints(e);
            } else {
                let generics = struct_def.borrow();
                let name = generics.name.to_string();
                self.errors.insert(TypeErr::CannotInferTypeArgs {
                    span,
                    generic_name: name.clone(),
                    reason: format!(
                        "cannot determine type arguments for struct `{}` from field values or context",
                        name
                    ),
                });
                for pn in generics.generics.as_ref().into_iter().flat_map(|g| g.keys()) {
                    self.errors.insert(TypeErr::UnboundGenericParam {
                        span,
                        param_name: pn.to_string(),
                        generic_name: name.clone(),
                    });
                }
                self.apply_struct_field_constraints(e);
            }
        } else {
            self.apply_struct_field_constraints(e);
        }
    }

    fn apply_struct_field_constraints(&mut self, e: &ValueId) {
        let value = e.borrow().clone();
        let Value::StructObject { struct_def, fields, .. } = &value else {
            return;
        };
        let sd = struct_def.borrow();
        let is_generic = sd.generics.is_some();
        for (fn_, fv) in fields {
            if let Some(field) = sd.fields.get(fn_) {
                if is_generic && type_contains_any_generic_param(&field.ty) {
                    self.visit(fv);
                    continue;
                }
                self.add_constraint(fv, TypeConstraint::Equal(field.ty));
                self.visit(fv);
            }
        }
    }

    fn visit_enum_variant(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::EnumVariant {
            enum_def,
            variant,
            value: inner,
            ..
        } = &*value
        else {
            return;
        };
        let vt = enum_def
            .borrow()
            .variants
            .iter()
            .find(|item| item.name == *variant)
            .expect("variant not present")
            .ty;
        self.add_constraint(inner, TypeConstraint::Equal(vt));
        self.visit(inner);
    }

    fn visit_binary(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Binary { left, op, right, .. } = &*value else {
            return;
        };
        let span = value.span();
        if let Some(pc) = self.constraints.get(e).cloned() {
            for c in &pc {
                let result_ty = c.type_id();
                if is_arithmetic_op(op) {
                    if let (Some(lb), Some(rb)) = (self.get_effective_bounds(left), self.get_effective_bounds(right)) {
                        if let Some(res) = compute_binary_bounds(op, lb, rb) {
                            if !check_bounds_against_constraint(res, &result_ty) {
                                if let Some(bnds) = extract_bounds_from_type(&result_ty) {
                                    self.errors.insert(TypeErr::OperationResultOutOfRefinementBounds {
                                        span,
                                        refinement_type: result_ty,
                                        computed_min: bnds.lo.max(0) as u128,
                                        computed_max: bnds.hi,
                                    });
                                }
                            }
                        }
                    }
                }
            }
            for c in pc {
                self.add_constraint(left, c.clone());
                self.add_constraint(right, c.clone());
            }
        }
        if let Ok(lt) = left.borrow().determine_type(self.symbol_tab) {
            if let Ok(rt) = right.borrow().determine_type(self.symbol_tab) {
                let lt_id: TypeId = lt.clone().into();
                let rt_id: TypeId = rt.clone().into();
                if !lt.is_inferred() && !rt.is_inferred() && lt_id != rt_id && is_arithmetic_op(op) {
                    self.errors.insert(TypeErr::AmbiguousType {
                        span,
                        description: "binary operation has operands of different types".into(),
                    });
                }
                if lt_id == rt_id && !lt.is_inferred() && is_arithmetic_op(op) {
                    self.add_constraint(e, TypeConstraint::Equal(lt_id));
                }
                if is_comparison_or_logical_op(op) {
                    self.add_constraint(e, TypeConstraint::eq_type(Type::Bool { span }));
                    if !lt.is_inferred() {
                        self.add_constraint(right, TypeConstraint::Equal(lt_id));
                    }
                    if !rt.is_inferred() {
                        self.add_constraint(left, TypeConstraint::Equal(rt_id));
                    }
                }
            }
        }
        self.visit(left);
        self.visit(right);
    }

    fn visit_unary(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Unary { op, operand, .. } = &*value else {
            return;
        };
        let span = value.span();
        if let Some(constraints) = self.constraints.get(e).cloned() {
            for c in &constraints {
                self.add_constraint(operand, c.clone());
            }
        }
        self.visit(operand);
        if let Some(constraints) = self.constraints.get(e).cloned() {
            for c in &constraints {
                let result_ty = c.type_id();
                if let Some(ob) = self.get_effective_bounds(operand) {
                    let res = compute_unary_bounds(op, ob);
                    if !check_bounds_against_constraint(res, &result_ty) {
                        if let Some(bnds) = extract_bounds_from_type(&result_ty) {
                            self.errors.insert(TypeErr::OperationResultOutOfRefinementBounds {
                                span,
                                refinement_type: result_ty,
                                computed_min: bnds.lo.max(0) as u128,
                                computed_max: bnds.hi,
                            });
                        }
                    }
                }
            }
        }
    }

    fn visit_range(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::Range { start, end, .. } = &*v else { return };
        if let Some(s) = start {
            self.visit(s);
        }
        if let Some(e) = end {
            self.visit(e);
        }
    }

    fn visit_index_access(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::IndexAccess { collection, index, .. } = &*v else {
            return;
        };
        self.add_constraint(
            index,
            TypeConstraint::eq_type(Type::USize {
                span: ByteSpan::default(),
            }),
        );
        if let Some(pc) = self.constraints.get(e).cloned() {
            if let Ok(ct) = collection.borrow().determine_type(self.symbol_tab) {
                let et = match &ct {
                    Type::Array { element_type, .. }
                    | Type::SliceRef { element_type, .. }
                    | Type::SlicePtr { element_type, .. } => Some(*element_type),
                    _ => None,
                };
                if let Some(et) = et {
                    self.add_constraint(e, TypeConstraint::Equal(et));
                    if let Value::List { elements, .. } = &*collection.borrow() {
                        for el in elements {
                            for c in &pc {
                                self.add_constraint(el, c.clone());
                            }
                        }
                    }
                }
            }
        }
        self.visit(collection);
        self.visit(index);
    }

    fn visit_field_access(&mut self, e: &ValueId) {
        let v = e.borrow();
        if let Value::FieldAccess { expr, .. } = &*v {
            self.visit(expr);
        }
    }
    fn visit_assign(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::Assign { place, value: val, .. } = &*v else {
            return;
        };
        if let Ok(pt) = place.borrow().determine_type(self.symbol_tab) {
            self.add_constraint(val, TypeConstraint::Equal(pt.into()));
        }
        self.visit(place);
        self.visit(val);
    }
    fn visit_deref(&mut self, e: &ValueId) {
        let v = e.borrow();
        if let Value::Deref { place, .. } = &*v {
            self.visit(place);
        }
    }
    fn visit_cast(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::Cast {
            value: val,
            target_type,
            ..
        } = &*v
        else {
            return;
        };
        self.add_constraint(val, TypeConstraint::Equal(*target_type));
        self.visit(val);
    }
    fn visit_borrow(&mut self, e: &ValueId) {
        let v = e.borrow();
        if let Value::Borrow { place, .. } = &*v {
            self.visit(place);
        }
    }

    fn visit_list(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::List { elements, .. } = &*v else { return };
        if let Some(pc) = self.constraints.get(e).cloned() {
            for el in elements {
                for c in &pc {
                    let ty = c.type_id();
                    let et = match &*ty {
                        Type::Array { element_type, .. }
                        | Type::SliceRef { element_type, .. }
                        | Type::SlicePtr { element_type, .. } => Some(*element_type),
                        Type::Refine { base, .. } => match &**base {
                            Type::Array { element_type, .. }
                            | Type::SliceRef { element_type, .. }
                            | Type::SlicePtr { element_type, .. } => Some(*element_type),
                            _ => Some(ty),
                        },
                        _ => Some(ty),
                    };
                    if let Some(et) = et {
                        self.add_constraint(el, TypeConstraint::Equal(et));
                    }
                }
            }
        }
        let concrete = elements.iter().find(|el| {
            !matches!(
                &*el.borrow(),
                Value::InferredInteger { .. } | Value::InferredFloat { .. }
            )
        });
        if let Some(ce) = concrete {
            if let Ok(ct) = ce.borrow().determine_type(self.symbol_tab) {
                if !ct.is_inferred() {
                    for el in elements {
                        if matches!(
                            &*el.borrow(),
                            Value::InferredInteger { .. } | Value::InferredFloat { .. }
                        ) {
                            self.add_constraint(el, TypeConstraint::Equal(ct.clone().into()));
                        }
                    }
                }
            }
        }
        for el in elements {
            self.visit(el);
        }
    }

    fn visit_tuple(&mut self, e: &ValueId) {
        let v = e.borrow();
        if let Value::Tuple { elements, .. } = &*v {
            for el in elements {
                self.visit(el);
            }
        }
    }

    fn visit_if(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::If {
            condition,
            true_branch,
            false_branch,
            ..
        } = &*v
        else {
            return;
        };
        let span = v.span();
        self.add_constraint(
            condition,
            TypeConstraint::eq_type(Type::Bool {
                span: ByteSpan::default(),
            }),
        );
        self.visit(condition);
        self.visit_block(true_branch);
        if let Some(fb) = false_branch {
            self.visit_block(fb);
            if let (Ok(tt), Ok(ft)) = (
                true_branch.borrow().determine_type(self.symbol_tab),
                fb.borrow().determine_type(self.symbol_tab),
            ) {
                if tt != ft
                    && !tt.is_inferred()
                    && !ft.is_inferred()
                    && !matches!(&tt, Type::Never { .. })
                    && !matches!(&ft, Type::Never { .. })
                {
                    self.errors.insert(TypeErr::MismatchedBranchTypes {
                        span,
                        true_type: tt.into(),
                        false_type: ft.into(),
                    });
                }
            }
        }
    }

    fn visit_while(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::While { condition, body, .. } = &*v else {
            return;
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
    fn visit_loop(&mut self, e: &ValueId) {
        let v = e.borrow();
        if let Value::Loop { body, .. } = &*v {
            self.visit_block(body);
        }
    }
    fn visit_return(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::Return { value: val, .. } = &*v else { return };
        if let Some(rt) = self.function_return_type {
            self.add_constraint(val, TypeConstraint::Equal(rt));
        }
        self.visit(val);
    }
    fn visit_block_value(&mut self, e: &ValueId) {
        let v = e.borrow();
        if let Value::Block { block, .. } = &*v {
            for element in &mut block.borrow_mut().elements {
                self.visit_block_element(element);
            }
        }
    }

    fn visit_call(&mut self, e: &ValueId) {
        let v = e.borrow();
        let Value::Call { callee, args, .. } = &*v else { return };
        let callee_func_id = match &*callee.borrow() {
            Value::FunctionSymbol { id, .. } if id.borrow().generics.as_ref().is_some_and(|g| !g.is_empty()) => {
                Some(id.clone())
            }
            _ => None,
        };
        if let Some(ref fid) = callee_func_id {
            if let Some(subst) = self.infer_generic_args_from_call(fid, &args.positional) {
                let mono_id = self.monomorphize_function(fid, &subst);
                callee.replace(Value::FunctionSymbol {
                    span: ByteSpan::default(),
                    id: mono_id,
                });
            }
        }
        if let Some(ref fid) = callee_func_id {
            if !args.named.is_empty() {
                if let Some(subst) = self.infer_generic_args_from_call_named(fid, args) {
                    let mono_id = self.monomorphize_function(fid, &subst);
                    callee.replace(Value::FunctionSymbol {
                        span: ByteSpan::default(),
                        id: mono_id,
                    });
                }
            }
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
        for (_, arg) in &args.named {
            self.visit(arg);
        }
    }

    fn visit_method_call(&mut self, e: &ValueId) {
        let (object_id, method_name, args, span, obj_type) = {
            let v = e.borrow();
            let Value::MethodCall {
                object,
                method_name,
                args,
                span,
            } = &*v
            else {
                return;
            };
            let ot: Option<TypeId> = object.borrow().determine_type(self.symbol_tab).ok().map(|ty| ty.into());
            (object.clone(), method_name.clone(), args.clone(), *span, ot)
        };
        if let Some(ot) = obj_type {
            if let Some(method_id) = self.symbol_tab.get_method(&ot, &method_name).cloned() {
                let is_generic = method_id.borrow().generics.as_ref().is_some_and(|g| !g.is_empty());
                if is_generic {
                    let mut args_with_self = args.clone();
                    args_with_self.positional.insert(0, object_id.clone());
                    if let Some(subst) = self
                        .infer_generic_args_from_call(&method_id, &args_with_self.positional)
                        .or_else(|| {
                            if !args.named.is_empty() {
                                self.infer_generic_args_from_call_named(&method_id, &args_with_self)
                            } else {
                                None
                            }
                        })
                    {
                        let mono_id = self.monomorphize_function(&method_id, &subst);
                        e.replace(Value::Call {
                            span: ByteSpan::default(),
                            callee: ValueId::from(Value::FunctionSymbol {
                                span: ByteSpan::default(),
                                id: mono_id,
                            }),
                            args: args_with_self,
                        });
                        self.visit(e);
                        return;
                    }
                } else {
                    let mf = method_id.borrow();
                    let first_param_is_ref = mf.params.first().map_or(false, |pid| {
                        matches!(&*pid.borrow().ty, Type::Reference { .. } | Type::SliceRef { .. })
                    });
                    let self_arg = if first_param_is_ref {
                        ValueId::from(Value::Borrow {
                            span: ByteSpan::default(),
                            exclusive: false,
                            mutable: false,
                            place: object_id.clone(),
                        })
                    } else {
                        object_id.clone()
                    };
                    let mut args_with_self = args.clone();
                    args_with_self.positional.insert(0, self_arg);
                    for (i, arg) in args.positional.iter().enumerate() {
                        if let Some(param) = mf.params.get(i + 1) {
                            self.add_constraint(arg, TypeConstraint::Equal(param.borrow().ty));
                        }
                    }
                    for (name, arg) in &args.named {
                        if let Some(param) = mf.params.iter().find(|p| p.borrow().name == *name) {
                            self.add_constraint(arg, TypeConstraint::Equal(param.borrow().ty));
                        }
                    }
                    drop(mf);
                    e.replace(Value::Call {
                        span: ByteSpan::default(),
                        callee: ValueId::from(Value::FunctionSymbol {
                            span: ByteSpan::default(),
                            id: method_id,
                        }),
                        args: args_with_self,
                    });
                    self.visit(e);
                    return;
                }
            } else {
                self.errors.insert(TypeErr::MethodNotFound {
                    span,
                    method_name: method_name.to_string(),
                    receiver_type: ot,
                });
            }
        }
        self.visit(&object_id);
        for arg in &args.positional {
            self.visit(arg);
        }
        for (_, arg) in &args.named {
            self.visit(arg);
        }
    }

    // ── Generics / Monomorphization ────────────────────────────────

    fn mono_cache_key(&self, func_id: &FunctionId, subst: &Substitution) -> MonoCacheKey {
        let mut sa: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sa.sort_by_key(|(k, _)| *k);
        MonoCacheKey::new(func_id.as_usize(), &sa)
    }
    fn struct_mono_cache_key(&self, sid: &StructDefId, subst: &Substitution) -> MonoCacheKey {
        let mut sa: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sa.sort_by_key(|(k, _)| *k);
        MonoCacheKey::new(sid.as_usize(), &sa)
    }

    fn infer_generic_args_from_call(&self, fid: &FunctionId, args: &[ValueId]) -> Option<Substitution> {
        let func = fid.borrow();
        let generics = func.generics.as_ref()?;
        if generics.is_empty() {
            return Some(Substitution::default());
        }
        let mut subst = Substitution::default();
        let ptypes: Vec<TypeId> = func.params.iter().map(|p| p.borrow().ty).collect();
        if ptypes.len() != args.len() {
            return None;
        }
        for (a, p) in args.iter().zip(ptypes.iter()) {
            let at = a.borrow().determine_type(self.symbol_tab).ok()?;
            unify_types_with_subst(&at, p, &mut subst);
        }
        if subst.mapping.is_empty() { None } else { Some(subst) }
    }

    fn infer_generic_args_from_call_named(&self, fid: &FunctionId, args: &Arguments<ValueId>) -> Option<Substitution> {
        let func = fid.borrow();
        let generics = func.generics.as_ref()?;
        if generics.is_empty() {
            return Some(Substitution::default());
        }
        let mut subst = Substitution::default();
        let mut any = false;
        for (name, v) in &args.named {
            if let Some(pid) = func.params.iter().find(|p| p.borrow().name == *name) {
                let pt = pid.borrow().ty;
                if let Ok(at) = v.borrow().determine_type(self.symbol_tab) {
                    if at.is_inferred() {
                        continue;
                    }
                    any = true;
                    unify_types_with_subst(&at, &pt, &mut subst);
                }
            }
        }
        for (i, v) in args.positional.iter().enumerate() {
            if let Some(pid) = func.params.get(i) {
                let pt = pid.borrow().ty;
                if let Ok(at) = v.borrow().determine_type(self.symbol_tab) {
                    if at.is_inferred() {
                        continue;
                    }
                    any = true;
                    unify_types_with_subst(&at, &pt, &mut subst);
                }
            }
        }
        if !any || subst.mapping.is_empty() {
            return None;
        }
        for (pn, _) in generics.iter() {
            let idx = func.params.iter().find_map(|pid| {
                let p = pid.borrow();
                if type_contains_generic_param_name(&p.ty, pn) {
                    if let Type::GenericParam { index, .. } = &*p.ty {
                        Some(*index)
                    } else {
                        None
                    }
                } else {
                    None
                }
            });
            if let Some(idx) = idx {
                if !subst.mapping.contains_key(&idx) {
                    return None;
                }
            }
        }
        Some(subst)
    }

    fn infer_generic_args_from_struct_fields(
        &self,
        sid: &StructDefId,
        field_values: &[(NString, ValueId)],
    ) -> Option<Substitution> {
        let sd = sid.borrow();
        let generics = sd.generics.as_ref()?;
        if generics.is_empty() {
            return Some(Substitution::default());
        }
        let mut subst = Substitution::default();
        let mut any = false;
        struct GI {
            index: u32,
            appears: bool,
        }
        let mut pi: BTreeMap<NString, GI> = BTreeMap::new();
        for (pn, pd) in generics.iter() {
            let idx = pd
                .as_ref()
                .and_then(|t| {
                    if let Type::GenericParam { index, .. } = &**t {
                        Some(*index)
                    } else {
                        None
                    }
                })
                .unwrap_or_else(|| generics.keys().position(|k| k == pn).unwrap_or(0) as u32);
            pi.insert(
                pn.clone(),
                GI {
                    index: idx,
                    appears: false,
                },
            );
        }
        for (fn_, fv) in field_values {
            if let Some(field) = sd.fields.get(fn_) {
                for (pn, info) in pi.iter_mut() {
                    if type_contains_generic_param_name(&field.ty, pn) {
                        info.appears = true;
                    }
                }
                let ft = &*field.ty;
                if let Ok(at) = fv.borrow().determine_type(self.symbol_tab) {
                    if at.is_inferred() {
                        continue;
                    }
                    any = true;
                    unify_types_with_subst(&at, ft, &mut subst);
                }
            }
        }
        if !any {
            return None;
        }
        for info in pi.values() {
            if info.appears && !subst.mapping.contains_key(&info.index) {
                return None;
            }
        }
        Some(subst)
    }

    fn infer_generic_args_from_constraints(&self, vid: &ValueId, sid: &StructDefId) -> Option<Substitution> {
        let sd = sid.borrow();
        let generics = sd.generics.as_ref()?;
        if generics.is_empty() {
            return Some(Substitution::default());
        }
        let mut pni: BTreeMap<NString, u32> = BTreeMap::new();
        for field in sd.fields.values() {
            collect_generic_params_from_type(&field.ty, &mut pni);
        }
        let opn: Vec<&NString> = generics.keys().collect();
        let constraints = self.constraints.get(vid)?;
        for c in constraints {
            let ty = c.type_id();
            let (ga, sd2) = match &*ty {
                Type::Parameterized { base, args, .. } => {
                    if let Type::Struct { def, .. } = &**base {
                        (Some(args.positional.clone()), Some(def.clone()))
                    } else {
                        (None, None)
                    }
                }
                Type::Struct { def, .. } => (None, Some(def.clone())),
                _ => (None, None),
            };
            if let Some(ref args) = ga {
                if let Some(ref cd) = sd2 {
                    if cd.as_usize() != sid.as_usize() {
                        continue;
                    }
                    if args.len() != opn.len() {
                        continue;
                    }
                    let mut subst = Substitution::default();
                    for (i, pn) in opn.iter().enumerate() {
                        if let Some(idx) = pni.get(*pn) {
                            subst.mapping.insert(*idx, args[i]);
                        } else if i < args.len() {
                            subst.mapping.insert(i as u32, args[i]);
                        }
                    }
                    if !subst.mapping.is_empty() {
                        return Some(subst);
                    }
                }
            }
        }
        None
    }

    fn monomorphize_function(&mut self, fid: &FunctionId, subst: &Substitution) -> FunctionId {
        if self.mono_depth >= MAX_MONO_DEPTH {
            panic!("mono depth limit exceeded");
        }
        let ck = self.mono_cache_key(fid, subst);
        if let Some(existing) = self.mono_cache.get(&ck) {
            return existing.clone();
        }
        if !self.mono_in_progress.insert(ck) {
            return fid.clone();
        }
        self.mono_depth += 1;
        self.mono_counter += 1;
        let func = fid.borrow();
        let mono_name = format!("{}::<mono-{}>", func.name, self.mono_counter);
        let new_params: Vec<nitrate_hir::ParameterId> = func
            .params
            .iter()
            .map(|pid| {
                let p = pid.borrow();
                let nt = subst.apply(&p.ty);
                nitrate_hir::ParameterId::from(nitrate_hir::Parameter {
                    span: p.span,
                    attributes: p.attributes.clone(),
                    is_mutable: p.is_mutable,
                    name: p.name.clone(),
                    ty: TypeId::from(nt),
                    default_value: p.default_value.clone(),
                })
            })
            .collect();
        let new_ret = TypeId::from(subst.apply(&func.return_type));
        let new_body = func
            .body
            .as_ref()
            .map(|body| body.iter().map(|el| clone_block_element(el, subst)).collect());
        let mono_func = Function {
            span: ByteSpan::default(),
            visibility: func.visibility,
            attributes: func.attributes.clone(),
            is_unsafe: func.is_unsafe,
            name: mono_name.clone().into(),
            mangled_name: Some(mono_name.into()),
            generics: None,
            params: new_params,
            return_type: new_ret,
            body: new_body,
        };
        let mono_id = FunctionId::from(mono_func);
        self.symbol_tab.add_function(mono_id.clone());
        self.mono_cache.insert(ck, mono_id.clone());
        self.mono_depth -= 1;
        self.mono_in_progress.remove(&ck);
        mono_id
    }

    fn monomorphize_struct(&mut self, sid: &StructDefId, subst: &Substitution) -> StructDefId {
        if self.mono_depth >= MAX_MONO_DEPTH {
            panic!("mono depth limit exceeded");
        }
        let ck = self.struct_mono_cache_key(sid, subst);
        if let Some(existing) = self.struct_mono_cache.get(&ck) {
            return existing.clone();
        }
        if !self.mono_in_progress.insert(ck) {
            return sid.clone();
        }
        self.mono_depth += 1;
        self.mono_counter += 1;
        let sd = sid.borrow();
        let mono_name = format!("{}::<mono-{}>", sd.name, self.mono_counter);
        let mut new_fields = BTreeMap::new();
        let mut new_layout = Vec::new();
        for (fn_, field) in &sd.fields {
            let nt = subst.apply(&field.ty);
            new_fields.insert(
                fn_.clone(),
                nitrate_hir::StructField {
                    span: field.span,
                    visibility: field.visibility,
                    attributes: field.attributes.clone(),
                    name: field.name.clone(),
                    ty: TypeId::from(nt),
                    default_value: field.default_value.clone(),
                },
            );
            new_layout.push(nitrate_hir::StructMemoryLayoutCell::Field {
                field_name: fn_.clone(),
            });
        }
        let mono_struct = StructDef {
            span: ByteSpan::default(),
            visibility: sd.visibility,
            name: mono_name.into(),
            attributes: sd.attributes.clone(),
            fields: new_fields,
            generics: None,
            layout: new_layout.into(),
        };
        let mono_id = StructDefId::from(mono_struct);
        self.symbol_tab.add_struct(mono_id.clone());
        self.struct_mono_cache.insert(ck, mono_id.clone());
        self.mono_depth -= 1;
        self.mono_in_progress.remove(&ck);
        mono_id
    }

    // ── Main entry points ──────────────────────────────────────────

    fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        function.return_type = resolve_type_impl(self, &function.return_type, log);
        for param_id in &function.params {
            let mut p = param_id.borrow_mut();
            p.ty = resolve_type_impl(self, &p.ty, log);
        }
        if let Some(body) = &mut function.body {
            self.function_return_type = Some(function.return_type);
            self.add_all_elements_to_worklist(body);
            loop {
                let pending: Vec<ValueId> = self.worklist.drain().collect();
                if pending.is_empty() {
                    break;
                }
                let prev_ver = self.constraint_version;
                let prev_mono = self.mono_counter;
                for vid in &pending {
                    self.visit(vid);
                }
                if self.constraint_version == prev_ver && self.mono_counter == prev_mono {
                    break;
                }
            }
            self.finalize_inferred_literals(body);
        }
        if let Some(body) = &function.body {
            for element in body {
                if let BlockElement::Local(lv) = element {
                    let mut l = lv.borrow_mut();
                    l.ty = resolve_type_impl(self, &l.ty, log);
                }
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }

    fn finalize_inferred_literals(&mut self, body: &mut [BlockElement]) {
        for element in body.iter_mut() {
            match element {
                BlockElement::Expr(id) => self.finalize_value_recursive(id),
                BlockElement::Local(lv) => {
                    if let Some(init) = &lv.borrow().initializer {
                        self.finalize_value_recursive(init);
                    }
                }
            }
        }
    }

    fn finalize_value_recursive(&mut self, vid: &ValueId) {
        let span = vid.borrow().span();
        if matches!(
            &*vid.borrow(),
            Value::InferredInteger { .. } | Value::InferredFloat { .. }
        ) && self.constraints.contains_key(vid)
        {
            self.visit(vid);
        }
        let info = {
            let v = vid.borrow();
            if let Value::InferredInteger { value, .. } = &*v {
                Some((true, **value, 0.0))
            } else if let Value::InferredFloat { value, .. } = &*v {
                Some((false, 0, value.0))
            } else {
                None
            }
        };
        if let Some((is_int, int_val, float_val)) = info {
            if !self.constraints.get(vid).map_or(false, |cs| {
                cs.iter().any(|c| {
                    let eff = match &*c.type_id() {
                        Type::Refine { base, .. } => *base,
                        _ => c.type_id(),
                    };
                    if is_int {
                        !eff.is_integer_primitive()
                    } else {
                        !eff.is_float_primitive()
                    }
                })
            }) {
                if is_int {
                    let new_val = match i32::try_from(int_val) {
                        Ok(v) => Value::I32 { span, value: v },
                        Err(_) => match i64::try_from(int_val) {
                            Ok(v) => Value::I64 { span, value: v },
                            Err(_) => match u64::try_from(int_val) {
                                Ok(v) => Value::U64 { span, value: v },
                                Err(_) => Value::U128 {
                                    span,
                                    value: Box::new(int_val),
                                },
                            },
                        },
                    };
                    vid.replace(new_val);
                } else {
                    vid.replace(Value::F64 {
                        span,
                        value: OrderedFloat(float_val),
                    });
                }
            }
            return;
        }
        let children: Vec<ValueId> = {
            let v = vid.borrow();
            match &*v {
                Value::Block { block, .. } => {
                    self.finalize_inferred_literals(&mut block.borrow_mut().elements);
                    return;
                }
                Value::StructObject { fields, .. } => fields.iter().map(|(_, v)| v.clone()).collect(),
                Value::EnumVariant { value: v, .. } => vec![v.clone()],
                Value::Binary { left, right, .. } => vec![left.clone(), right.clone()],
                Value::Unary { operand, .. } => vec![operand.clone()],
                Value::IndexAccess { collection, index, .. } => vec![collection.clone(), index.clone()],
                Value::FieldAccess { expr, .. } => vec![expr.clone()],
                Value::Assign { place, value: val, .. } => vec![place.clone(), val.clone()],
                Value::Deref { place, .. } => vec![place.clone()],
                Value::Cast { value: val, .. } => vec![val.clone()],
                Value::Borrow { place, .. } => vec![place.clone()],
                Value::List { elements, .. } => elements.iter().cloned().collect(),
                Value::Tuple { elements, .. } => elements.iter().cloned().collect(),
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
                    vec![condition.clone()]
                }
                Value::While { condition, body, .. } => {
                    self.finalize_inferred_literals(&mut body.borrow_mut().elements);
                    vec![condition.clone()]
                }
                Value::Loop { body, .. } => {
                    self.finalize_inferred_literals(&mut body.borrow_mut().elements);
                    return;
                }
                Value::Return { value: val, .. } => vec![val.clone()],
                Value::Call { callee, args, .. } => {
                    let mut ids = vec![callee.clone()];
                    ids.extend(args.positional.iter().cloned());
                    ids.extend(args.named.iter().map(|(_, v)| v.clone()));
                    ids
                }
                Value::MethodCall { object, args, .. } => {
                    let mut ids = vec![object.clone()];
                    ids.extend(args.positional.iter().cloned());
                    ids.extend(args.named.iter().map(|(_, v)| v.clone()));
                    ids
                }
                _ => return,
            }
        };
        for child in &children {
            self.finalize_value_recursive(child);
        }
    }

    fn solve_global_variable(&mut self, g: &mut GlobalVariable, log: &CompilerLog) -> Result<(), ()> {
        g.ty = resolve_type_impl(self, &g.ty, log);
        loop {
            let prev_ver = self.constraint_version;
            if g.ty.is_inferred() {
                if let Ok(ty) = g.initializer.borrow().determine_type(self.symbol_tab) {
                    g.ty = ty.into();
                }
            } else {
                self.add_constraint(&g.initializer.clone(), TypeConstraint::Equal(g.ty));
            }
            self.visit(&g.initializer);
            if self.constraint_version == prev_ver {
                break;
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }
}

// ── Free functions that are borrowed by Solver via &self ───────────

fn resolve_type_impl(s: &Solver, ty: &TypeId, log: &CompilerLog) -> TypeId {
    let span = ty.span();
    match &*ty.deref() {
        Type::UnresolvedArray { element_type, len, .. } => {
            let mut ev = Evaluator::new(log, s.symbol_tab.arch_ptr_size());
            match ev.evaluate_to_literal(&len.borrow()) {
                Ok(lit) => {
                    let len_u32 = nitrate_hir_type::lit_to_u128(&lit)
                        .and_then(|v| u32::try_from(v).ok())
                        .unwrap_or(0);
                    Type::Array {
                        span,
                        element_type: resolve_type_impl(s, element_type, log),
                        len: len_u32,
                    }
                    .into()
                }
                Err(_) => ty.clone(),
            }
        }
        Type::UnresolvedRefine { base, min, max, .. } => {
            let mut ev = Evaluator::new(log, s.symbol_tab.arch_ptr_size());
            let min_lit = ev
                .evaluate_to_literal(&min.borrow())
                .ok()
                .map(|lit| LiteralId::from(get_storage(|s| s.store_literal(lit))));
            let max_lit = ev
                .evaluate_to_literal(&max.borrow())
                .ok()
                .map(|lit| LiteralId::from(get_storage(|s| s.store_literal(lit))));
            let resolved_base = resolve_type_impl(s, base, log);
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
            let re = resolve_type_impl(s, element_type, log);
            if re.as_usize() != element_type.as_usize() {
                Type::Array {
                    span,
                    element_type: re,
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
            let rb = resolve_type_impl(s, base, log);
            let ra: Vec<TypeId> = args.positional.iter().map(|a| resolve_type_impl(s, a, log)).collect();
            Type::Parameterized {
                span,
                base: rb,
                args: Arguments {
                    positional: ra.into(),
                    named: args.named.clone(),
                },
            }
            .into()
        }
        Type::Tuple { element_types, .. } => {
            let r: Vec<TypeId> = element_types.iter().map(|et| resolve_type_impl(s, et, log)).collect();
            Type::Tuple {
                span,
                element_types: r.into(),
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
            let rt = resolve_type_impl(s, to, log);
            if rt.as_usize() != to.as_usize() {
                Type::Reference {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: rt,
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
            let re = resolve_type_impl(s, element_type, log);
            if re.as_usize() != element_type.as_usize() {
                Type::SliceRef {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: re,
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
            let rt = resolve_type_impl(s, to, log);
            if rt.as_usize() != to.as_usize() {
                Type::Pointer {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: rt,
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
            let re = resolve_type_impl(s, element_type, log);
            if re.as_usize() != element_type.as_usize() {
                Type::SlicePtr {
                    span,
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: re,
                }
                .into()
            } else {
                ty.clone()
            }
        }
        _ => ty.clone(),
    }
}

fn get_effective_bounds_impl(s: &Solver, id: &ValueId) -> Option<Bounds> {
    let own_bounds = {
        let value = id.borrow();
        match &*value {
            Value::LocalVariableSymbol { id, .. } => extract_bounds_from_type(&id.borrow().ty),
            Value::GlobalVariableSymbol { id, .. } => extract_bounds_from_type(&id.borrow().ty),
            Value::ParameterSymbol { id, .. } => extract_bounds_from_type(&id.borrow().ty),
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
    if let Some(constraints) = s.constraints.get(id) {
        let mut eff = own_bounds;
        for c in constraints {
            if let Some(b) = extract_bounds_from_type(&c.type_id()) {
                eff = Some(match eff {
                    Some(cur) => Bounds::new(std::cmp::max(cur.lo, b.lo), std::cmp::min(cur.hi, b.hi)),
                    None => b,
                });
            }
        }
        return eff;
    }
    own_bounds
}

fn classify_value(value: &Value) -> u8 {
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

// ── Public API ─────────────────────────────────────────────────────

pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    ensure_range_structs(m);
    Solver::new(m).solve_function(function, log)
}

pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    ensure_range_structs(m);
    Solver::new(m).solve_global_variable(global, log)
}
