use crate::substitution::{NodeAction, TypeConstraint};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    BinaryOp, BlockElement, BlockId, Function, FunctionId, GlobalVariable, Lit, PtrSize, SymbolTab, Type, TypeId,
    UnaryOp, Value, ValueId, get_storage,
};
use nitrate_hir_get_type::HirGetType;
use ordered_float::OrderedFloat;
use std::collections::{HashMap, HashSet};
use std::ops::Deref;

pub(crate) struct Solver<'m> {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    pub(crate) m: &'m mut SymbolTab,
    errors: HashSet<crate::diagnosis::TypeErr>,
    function_return_type: Option<TypeId>,
    pub(crate) mono_counter: u32,
    pub(crate) mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>,
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
        }
    }

    fn report_out_of_range(&mut self, integer: u128, target_type: TypeId) {
        self.errors
            .insert(crate::diagnosis::TypeErr::IntegerLiteralOutsizeRange {
                value: integer,
                target_type,
            });
    }

    fn lit_to_i128(lit: &Lit) -> Option<i128> {
        match lit {
            Lit::U8(v) => Some(*v as i128),
            Lit::U16(v) => Some(*v as i128),
            Lit::U32(v) => Some(*v as i128),
            Lit::U64(v) => Some(*v as i128),
            Lit::U128(v) => Some(*v as i128),
            Lit::USize32(v) => Some(*v as i128),
            Lit::USize64(v) => Some(*v as i128),
            Lit::I8(v) => Some(*v as i128),
            Lit::I16(v) => Some(*v as i128),
            Lit::I32(v) => Some(*v as i128),
            Lit::I64(v) => Some(*v as i128),
            Lit::I128(v) => Some(*v),
            _ => None,
        }
    }

    fn integer_primitive_bounds(ty: &Type) -> Option<Bounds> {
        match ty {
            Type::U8 => Some((0, 255)),
            Type::U16 => Some((0, 65535)),
            Type::U32 => Some((0, 4294967295)),
            Type::U64 => Some((0, 18446744073709551615)),
            Type::U128 => Some((0, i128::MAX)),
            Type::USize => Some((0, 18446744073709551615)),
            Type::I8 => Some((-128, 127)),
            Type::I16 => Some((-32768, 32767)),
            Type::I32 => Some((-2147483648, 2147483647)),
            Type::I64 => Some((-9223372036854775808, 9223372036854775807)),
            Type::I128 => Some((i128::MIN, i128::MAX)),
            _ => None,
        }
    }

    fn extract_bounds_from_type(ty: &Type) -> Option<Bounds> {
        match ty {
            Type::Refine { min, max, .. } => {
                let min_lit = get_storage(|store| store[min].clone());
                let max_lit = get_storage(|store| store[max].clone());
                match (Self::lit_to_i128(&min_lit), Self::lit_to_i128(&max_lit)) {
                    (Some(min_val), Some(max_val)) => Some((min_val, max_val)),
                    _ => None,
                }
            }
            _ => Self::integer_primitive_bounds(ty),
        }
    }

    fn get_effective_bounds(&self, id: &ValueId) -> Option<Bounds> {
        let own_bounds = {
            let value = id.borrow();
            match &*value {
                Value::LocalVariableSymbol { id } => Self::extract_bounds_from_type(id.borrow().ty.deref()),
                Value::GlobalVariableSymbol { id } => Self::extract_bounds_from_type(id.borrow().ty.deref()),
                Value::ParameterSymbol { id } => Self::extract_bounds_from_type(id.borrow().ty.deref()),
                Value::I8(_) => Some((-128, 127)),
                Value::I16(_) => Some((-32768, 32767)),
                Value::I32(_) => Some((-2147483648, 2147483647)),
                Value::I64(_) => Some((-9223372036854775808, 9223372036854775807)),
                Value::I128(_) => Some((i128::MIN, i128::MAX)),
                Value::U8(_) => Some((0, 255)),
                Value::U16(_) => Some((0, 65535)),
                Value::U32(_) => Some((0, 4294967295)),
                Value::U64(_) => Some((0, 18446744073709551615)),
                Value::U128(_) => Some((0, i128::MAX)),
                Value::USize32(_) => Some((0, 4294967295)),
                Value::USize64(_) => Some((0, 18446744073709551615)),
                Value::InferredInteger(v) => Some((**v as i128, **v as i128)),
                _ => None,
            }
        };

        if let Some(constraints) = self.constraints.get(id) {
            let mut effective_bounds = own_bounds;
            for constraint in constraints {
                let TypeConstraint::Equal(ty) = constraint;
                if let Some(bounds) = Self::extract_bounds_from_type(ty) {
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

    fn compute_binary_bounds(op: &BinaryOp, left: Bounds, right: Bounds) -> Option<Bounds> {
        let (l_min, l_max) = left;
        let (r_min, r_max) = right;
        match op {
            BinaryOp::Add => Some((l_min.saturating_add(r_min), l_max.saturating_add(r_max))),
            BinaryOp::Sub => Some((l_min.saturating_sub(r_max), l_max.saturating_sub(r_min))),
            BinaryOp::Mul => {
                let products = [
                    l_min.saturating_mul(r_min),
                    l_min.saturating_mul(r_max),
                    l_max.saturating_mul(r_min),
                    l_max.saturating_mul(r_max),
                ];
                Some((*products.iter().min().unwrap(), *products.iter().max().unwrap()))
            }
            BinaryOp::Div => {
                let candidates = if r_min <= 0 && r_max >= 0 {
                    vec![
                        if r_min != 0 {
                            l_min.saturating_div(r_min)
                        } else {
                            i128::MAX
                        },
                        if r_max != 0 {
                            l_min.saturating_div(r_max)
                        } else {
                            i128::MAX
                        },
                        if r_min != 0 {
                            l_max.saturating_div(r_min)
                        } else {
                            i128::MIN
                        },
                        if r_max != 0 {
                            l_max.saturating_div(r_max)
                        } else {
                            i128::MIN
                        },
                    ]
                } else {
                    vec![
                        l_min.saturating_div(r_min),
                        l_min.saturating_div(r_max),
                        l_max.saturating_div(r_min),
                        l_max.saturating_div(r_max),
                    ]
                };
                Some((*candidates.iter().min().unwrap(), *candidates.iter().max().unwrap()))
            }
            BinaryOp::Mod => {
                if r_min <= 0 && r_max >= 0 {
                    Some((i128::MIN, i128::MAX))
                } else {
                    let a = std::cmp::max(r_min.abs(), r_max.abs());
                    Some((0, a - 1))
                }
            }
            BinaryOp::And => {
                if l_min >= 0 && r_min >= 0 {
                    Some((0, std::cmp::min(l_max, r_max)))
                } else {
                    Some((std::cmp::min(l_min, r_min), std::cmp::max(l_max, r_max)))
                }
            }
            BinaryOp::Or => Some((std::cmp::min(l_min, r_min), std::cmp::max(l_max, r_max))),
            BinaryOp::Xor => Some((std::cmp::min(l_min, r_min), std::cmp::max(l_max, r_max))),
            BinaryOp::Shl | BinaryOp::Rol => Some((i128::MIN, i128::MAX)),
            BinaryOp::Shr | BinaryOp::Ror => {
                if l_min >= 0 && r_min >= 0 {
                    let smin = if r_max > 0 {
                        l_min.checked_shr(r_max as u32).unwrap_or(l_min)
                    } else {
                        l_min
                    };
                    let smax = if r_min > 0 {
                        l_max.checked_shr(r_min as u32).unwrap_or(l_max)
                    } else {
                        l_max
                    };
                    Some((smin, smax))
                } else {
                    Some((i128::MIN, i128::MAX))
                }
            }
            BinaryOp::Lt
            | BinaryOp::Gt
            | BinaryOp::Lte
            | BinaryOp::Gte
            | BinaryOp::Eq
            | BinaryOp::Ne
            | BinaryOp::LogicAnd
            | BinaryOp::LogicOr => None,
        }
    }

    fn compute_unary_bounds(op: &UnaryOp, operand: Bounds) -> Bounds {
        let (min, max) = operand;
        match op {
            UnaryOp::Add => (min, max),
            UnaryOp::Sub => (max.saturating_neg(), min.saturating_neg()),
            UnaryOp::Not => (!max, !min),
        }
    }

    fn check_bounds_against_constraint(&mut self, computed_bounds: Bounds, constraint_ty: &Type) -> bool {
        let target_bounds = Self::extract_bounds_from_type(constraint_ty);
        if let Some((target_min, target_max)) = target_bounds {
            let (comp_min, comp_max) = computed_bounds;
            if comp_min < target_min || comp_max > target_max {
                if matches!(constraint_ty, Type::Refine { .. }) {
                    self.errors
                        .insert(crate::diagnosis::TypeErr::OperationResultOutOfRefinementBounds {
                            refinement_type: TypeId::from(constraint_ty.clone()),
                            computed_min: comp_min.max(0) as u128,
                            computed_max: comp_max.max(0) as u128,
                        });
                    return false;
                }
            }
        }
        true
    }

    fn check_refinement_bounds(&mut self, value: u128, constraint_ty: &Type) -> bool {
        match constraint_ty {
            Type::Refine { min, max, .. } => {
                let min_lit: Lit = get_storage(|store| store[min].clone());
                let max_lit: Lit = get_storage(|store| store[max].clone());
                let min_val = Self::lit_to_u128_check(&min_lit);
                let max_val = Self::lit_to_u128_check(&max_lit);
                match (min_val, max_val) {
                    (Some(_mn), Some(_mx)) if value >= _mn && value <= _mx => true,
                    (Some(_), Some(_)) => {
                        self.errors
                            .insert(crate::diagnosis::TypeErr::IntegerLiteralOutOfRefinementBounds {
                                value,
                                refinement_type: TypeId::from(constraint_ty.clone()),
                            });
                        false
                    }
                    _ => true,
                }
            }
            _ => true,
        }
    }

    fn lit_to_u128_check(lit: &Lit) -> Option<u128> {
        match lit {
            Lit::U8(v) => Some(*v as u128),
            Lit::U16(v) => Some(*v as u128),
            Lit::U32(v) => Some(*v as u128),
            Lit::U64(v) => Some(*v as u128),
            Lit::U128(v) => Some(*v),
            Lit::USize32(v) => Some(*v as u128),
            Lit::USize64(v) => Some(*v as u128),
            Lit::I8(v) if *v >= 0 => Some(*v as u128),
            Lit::I16(v) if *v >= 0 => Some(*v as u128),
            Lit::I32(v) if *v >= 0 => Some(*v as u128),
            Lit::I64(v) if *v >= 0 => Some(*v as u128),
            Lit::I128(v) if *v >= 0 => Some(*v as u128),
            _ => None,
        }
    }

    fn solve_inferred_integer(&mut self, id: &ValueId, value: u128) -> NodeAction {
        let constraints: Vec<TypeConstraint> = self
            .constraints
            .get(id)
            .cloned()
            .unwrap_or_default()
            .into_iter()
            .collect();
        for constraint in &constraints {
            let TypeConstraint::Equal(ty) = constraint;
            if let Type::Refine { .. } = &**ty {
                self.check_refinement_bounds(value, ty);
            }
            let effective_ty = match &**ty {
                Type::Refine { base, .. } => &*base.deref(),
                _ => ty.deref(),
            };
            if !effective_ty.is_integer_primitive() {
                self.errors
                    .insert(crate::diagnosis::TypeErr::IntegerLiteralUnsatisfiable {
                        value,
                        unsatisfiable_type: ty.clone(),
                    });
                break;
            }
            return match **ty {
                Type::I8 => match i8::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::I8(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::I16 => match i16::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::I16(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::I32 => match i32::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::I32(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::I64 => match i64::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::I64(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::I128 => match i128::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::I128(Box::new(v))),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::U8 => match u8::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::U8(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::U16 => match u16::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::U16(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::U32 => match u32::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::U32(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::U64 => match u64::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::U64(v)),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::U128 => match u128::try_from(value) {
                    Ok(v) => NodeAction::Replace(Value::U128(Box::new(v))),
                    Err(_) => {
                        self.report_out_of_range(value, ty.clone());
                        NodeAction::NoChange
                    }
                },
                Type::USize => match self.m.arch_ptr_size() {
                    PtrSize::U32 => match u32::try_from(value) {
                        Ok(v) => NodeAction::Replace(Value::USize32(v)),
                        Err(_) => {
                            self.report_out_of_range(value, ty.clone());
                            NodeAction::NoChange
                        }
                    },
                    PtrSize::U64 => match u64::try_from(value) {
                        Ok(v) => NodeAction::Replace(Value::USize64(v)),
                        Err(_) => {
                            self.report_out_of_range(value, ty.clone());
                            NodeAction::NoChange
                        }
                    },
                },
                Type::Refine { .. } => return NodeAction::NoChange,
                _ => unreachable!(),
            };
        }
        NodeAction::NoChange
    }

    fn solve_inferred_float(&mut self, id: &ValueId, value: OrderedFloat<f64>) -> NodeAction {
        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                let TypeConstraint::Equal(ty) = constraint;
                if !ty.is_float_primitive() {
                    self.errors
                        .insert(crate::diagnosis::TypeErr::FloatLiteralUnsatisfiable {
                            value,
                            unsatisfiable_type: ty.clone(),
                        });
                    break;
                }
                return match **ty {
                    Type::F32 => NodeAction::Replace(Value::F32((*value as f32).into())),
                    Type::F64 => NodeAction::Replace(Value::F64(value)),
                    _ => unreachable!(),
                };
            }
        }
        NodeAction::NoChange
    }

    fn determine_action(&mut self, value: &Value, id: &ValueId) -> NodeAction {
        match value {
            Value::Unit
            | Value::Bool(_)
            | Value::I8(_)
            | Value::I16(_)
            | Value::I32(_)
            | Value::I64(_)
            | Value::I128(_)
            | Value::U8(_)
            | Value::U16(_)
            | Value::U32(_)
            | Value::U64(_)
            | Value::U128(_)
            | Value::F32(_)
            | Value::F64(_)
            | Value::USize32(_)
            | Value::USize64(_)
            | Value::StringLit(_)
            | Value::BStringLit(_)
            | Value::StructObject { .. }
            | Value::EnumVariant { .. }
            | Value::Binary { .. }
            | Value::Unary { .. }
            | Value::IndexAccess { .. }
            | Value::FieldAccess { .. }
            | Value::Assign { .. }
            | Value::Deref { .. }
            | Value::Cast { .. }
            | Value::Borrow { .. }
            | Value::List { .. }
            | Value::Tuple { .. }
            | Value::If { .. }
            | Value::While { .. }
            | Value::Loop { .. }
            | Value::Break { .. }
            | Value::Continue { .. }
            | Value::Return { .. }
            | Value::Block { .. }
            | Value::Call { .. }
            | Value::MethodCall { .. }
            | Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => NodeAction::NoChange,
            Value::InferredInteger(integer) => self.solve_inferred_integer(id, **integer),
            Value::InferredFloat(float) => self.solve_inferred_float(id, *float),
        }
    }

    fn visit_children(&mut self, e: &ValueId) {
        let value = e.borrow().clone();
        match &value {
            Value::Unit
            | Value::Bool(_)
            | Value::I8(_)
            | Value::I16(_)
            | Value::I32(_)
            | Value::I64(_)
            | Value::I128(_)
            | Value::U8(_)
            | Value::U16(_)
            | Value::U32(_)
            | Value::U64(_)
            | Value::U128(_)
            | Value::F32(_)
            | Value::F64(_)
            | Value::USize32(_)
            | Value::USize64(_)
            | Value::StringLit(_)
            | Value::BStringLit(_)
            | Value::InferredInteger(_)
            | Value::InferredFloat(_) => {}

            Value::StructObject { struct_def, fields } => {
                for (field_name, field_value) in fields {
                    let struct_def_b = struct_def.borrow();
                    if let Some(field) = struct_def_b.fields.get(field_name) {
                        let field_type = field.ty;
                        self.constraints
                            .entry(field_value.clone())
                            .or_default()
                            .insert(TypeConstraint::Equal(field_type));
                        self.visit(field_value);
                    }
                }
            }

            Value::EnumVariant {
                enum_def,
                variant,
                value: inner_value,
            } => {
                let variant_type = enum_def
                    .borrow()
                    .variants
                    .iter()
                    .find(|item| item.name == *variant)
                    .expect("variant not present")
                    .ty;
                self.constraints
                    .entry(inner_value.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(variant_type));
                self.visit(inner_value);
            }

            Value::Binary { left, op, right } => {
                // Check refinement bounds FIRST, before constraint propagation contaminates operands
                let constraints_copy = self.constraints.get(e).cloned().unwrap_or_default();
                for c in &constraints_copy {
                    let TypeConstraint::Equal(result_ty) = c;
                    if matches!(
                        op,
                        BinaryOp::Add
                            | BinaryOp::Sub
                            | BinaryOp::Mul
                            | BinaryOp::Div
                            | BinaryOp::Mod
                            | BinaryOp::And
                            | BinaryOp::Or
                            | BinaryOp::Xor
                            | BinaryOp::Shl
                            | BinaryOp::Shr
                            | BinaryOp::Rol
                            | BinaryOp::Ror
                    ) {
                        if let (Some(lb), Some(rb)) =
                            (self.get_effective_bounds(left), self.get_effective_bounds(right))
                        {
                            if let Some(res) = Self::compute_binary_bounds(op, lb, rb) {
                                self.check_bounds_against_constraint(res, result_ty);
                            }
                        }
                    }
                }
                // Now propagate base type constraints to children
                if let Some(parent_constraints) = self.constraints.get(e).cloned() {
                    let child_constraints: HashSet<TypeConstraint> = parent_constraints
                        .iter()
                        .map(|c| match c {
                            TypeConstraint::Equal(ty) => match &**ty {
                                Type::Refine { base, .. } => TypeConstraint::Equal(*base),
                                _ => c.clone(),
                            },
                        })
                        .collect();
                    self.constraints
                        .entry(left.clone())
                        .or_default()
                        .extend(child_constraints.clone());
                    self.constraints
                        .entry(right.clone())
                        .or_default()
                        .extend(child_constraints);
                }
                self.visit(left);
                self.visit(right);
            }

            Value::Unary { op, operand } => {
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    self.constraints.entry(operand.clone()).or_default().extend(constraints);
                }
                self.visit(operand);
                let constraints_copy = self.constraints.get(e).cloned().unwrap_or_default();
                for c in &constraints_copy {
                    let TypeConstraint::Equal(result_ty) = c;
                    if let Some(ob) = self.get_effective_bounds(operand) {
                        let res = Self::compute_unary_bounds(op, ob);
                        self.check_bounds_against_constraint(res, result_ty);
                    }
                }
            }

            Value::IndexAccess { collection, index } => {
                self.constraints
                    .entry(index.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(Type::USize.into()));
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    if let Ok(collection_type) = collection.borrow().determine_type(self.m) {
                        let element_type_id = match &collection_type {
                            Type::Array { element_type, .. }
                            | Type::SliceRef { element_type, .. }
                            | Type::SlicePtr { element_type, .. } => Some(*element_type),
                            _ => None,
                        };
                        if let Some(element_type_id) = element_type_id {
                            self.constraints
                                .entry(e.clone())
                                .or_default()
                                .insert(TypeConstraint::Equal(element_type_id));
                            if let Value::List { elements } = &*collection.borrow() {
                                for element in elements {
                                    self.constraints
                                        .entry(element.clone())
                                        .or_default()
                                        .extend(constraints.clone());
                                }
                            }
                        }
                    }
                }
                self.visit(collection);
                self.visit(index);
            }

            Value::FieldAccess { expr, .. } => {
                self.visit(expr);
            }
            Value::Assign { place, value: v } => {
                if let Ok(place_type) = place.borrow().determine_type(self.m) {
                    self.constraints
                        .entry(v.clone())
                        .or_default()
                        .insert(TypeConstraint::Equal(place_type.into()));
                }
                self.visit(place);
                self.visit(v);
            }
            Value::Deref { place } => self.visit(place),
            Value::Cast { value: v, target_type } => {
                self.constraints
                    .entry(v.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(target_type.clone()));
                self.visit(v);
            }
            Value::Borrow { place, .. } => self.visit(place),

            Value::List { elements } => {
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    for element in elements {
                        let ec: HashSet<TypeConstraint> = constraints
                            .iter()
                            .map(|c| match c {
                                TypeConstraint::Equal(ty) => match &**ty {
                                    Type::Array { element_type, .. }
                                    | Type::SliceRef { element_type, .. }
                                    | Type::SlicePtr { element_type, .. } => TypeConstraint::Equal(*element_type),
                                    _ => c.clone(),
                                },
                            })
                            .collect();
                        self.constraints.entry(element.clone()).or_default().extend(ec);
                    }
                }
                let concrete_element = elements
                    .iter()
                    .find(|el| !matches!(&*el.borrow(), Value::InferredInteger(_) | Value::InferredFloat(_)));
                if let Some(concrete_element) = concrete_element {
                    if let Some(concrete_type_id) = concrete_element
                        .borrow()
                        .determine_type(self.m)
                        .ok()
                        .map(|ty| TypeId::from(ty))
                    {
                        for element in elements.iter() {
                            if matches!(&*element.borrow(), Value::InferredInteger(_) | Value::InferredFloat(_)) {
                                self.constraints
                                    .entry(element.clone())
                                    .or_default()
                                    .insert(TypeConstraint::Equal(concrete_type_id));
                            }
                        }
                    }
                }
                for element in elements {
                    self.visit(element);
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    self.visit(element);
                }
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                self.constraints
                    .entry(condition.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(Type::Bool.into()));
                self.visit(condition);
                self.visit_block(true_branch);
                if let Some(false_branch) = false_branch {
                    self.visit_block(false_branch);
                }
            }

            Value::While { condition, body } => {
                self.constraints
                    .entry(condition.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(Type::Bool.into()));
                self.visit(condition);
                self.visit_block(body);
            }

            Value::Loop { body } => self.visit_block(body),
            Value::Break { .. } | Value::Continue { .. } => {}

            Value::Return { value: v } => {
                if let Some(ret_type) = self.function_return_type {
                    self.constraints
                        .entry(v.clone())
                        .or_default()
                        .insert(TypeConstraint::Equal(ret_type));
                }
                self.visit(v);
            }

            Value::Block { block } => {
                for element in &mut block.borrow_mut().elements {
                    self.visit_block_element(element);
                }
            }

            Value::Call { callee, args } => {
                let callee_func_id: Option<FunctionId> = match &*callee.borrow() {
                    Value::FunctionSymbol { id } => {
                        let func = id.borrow();
                        if func.generics.is_some() && func.generics.as_ref().map_or(false, |g| !g.is_empty()) {
                            Some(id.clone())
                        } else {
                            None
                        }
                    }
                    _ => None,
                };
                if let Some(func_id) = callee_func_id {
                    if let Some(subst) = self.infer_generic_args_from_call(&func_id, &args.positional) {
                        let mono_id = self.monomorphize_function(&func_id, &subst);
                        callee.replace(Value::FunctionSymbol { id: mono_id });
                    }
                }
                self.visit(callee);
                if let Value::FunctionSymbol { id } = &*callee.borrow() {
                    let func = id.borrow();
                    for (i, arg) in args.positional.iter().enumerate() {
                        if let Some(param) = func.params.get(i) {
                            let param_type = param.borrow().ty;
                            self.constraints
                                .entry(arg.clone())
                                .or_default()
                                .insert(TypeConstraint::Equal(param_type));
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

            Value::MethodCall {
                object,
                method_name,
                args,
            } => {
                let obj_type: Option<TypeId> = object.borrow().determine_type(self.m).ok().map(|ty| ty.into());
                let method_id_opt: Option<FunctionId> =
                    obj_type.and_then(|obj_type| self.m.get_method(&obj_type, method_name).cloned());
                if let Some(method_id) = method_id_opt {
                    let is_generic = {
                        let mf = method_id.borrow();
                        mf.generics.is_some() && mf.generics.as_ref().map_or(false, |g| !g.is_empty())
                    };
                    if is_generic {
                        if let Some(subst) = self.infer_generic_args_from_call(&method_id, &args.positional) {
                            let mono_id = self.monomorphize_function(&method_id, &subst);
                            let new_call = Value::Call {
                                callee: ValueId::from(Value::FunctionSymbol { id: mono_id }),
                                args: args.clone(),
                            };
                            e.replace(new_call);
                            self.visit(e);
                            return;
                        }
                    }
                }
                self.visit(object);
                for arg in &args.positional {
                    self.visit(arg);
                }
                for (_name, arg) in &args.named {
                    self.visit(arg);
                }
            }

            Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => {}
        }
    }

    fn visit(&mut self, e: &ValueId) {
        let action = {
            let current_value = e.borrow();
            self.determine_action(&*current_value, e)
        };
        match action {
            NodeAction::Replace(new_value) => {
                e.replace(new_value);
            }
            NodeAction::NoChange => self.visit_children(e),
        }
    }

    fn visit_block(&mut self, block: &BlockId) {
        for element in &mut block.borrow_mut().elements {
            self.visit_block_element(element);
        }
    }

    fn visit_block_element(&mut self, element: &mut BlockElement) {
        match element {
            BlockElement::Expr(e) => self.visit(e),
            BlockElement::Local(local_var) => {
                if local_var.borrow().ty.is_inferred() {
                    if let Ok(ty) = local_var.borrow().initializer.borrow().determine_type(self.m) {
                        local_var.borrow_mut().ty = ty.into();
                    }
                } else {
                    let value = local_var.borrow().initializer.clone();
                    let ty = local_var.borrow().ty.clone();
                    self.constraints
                        .entry(value)
                        .or_default()
                        .insert(TypeConstraint::Equal(ty));
                }
                self.visit(&local_var.borrow().initializer);
            }
        }
    }

    fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        if let Some(body) = &mut function.body {
            self.function_return_type = Some(function.return_type);
            loop {
                let prev_len = self.constraints.len();
                for element in body.iter_mut() {
                    self.visit_block_element(element);
                }
                if self.constraints.len() == prev_len {
                    break;
                }
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }

    fn solve_global_variable(&mut self, g: &mut GlobalVariable, log: &CompilerLog) -> Result<(), ()> {
        loop {
            let prev_len = self.constraints.len();
            if g.ty.is_inferred() {
                if let Ok(ty) = g.initializer.borrow().determine_type(self.m) {
                    g.ty = ty.into();
                }
            } else {
                let value = g.initializer.clone();
                let ty = g.ty.clone();
                self.constraints
                    .entry(value)
                    .or_default()
                    .insert(TypeConstraint::Equal(ty));
            }
            self.visit(&mut g.initializer);
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

pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_function(function, log)
}

pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_global_variable(global, log)
}
