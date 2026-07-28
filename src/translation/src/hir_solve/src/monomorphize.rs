use crate::solver::Solver;
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

impl<'m> Solver<'m> {
    pub(crate) fn mono_cache_key(&self, func_id: &FunctionId, subst: &Substitution) -> (usize, Vec<(u32, TypeId)>) {
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        (func_id.as_usize(), sorted_args)
    }

    pub(crate) fn struct_mono_cache_key(
        &self,
        struct_id: &StructDefId,
        subst: &Substitution,
    ) -> (usize, Vec<(u32, TypeId)>) {
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        (struct_id.as_usize(), sorted_args)
    }

    /// Infer concrete types for generic parameters from argument types at a call site.
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

        for (arg_value_id, param_type_id) in positional_args.iter().zip(param_types.iter()) {
            let arg_type = arg_value_id.borrow().determine_type(self.m).ok()?;
            let param_type = param_type_id;
            Self::unify_types_with_subst(&arg_type, param_type, &mut subst);
        }

        Some(subst)
    }

    /// Infer concrete types for generic struct parameters from field values.
    /// Returns None if inference is incomplete (some generics still unbound).
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

        for (field_name, field_value_id) in field_values {
            if let Some(field) = struct_def.fields.get(field_name) {
                let field_type = &*field.ty;
                // Only unify if we can determine the value's type
                if let Ok(arg_type) = field_value_id.borrow().determine_type(self.m) {
                    // Skip if the value type is still inferred (not yet concrete)
                    if arg_type.is_inferred() {
                        continue;
                    }
                    any_concrete_type_found = true;
                    Self::unify_types_with_subst(&arg_type, field_type, &mut subst);
                }
            }
        }

        // If no field values have concrete types yet, we can't infer
        if !any_concrete_type_found {
            return None;
        }

        // Check that all generic params that appear in field types have been bound
        for _param_name in generics.keys() {
            // Check if this generic param appears in any field type
            let appears_in_fields = struct_def
                .fields
                .values()
                .any(|f| Self::type_contains_generic_param(&f.ty, _param_name));
            if appears_in_fields {
                let _idx = Self::find_generic_index_in_type(
                    &struct_def.fields.values().next().map(|f| &f.ty).cloned().unwrap_or(
                        Type::Unit {
                            span: ByteSpan::default(),
                        }
                        .into(),
                    ),
                    _param_name,
                )
                .unwrap_or(0);
                // Find the correct index for this param name
                let generic_idx = struct_def
                    .fields
                    .values()
                    .find_map(|f| match &*f.ty {
                        Type::GenericParam { index, name, .. } if name == _param_name => Some(*index),
                        _ => Self::find_generic_index_in_type_deep(&f.ty, _param_name),
                    })
                    .unwrap_or(0);
                if generic_idx > 0 && !subst.mapping.contains_key(&generic_idx) {
                    // This generic param is used in field types but couldn't be inferred
                    // Return None to defer monomorphization until types are resolved
                    return None;
                }
            }
        }

        Some(subst)
    }

    fn type_contains_generic_param(ty: &Type, param_name: &NString) -> bool {
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

    fn find_generic_index_in_type_deep(ty: &Type, param_name: &NString) -> Option<u32> {
        match ty {
            Type::GenericParam { index, name, .. } if name == param_name => Some(*index),
            Type::Array { element_type, .. } => Self::find_generic_index_in_type_deep(element_type, param_name),
            Type::Tuple { element_types, .. } => {
                for et in element_types.iter() {
                    if let Some(idx) = Self::find_generic_index_in_type_deep(et, param_name) {
                        return Some(idx);
                    }
                }
                None
            }
            Type::Reference { to, .. } | Type::Pointer { to, .. } => {
                Self::find_generic_index_in_type_deep(to, param_name)
            }
            Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
                Self::find_generic_index_in_type_deep(element_type, param_name)
            }
            _ => None,
        }
    }

    fn find_generic_index_in_type(ty: &Type, param_name: &NString) -> Option<u32> {
        match ty {
            Type::GenericParam { index, name, .. } if name == param_name => Some(*index),
            Type::Array { element_type, .. } => Self::find_generic_index_in_type(element_type, param_name),
            Type::Tuple { element_types, .. } => {
                for et in element_types.iter() {
                    if let Some(idx) = Self::find_generic_index_in_type(et, param_name) {
                        return Some(idx);
                    }
                }
                None
            }
            Type::Reference { to, .. } | Type::Pointer { to, .. } => Self::find_generic_index_in_type(to, param_name),
            Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
                Self::find_generic_index_in_type(element_type, param_name)
            }
            _ => None,
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
            // Handle struct types - unify inside
            (arg, Type::Struct { def: struct_def_id, .. }) => {
                // For struct types, inspect field types for generic params
                let struct_def = struct_def_id.borrow();
                if struct_def.generics.is_some() {
                    // Try to extract concrete types from the arg struct type
                    if let Type::Struct { def: arg_def, .. } = arg
                        && arg_def != struct_def_id
                    {
                        // Different struct, nothing to unify
                        return;
                    }
                    for field in struct_def.fields.values() {
                        if let Type::GenericParam { index: _, .. } = &*field.ty {
                            // This doesn't give us concrete types from arg directly
                            // Need to look at the actual value to infer
                        }
                    }
                }
            }
            _ => {}
        }
    }

    /// Monomorphize a generic struct by creating a concrete copy with substituted field types.
    pub(crate) fn monomorphize_struct(&mut self, struct_id: &StructDefId, subst: &Substitution) -> StructDefId {
        // Generate a struct-level cache key
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        let _cache_key = (struct_id.as_usize(), sorted_args);

        // Store the cache in the solver (we use a separate field for struct mono cache)
        // For now, we don't cache struct monomorphization (they're simple enough)

        let struct_def = struct_id.borrow();

        self.mono_counter += 1;
        let mono_name = format!("{}::<mono-{}>", struct_def.name, self.mono_counter);
        let mono_name_ns: NString = mono_name.into();

        // Apply substitution to each field's type
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
            generics: None, // Monomorphized - no more generics
            layout: new_layout.into(),
        };

        let mono_id: StructDefId = mono_struct.into();
        // Register in symbol table so codegen can find it
        self.m.add_struct(mono_id.clone());
        mono_id
    }

    /// Try to substitute `type GenericParam` types in a value to concrete types.
    /// This is needed when struct field values contain generic types.
    pub(crate) fn substitute_in_value(&self, value: &Value, subst: &Substitution) -> Value {
        match value {
            Value::Cast {
                value: v, target_type, ..
            } => {
                let new_target = subst.apply(target_type);
                let v_borrowed = v.borrow();
                let new_v = self.substitute_in_value(&v_borrowed, subst);
                Value::Cast {
                    span: ByteSpan::default(),
                    value: ValueId::from(new_v),
                    target_type: TypeId::from(new_target),
                }
            }
            Value::StructObject { struct_def, fields, .. } => {
                let new_fields: ThinVec<(NString, ValueId)> = fields
                    .iter()
                    .map(|(name, val_id)| {
                        let val_borrowed = val_id.borrow();
                        let new_val = self.substitute_in_value(&val_borrowed, subst);
                        (name.clone(), ValueId::from(new_val))
                    })
                    .collect();
                Value::StructObject {
                    span: ByteSpan::default(),
                    struct_def: struct_def.clone(),
                    fields: new_fields,
                }
            }
            // For literals and symbols, just clone
            _ => value.clone(),
        }
    }

    pub(crate) fn monomorphize_function(&mut self, func_id: &FunctionId, subst: &Substitution) -> FunctionId {
        // Check cache first — if we already monomorphized this generic function
        // with identical concrete type arguments, return the existing copy.
        let cache_key = self.mono_cache_key(func_id, subst);
        if let Some(existing) = self.mono_cache.get(&cache_key) {
            return existing.clone();
        }

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
                    span: param.span,
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
            span: ByteSpan::default(),
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
        // Cache for future identical instantiations
        self.mono_cache.insert(cache_key, mono_id.clone());
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
                let new_init_val = local.initializer.borrow();
                let new_init = self.apply_subst_to_value(&new_init_val, subst);
                let new_local = LocalVariable {
                    span: local.span,
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
                    // Apply substitution to field value types if they contain generic params
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
            Value::Call { callee, args, .. } => {
                let new_args = Arguments {
                    positional: args.positional.clone(),
                    named: args.named.clone(),
                };
                Value::Call {
                    span: ByteSpan::default(),
                    callee: callee.clone(),
                    args: new_args,
                }
            }
            Value::FunctionSymbol { id, .. } => Value::FunctionSymbol {
                span: ByteSpan::default(),
                id: id.clone(),
            },
            // For all other values, just clone
            val => val.clone(),
        }
    }
}
