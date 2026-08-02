use crate::solver::{MAX_MONO_DEPTH, MonoCacheKey, Solver};
use crate::substitution::Substitution;
use nitrate_hir::{
    Arguments, BlockElement, Function, FunctionId, LocalVariable, LocalVariableId, Parameter, ParameterId, StructDef,
    StructDefId, StructField, StructMemoryLayoutCell, Type, TypeId, Value, ValueId,
};
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::BTreeMap;
use thin_vec::ThinVec;

pub(crate) type StructMonoCacheValue = StructDefId;

impl<'m> Solver<'m> {
    pub(crate) fn mono_cache_key(&self, func_id: &FunctionId, subst: &Substitution) -> MonoCacheKey {
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        MonoCacheKey::new(func_id.as_usize(), &sorted_args)
    }

    pub(crate) fn struct_mono_cache_key(&self, struct_id: &StructDefId, subst: &Substitution) -> MonoCacheKey {
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        MonoCacheKey::new(struct_id.as_usize(), &sorted_args)
    }

    pub(crate) fn infer_generic_args_from_call(
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

        if param_types.len() != positional_args.len() {
            return None;
        }

        for (arg_value_id, param_type_id) in positional_args.iter().zip(param_types.iter()) {
            let arg_type = arg_value_id.borrow().determine_type(self.m).ok()?;
            let param_type = param_type_id;
            Self::unify_types_with_subst(&arg_type, param_type, &mut subst);
        }

        if subst.mapping.is_empty() {
            return None;
        }

        Some(subst)
    }

    pub(crate) fn infer_generic_args_from_call_named(
        &self,
        callee_func_id: &FunctionId,
        args: &Arguments<ValueId>,
    ) -> Option<Substitution> {
        let callee_func = callee_func_id.borrow();
        let generics = callee_func.generics.as_ref()?;

        if generics.is_empty() {
            return Some(Substitution::default());
        }

        let mut subst = Substitution::default();
        let mut any_concrete_type_found = false;

        for (arg_name, arg_value_id) in &args.named {
            if let Some(param_id) = callee_func.params.iter().find(|p| p.borrow().name == *arg_name) {
                let param_type = param_id.borrow().ty;
                if let Ok(arg_type) = arg_value_id.borrow().determine_type(self.m) {
                    if arg_type.is_inferred() {
                        continue;
                    }
                    any_concrete_type_found = true;
                    Self::unify_types_with_subst(&arg_type, &param_type, &mut subst);
                }
            }
        }

        for (i, arg_value_id) in args.positional.iter().enumerate() {
            if let Some(param_id) = callee_func.params.get(i) {
                let param_type = param_id.borrow().ty;
                if let Ok(arg_type) = arg_value_id.borrow().determine_type(self.m) {
                    if arg_type.is_inferred() {
                        continue;
                    }
                    any_concrete_type_found = true;
                    Self::unify_types_with_subst(&arg_type, &param_type, &mut subst);
                }
            }
        }

        if !any_concrete_type_found || subst.mapping.is_empty() {
            return None;
        }

        for (param_name, _) in generics.iter() {
            let param_index = callee_func.params.iter().find_map(|param_id| {
                let param = param_id.borrow();
                if Self::type_contains_generic_param_name(&param.ty, param_name) {
                    if let Type::GenericParam { index, .. } = &*param.ty {
                        Some(*index)
                    } else {
                        None
                    }
                } else {
                    None
                }
            });
            if let Some(index) = param_index {
                if !subst.mapping.contains_key(&index) {
                    return None;
                }
            }
        }

        Some(subst)
    }

    pub(crate) fn type_contains_generic_param_name(ty: &TypeId, param_name: &NString) -> bool {
        match &**ty {
            Type::GenericParam { name, .. } => name == param_name,
            Type::Array { element_type, .. } => Self::type_contains_generic_param_name(element_type, param_name),
            Type::Tuple { element_types, .. } => element_types
                .iter()
                .any(|et| Self::type_contains_generic_param_name(et, param_name)),
            Type::Reference { to, .. } | Type::Pointer { to, .. } => {
                Self::type_contains_generic_param_name(to, param_name)
            }
            Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
                Self::type_contains_generic_param_name(element_type, param_name)
            }
            _ => false,
        }
    }

    pub(crate) fn infer_generic_args_from_struct_fields(
        &self,
        struct_def_id: &StructDefId,
        field_values: &[(NString, ValueId)],
    ) -> Option<Substitution> {
        let struct_def = struct_def_id.borrow();
        let generics = struct_def.generics.as_ref()?;

        if generics.is_empty() {
            return Some(Substitution::default());
        }

        let mut subst = Substitution::default();
        let mut any_concrete_type_found = false;

        struct GenericFieldInfo {
            index: u32,
            appears: bool,
        }
        let mut param_info: BTreeMap<NString, GenericFieldInfo> = BTreeMap::new();
        for (param_name, param_default) in generics.iter() {
            let index = param_default
                .as_ref()
                .and_then(|tid| {
                    if let Type::GenericParam { index, .. } = &**tid {
                        Some(*index)
                    } else {
                        None
                    }
                })
                .unwrap_or_else(|| generics.keys().position(|k| k == param_name).unwrap_or(0) as u32);
            param_info.insert(param_name.clone(), GenericFieldInfo { index, appears: false });
        }

        for (field_name, field_value_id) in field_values {
            if let Some(field) = struct_def.fields.get(field_name) {
                for (param_name, info) in param_info.iter_mut() {
                    if Self::type_contains_generic_param(&field.ty, param_name) {
                        info.appears = true;
                    }
                }

                let field_type = &*field.ty;
                if let Ok(arg_type) = field_value_id.borrow().determine_type(self.m) {
                    let effective_type = match &arg_type {
                        Type::InferredInteger { .. } => Some(Type::I32 {
                            span: ByteSpan::default(),
                        }),
                        Type::InferredFloat { .. } => Some(Type::F64 {
                            span: ByteSpan::default(),
                        }),
                        _ => None,
                    };
                    if let Some(effective) = effective_type {
                        let field_has_generics = param_info
                            .keys()
                            .any(|k| Self::type_contains_generic_param(field_type, k));
                        if field_has_generics {
                            any_concrete_type_found = true;
                            Self::unify_types_with_subst(&effective, field_type, &mut subst);
                        }
                        continue;
                    }
                    if arg_type.is_inferred() {
                        continue;
                    }
                    any_concrete_type_found = true;
                    Self::unify_types_with_subst(&arg_type, field_type, &mut subst);
                }
            }
        }

        if !any_concrete_type_found {
            return None;
        }

        for info in param_info.values() {
            if info.appears && !subst.mapping.contains_key(&info.index) {
                return None;
            }
        }

        Some(subst)
    }

    pub(crate) fn type_contains_generic_param(ty: &Type, param_name: &NString) -> bool {
        match ty {
            Type::GenericParam { name, .. } => name == param_name,
            Type::Array { element_type, .. } => Self::type_contains_generic_param(element_type, param_name),
            Type::Tuple { element_types, .. } => element_types
                .iter()
                .any(|et| Self::type_contains_generic_param(et, param_name)),
            Type::Reference { to, .. } | Type::Pointer { to, .. } => Self::type_contains_generic_param(to, param_name),
            Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
                Self::type_contains_generic_param(element_type, param_name)
            }
            _ => false,
        }
    }

    pub(crate) fn unify_types_with_subst(arg_type: &Type, param_type: &Type, subst: &mut Substitution) {
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
            (Type::Pointer { to: a_to, .. }, Type::Pointer { to: p_to, .. }) => {
                Self::unify_types_with_subst(a_to, p_to, subst);
            }
            (Type::SlicePtr { element_type: a_e, .. }, Type::SlicePtr { element_type: p_e, .. }) => {
                Self::unify_types_with_subst(a_e, p_e, subst);
            }
            (Type::SliceRef { element_type: a_e, .. }, Type::SliceRef { element_type: p_e, .. }) => {
                Self::unify_types_with_subst(a_e, p_e, subst);
            }
            (Type::Reference { to: a_to, .. }, Type::Reference { to: p_to, .. }) => {
                Self::unify_types_with_subst(a_to, p_to, subst);
            }
            (Type::Array { element_type: a_e, .. }, Type::Array { element_type: p_e, .. }) => {
                Self::unify_types_with_subst(a_e, p_e, subst);
            }
            (
                Type::Tuple {
                    element_types: a_ets, ..
                },
                Type::Tuple {
                    element_types: p_ets, ..
                },
            ) => {
                for (a_et, p_et) in a_ets.iter().zip(p_ets.iter()) {
                    Self::unify_types_with_subst(a_et, p_et, subst);
                }
            }
            (
                Type::Function {
                    function_type: a_ft, ..
                },
                Type::Function {
                    function_type: p_ft, ..
                },
            ) => {
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

    pub(crate) fn infer_generic_args_from_constraints(
        &self,
        value_id: &ValueId,
        struct_def_id: &StructDefId,
    ) -> Option<Substitution> {
        let struct_def = struct_def_id.borrow();
        let generics = struct_def.generics.as_ref()?;

        if generics.is_empty() {
            return Some(Substitution::default());
        }

        let mut param_name_to_index: BTreeMap<NString, u32> = BTreeMap::new();
        for field in struct_def.fields.values() {
            Self::collect_generic_params_from_type(&field.ty, &mut param_name_to_index);
        }

        let ordered_param_names: Vec<&NString> = generics.keys().collect();

        let constraints = self.constraints.get(value_id)?;

        for constraint in constraints {
            let ty = constraint.type_id();
            let (generic_args, struct_def_from_constraint) = match &*ty {
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

            if let Some(ref args) = generic_args {
                if let Some(ref constraint_def) = struct_def_from_constraint {
                    if constraint_def.as_usize() != struct_def_id.as_usize() {
                        continue;
                    }

                    if args.len() != ordered_param_names.len() {
                        continue;
                    }

                    let mut subst = Substitution::default();
                    for (i, param_name) in ordered_param_names.iter().enumerate() {
                        if let Some(idx) = param_name_to_index.get(*param_name) {
                            subst.mapping.insert(*idx, args[i]);
                        } else {
                            if i < args.len() {
                                subst.mapping.insert(i as u32, args[i]);
                            }
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

    pub(crate) fn collect_generic_params_from_type(ty: &TypeId, mapping: &mut BTreeMap<NString, u32>) {
        match &**ty {
            Type::GenericParam { index, name, .. } => {
                mapping.entry(name.clone()).or_insert(*index);
            }
            Type::Array { element_type, .. } => Self::collect_generic_params_from_type(element_type, mapping),
            Type::Tuple { element_types, .. } => {
                for et in element_types.iter() {
                    Self::collect_generic_params_from_type(et, mapping);
                }
            }
            Type::Reference { to, .. } | Type::Pointer { to, .. } => {
                Self::collect_generic_params_from_type(to, mapping);
            }
            Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
                Self::collect_generic_params_from_type(element_type, mapping);
            }
            _ => {}
        }
    }

    pub(crate) fn monomorphize_struct(&mut self, struct_id: &StructDefId, subst: &Substitution) -> StructDefId {
        if self.mono_depth >= MAX_MONO_DEPTH {
            panic!("monomorphization depth limit ({}) exceeded for struct", MAX_MONO_DEPTH);
        }

        let cache_key = self.struct_mono_cache_key(struct_id, subst);
        if let Some(cached_id) = self.struct_mono_cache.get(&cache_key) {
            return cached_id.clone();
        }

        if !self.mono_in_progress.insert(cache_key) {
            return struct_id.clone();
        }

        self.mono_depth += 1;

        let struct_def = struct_id.borrow();

        self.mono_counter += 1;
        let mono_name = format!("{}::<mono-{}>", struct_def.name, self.mono_counter);
        let mono_name_ns: NString = mono_name.into();

        let mut new_fields = BTreeMap::new();
        let mut new_layout = Vec::new();

        for (field_name, field) in &struct_def.fields {
            let new_field_ty = subst.apply(&field.ty);
            let new_field = StructField {
                span: field.span,
                visibility: field.visibility,
                attributes: field.attributes.clone(),
                name: field.name.clone(),
                ty: TypeId::from(new_field_ty),
                default_value: field.default_value.clone(),
            };
            new_fields.insert(field_name.clone(), new_field);
            new_layout.push(StructMemoryLayoutCell::Field {
                field_name: field_name.clone(),
            });
        }

        let mono_struct = StructDef {
            span: ByteSpan::default(),
            visibility: struct_def.visibility,
            name: mono_name_ns,
            attributes: struct_def.attributes.clone(),
            fields: new_fields,
            generics: None,
            layout: new_layout.into(),
        };

        let mono_id: StructDefId = mono_struct.into();
        self.m.add_struct(mono_id.clone());
        self.struct_mono_cache
            .insert(cache_key, StructMonoCacheValue::from(mono_id.clone()));

        self.mono_depth -= 1;
        self.mono_in_progress.remove(&cache_key);

        mono_id
    }

    pub(crate) fn monomorphize_function(&mut self, func_id: &FunctionId, subst: &Substitution) -> FunctionId {
        if self.mono_depth >= MAX_MONO_DEPTH {
            panic!(
                "monomorphization depth limit ({}) exceeded for function",
                MAX_MONO_DEPTH
            );
        }

        let cache_key = self.mono_cache_key(func_id, subst);
        if let Some(existing) = self.mono_cache.get(&cache_key) {
            return existing.clone();
        }

        if !self.mono_in_progress.insert(cache_key) {
            return func_id.clone();
        }

        self.mono_depth += 1;

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
                ParameterId::from(Parameter {
                    span: param.span,
                    attributes: param.attributes.clone(),
                    is_mutable: param.is_mutable,
                    name: param.name.clone(),
                    ty: TypeId::from(new_ty),
                    default_value: param.default_value.clone(),
                })
            })
            .collect();

        let new_return_type = TypeId::from(subst.apply(&func.return_type));

        let new_body = func.body.as_ref().map(|body| {
            body.iter()
                .map(|element| self.clone_block_element(element, subst))
                .collect()
        });

        let mono_func = Function {
            span: ByteSpan::default(),
            visibility: func.visibility,
            attributes: func.attributes.clone(),
            is_unsafe: func.is_unsafe,
            name: mono_name_ns,
            mangled_name: Some(mono_mangled_name),
            generics: None,
            params: new_params,
            return_type: new_return_type,
            body: new_body,
        };

        let mono_id: FunctionId = mono_func.into();
        self.m.add_function(mono_id.clone());
        self.mono_cache.insert(cache_key, mono_id.clone());

        self.mono_depth -= 1;
        self.mono_in_progress.remove(&cache_key);

        mono_id
    }

    pub(crate) fn clone_block_element(&self, element: &BlockElement, subst: &Substitution) -> BlockElement {
        match element {
            BlockElement::Expr(expr_id) => {
                let value = expr_id.borrow();
                let new_value = self.apply_subst_to_value(&value, subst);
                BlockElement::Expr(ValueId::from(new_value))
            }
            BlockElement::Local(local_id) => {
                let local = local_id.borrow();
                let new_ty = subst.apply(&local.ty);
                let new_init = local.initializer.as_ref().map(|init_id| {
                    let init_val = init_id.borrow();
                    ValueId::from(self.apply_subst_to_value(&init_val, subst))
                });
                BlockElement::Local(LocalVariableId::from(LocalVariable {
                    span: local.span,
                    kind: local.kind.clone(),
                    attributes: local.attributes.clone(),
                    is_mutable: local.is_mutable,
                    name: local.name.clone(),
                    ty: TypeId::from(new_ty),
                    initializer: new_init,
                }))
            }
        }
    }

    pub(crate) fn apply_subst_to_value(&self, value: &Value, subst: &Substitution) -> Value {
        match value {
            Value::Cast {
                value: v, target_type, ..
            } => {
                let new_target = subst.apply(target_type);
                Value::Cast {
                    span: ByteSpan::default(),
                    value: v.clone(),
                    target_type: TypeId::from(new_target),
                }
            }
            Value::StructObject { struct_def, fields, .. } => {
                let struct_def_b = struct_def.borrow();
                if struct_def_b.generics.is_some() {
                    let new_fields: ThinVec<(NString, ValueId)> = fields
                        .iter()
                        .map(|(name, val_id)| {
                            let val = val_id.borrow();
                            let new_val = self.apply_subst_to_value(&val, subst);
                            (name.clone(), ValueId::from(new_val))
                        })
                        .collect();
                    Value::StructObject {
                        span: ByteSpan::default(),
                        struct_def: struct_def.clone(),
                        fields: new_fields,
                    }
                } else {
                    Value::StructObject {
                        span: ByteSpan::default(),
                        struct_def: struct_def.clone(),
                        fields: fields.clone(),
                    }
                }
            }
            Value::Call { callee, args, .. } => Value::Call {
                span: ByteSpan::default(),
                callee: callee.clone(),
                args: Arguments {
                    positional: args.positional.clone(),
                    named: args.named.clone(),
                },
            },
            Value::FunctionSymbol { id, .. } => Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: id.clone(),
            },
            val => val.clone(),
        }
    }
}
