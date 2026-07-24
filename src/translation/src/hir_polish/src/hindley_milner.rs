use crate::diagnosis::TypeErr;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    Arguments, BlockElement, BlockId, Function, FunctionId, FunctionType, GlobalVariable, LocalVariable,
    LocalVariableId, Parameter, ParameterId, PtrSize, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use ordered_float::OrderedFloat;
use std::collections::{HashMap, HashSet};
use thin_vec::ThinVec;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
enum TypeConstraint {
    Equal(TypeId),
}

enum NodeAction {
    NoChange,
    Replace(Value),
}

/// A substitution maps generic parameter indices (or inference variable IDs) to concrete types.
#[derive(Debug, Clone, Default)]
struct Substitution {
    /// Maps inference variable IDs or generic param indices to concrete types
    mapping: HashMap<u32, TypeId>,
}

impl Substitution {
    fn apply(&self, ty: &Type) -> Type {
        match ty {
            Type::GenericParam { index, .. } => {
                if let Some(concrete) = self.mapping.get(index) {
                    (&**concrete).clone()
                } else {
                    ty.clone()
                }
            }
            Type::Inferred { id, .. } => {
                if let Some(concrete) = self.mapping.get(&id.get()) {
                    (&**concrete).clone()
                } else {
                    ty.clone()
                }
            }
            Type::Struct { def } => {
                // Check if this is a monomorphized struct; return as-is
                ty.clone()
            }
            Type::Parameterized { base, .. } => {
                // Resolve parameterized type by applying substitution to base
                self.apply(base)
            }
            Type::Array { element_type, len } => {
                let new_elem = self.apply(element_type);
                Type::Array {
                    element_type: TypeId::from(new_elem),
                    len: *len,
                }
            }
            Type::Tuple { element_types } => {
                let new_elements: Vec<TypeId> = element_types.iter().map(|et| TypeId::from(self.apply(et))).collect();
                Type::Tuple {
                    element_types: new_elements.into(),
                }
            }
            Type::Function { function_type } => {
                let new_params: Vec<(NString, TypeId)> = function_type
                    .params
                    .iter()
                    .map(|(n, p)| (n.clone(), TypeId::from(self.apply(p))))
                    .collect();
                let new_ret = self.apply(&function_type.return_type);
                Type::Function {
                    function_type: Box::new(FunctionType {
                        attributes: function_type.attributes.clone(),
                        params: new_params.into(),
                        return_type: TypeId::from(new_ret),
                    }),
                }
            }
            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => {
                let new_to = self.apply(to);
                Type::Reference {
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: TypeId::from(new_to),
                }
            }
            Type::Pointer { exclusive, mutable, to } => {
                let new_to = self.apply(to);
                Type::Pointer {
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: TypeId::from(new_to),
                }
            }
            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
            } => {
                let new_elem = self.apply(element_type);
                Type::SliceRef {
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: TypeId::from(new_elem),
                }
            }
            Type::SlicePtr {
                exclusive,
                mutable,
                element_type,
            } => {
                let new_elem = self.apply(element_type);
                Type::SlicePtr {
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: TypeId::from(new_elem),
                }
            }
            Type::Refine { base, min, max } => {
                let new_base = self.apply(base);
                Type::Refine {
                    base: TypeId::from(new_base),
                    min: *min,
                    max: *max,
                }
            }
            Type::TypeAlias { def } => {
                let type_alias = def.borrow();
                self.apply(&type_alias.type_id)
            }
            _ => ty.clone(),
        }
    }
}

struct HindleyMilner<'m> {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    m: &'m mut SymbolTab,
    errors: HashSet<TypeErr>,
    function_return_type: Option<TypeId>,
    mono_counter: u32,
}

impl<'m> HindleyMilner<'m> {
    fn new(m: &'m mut SymbolTab) -> Self {
        Self {
            constraints: HashMap::new(),
            m,
            errors: HashSet::new(),
            function_return_type: None,
            mono_counter: 0,
        }
    }

    fn report_out_of_range(&mut self, integer: u128, target_type: TypeId) {
        self.errors.insert(TypeErr::IntegerLiteralOutsizeRange {
            value: integer,
            target_type,
        });
    }

    fn solve_inferred_integer(&mut self, id: &ValueId, value: u128) -> NodeAction {
        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                match constraint {
                    TypeConstraint::Equal(ty) => {
                        if !ty.is_integer_primitive() {
                            self.errors.insert(TypeErr::IntegerLiteralUnsatisfiable {
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
                            _ => unreachable!(),
                        };
                    }
                }
            }
        }
        NodeAction::NoChange
    }

    fn solve_inferred_float(&mut self, id: &ValueId, value: OrderedFloat<f64>) -> NodeAction {
        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                match constraint {
                    TypeConstraint::Equal(ty) => {
                        if !ty.is_float_primitive() {
                            self.errors.insert(TypeErr::FloatLiteralUnsatisfiable {
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

    /// Infer concrete types for generic parameters from argument types at a call site.
    fn infer_generic_args_from_call(
        &self,
        callee_func_id: &FunctionId,
        positional_args: &[ValueId],
    ) -> Option<Substitution> {
        let callee_func = callee_func_id.borrow();
        let generics = callee_func.generics.as_ref()?;

        if generics.is_empty() {
            return Some(Substitution::default());
        }

        let mut subst = Substitution::default();
        let param_types: Vec<TypeId> = callee_func.params.iter().map(|p| p.borrow().ty).collect();

        for (arg_value_id, param_type_id) in positional_args.iter().zip(param_types.iter()) {
            let arg_type = arg_value_id.borrow().determine_type(self.m).ok()?;
            let param_type = &*param_type_id;
            Self::unify_types_with_subst(&arg_type, param_type, &mut subst);
        }

        Some(subst)
    }

    fn unify_types_with_subst(arg_type: &Type, param_type: &Type, subst: &mut Substitution) {
        match (arg_type, param_type) {
            (concrete, Type::GenericParam { index, .. }) => {
                subst
                    .mapping
                    .entry(*index)
                    .or_insert_with(|| TypeId::from(concrete.clone()));
            }
            (concrete, Type::Inferred { id, .. }) => {
                subst
                    .mapping
                    .entry(id.get())
                    .or_insert_with(|| TypeId::from(concrete.clone()));
            }
            (Type::Reference { to: a_to, .. }, Type::Reference { to: p_to, .. }) => {
                Self::unify_types_with_subst(a_to, p_to, subst);
            }
            (Type::Pointer { to: a_to, .. }, Type::Pointer { to: p_to, .. }) => {
                Self::unify_types_with_subst(a_to, p_to, subst);
            }
            (Type::Array { element_type: a_e, .. }, Type::Array { element_type: p_e, .. }) => {
                Self::unify_types_with_subst(a_e, p_e, subst);
            }
            (Type::Tuple { element_types: a_ets }, Type::Tuple { element_types: p_ets }) => {
                for (a_et, p_et) in a_ets.iter().zip(p_ets.iter()) {
                    Self::unify_types_with_subst(a_et, p_et, subst);
                }
            }
            (Type::Function { function_type: a_ft }, Type::Function { function_type: p_ft }) => {
                Self::unify_types_with_subst(&a_ft.return_type, &p_ft.return_type, subst);
                for ((_, a_p), (_, p_p)) in a_ft.params.iter().zip(p_ft.params.iter()) {
                    Self::unify_types_with_subst(a_p, p_p, subst);
                }
            }
            (Type::GenericParam { index, .. }, concrete) => {
                subst
                    .mapping
                    .entry(*index)
                    .or_insert_with(|| TypeId::from(concrete.clone()));
            }
            _ => {}
        }
    }

    fn monomorphize_function(&mut self, func_id: &FunctionId, subst: &Substitution) -> FunctionId {
        let func = func_id.borrow();

        self.mono_counter += 1;
        let mono_name = format!("{}::<mono-{}>", func.name, self.mono_counter);
        let mono_name_ns: NString = mono_name.clone().into();
        let mono_mangled_name: NString = mono_name.into();

        let new_params: Vec<ParameterId> = func
            .params
            .iter()
            .map(|param_id| {
                let param = param_id.borrow();
                let new_ty = subst.apply(&param.ty);
                let new_param = Parameter {
                    attributes: param.attributes.clone(),
                    is_mutable: param.is_mutable,
                    name: param.name.clone(),
                    ty: TypeId::from(new_ty),
                    default_value: param.default_value.clone(),
                };
                ParameterId::from(new_param)
            })
            .collect();

        let new_return_type = TypeId::from(subst.apply(&func.return_type));

        let new_body = func.body.as_ref().map(|body| {
            body.iter()
                .map(|element| self.clone_block_element(element, subst))
                .collect()
        });

        let mono_func = Function {
            visibility: func.visibility,
            attributes: func.attributes.clone(),
            name: mono_name_ns,
            mangled_name: mono_mangled_name,
            generics: None,
            params: new_params,
            return_type: new_return_type,
            body: new_body,
        };

        let mono_id: FunctionId = mono_func.into();
        // Register the monomorphized function in the symbol table so the LLVM codegen can find it
        self.m.add_function(mono_id.clone());
        mono_id
    }

    fn clone_block_element(&self, element: &BlockElement, subst: &Substitution) -> BlockElement {
        match element {
            BlockElement::Expr(expr_id) => {
                let value = expr_id.borrow();
                let new_value = self.apply_subst_to_value(&value, subst);
                BlockElement::Expr(ValueId::from(new_value))
            }
            BlockElement::Local(local_id) => {
                let local = local_id.borrow();
                let new_ty = subst.apply(&local.ty);
                let new_init_val = local.initializer.borrow();
                let new_init = self.apply_subst_to_value(&new_init_val, subst);
                let new_local = LocalVariable {
                    kind: local.kind.clone(),
                    attributes: local.attributes.clone(),
                    is_mutable: local.is_mutable,
                    name: local.name.clone(),
                    ty: TypeId::from(new_ty),
                    initializer: ValueId::from(new_init),
                };
                BlockElement::Local(LocalVariableId::from(new_local))
            }
        }
    }

    fn apply_subst_to_value(&self, value: &Value, subst: &Substitution) -> Value {
        match value {
            Value::Cast { value: v, target_type } => {
                let new_target = subst.apply(target_type);
                Value::Cast {
                    value: v.clone(),
                    target_type: TypeId::from(new_target),
                }
            }
            Value::StructObject { struct_def, fields } => {
                // For generic structs, we need to monomorphize the struct def
                let struct_def_b = struct_def.borrow();
                if struct_def_b.generics.is_some() {
                    // Update field types using substitution
                    let new_fields: ThinVec<(NString, ValueId)> = fields
                        .iter()
                        .map(|(name, val_id)| (name.clone(), val_id.clone()))
                        .collect();
                    Value::StructObject {
                        struct_def: struct_def.clone(),
                        fields: new_fields,
                    }
                } else {
                    Value::StructObject {
                        struct_def: struct_def.clone(),
                        fields: fields.clone(),
                    }
                }
            }
            Value::Call { callee, args } => {
                let new_args = Arguments {
                    positional: args.positional.clone(),
                    named: args.named.clone(),
                };
                Value::Call {
                    callee: callee.clone(),
                    args: new_args,
                }
            }
            Value::FunctionSymbol { id } => Value::FunctionSymbol { id: id.clone() },
            // For all other values, just clone
            val => val.clone(),
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

            Value::Binary { left, op: _, right } => {
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    self.constraints
                        .entry(left.clone())
                        .or_default()
                        .extend(constraints.clone());
                    self.constraints.entry(right.clone()).or_default().extend(constraints);
                }
                self.visit(left);
                self.visit(right);
            }

            Value::Unary { operand, .. } => {
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    self.constraints.entry(operand.clone()).or_default().extend(constraints);
                }
                self.visit(operand);
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
                // Check if callee is a generic function - monomorphize if so
                // We need to drop the borrow before mutating, so clone what we need
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
                for arg in &args.positional {
                    self.visit(arg);
                }
                for (_name, arg) in &args.named {
                    self.visit(arg);
                }
            }

            Value::MethodCall { object, args, .. } => {
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
                    let init_ty = local_var.borrow().initializer.borrow().determine_type(self.m);
                    if let Ok(ty) = init_ty {
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
                let init_ty = g.initializer.borrow().determine_type(self.m);
                if let Ok(ty) = init_ty {
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
    let mut hm = HindleyMilner::new(m);
    hm.solve_function(function, log)
}

pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = HindleyMilner::new(m);
    hm.solve_global_variable(global, log)
}
