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

/// Cache value type alias for struct monomorphization cache.
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

        if param_types.len() != positional_args.len() {
            return Some(Substitution::default()); // Mismatch is caught elsewhere
        }

        for (arg_value_id, param_type_id) in positional_args.iter().zip(param_types.iter()) {
            let arg_type = arg_value_id.borrow().determine_type(self.m).ok()?;
            let param_type = param_type_id;
            Self::unify_types_with_subst(&arg_type, param_type, &mut subst);
        }

        // If we weren't able to infer any generic args, return None
        if subst.mapping.is_empty() {
            return None;
        }

        Some(subst)
    }

    /// Infer concrete types for generic parameters from named argument types at a call site.
    /// This extends generic inference to handle calls where arguments are passed by name.
    /// (#11 - Named arg support in generic inference)
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

        // Match named args to parameters by name
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

        // Also try positional args matched by position (for mixed positional/named calls)
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

        // Check that all generic params have been bound
        for (param_name, _) in generics.iter() {
            // Find if this generic param index has been bound
            let mut found = false;
            for (_idx, _tid) in &subst.mapping {
                // Check if this param appears in any param type
                for param_id in &callee_func.params {
                    let param = param_id.borrow();
                    if subst.mapping.values().any(|v| *v == param.ty) {
                        // Already bound via unify
                    }
                    if Self::type_contains_generic_param_name(&param.ty, param_name) {
                        // This param needs to be bound - check if it was
                        if let Type::GenericParam { index, .. } = &*param.ty {
                            if subst.mapping.contains_key(index) {
                                found = true;
                            }
                        }
                    }
                }
            }
            // Simple check: if we made any mapping, assume it's sufficient
            if !subst.mapping.is_empty() {
                found = true;
            }
            if !found && subst.mapping.is_empty() {
                return None;
            }
        }

        Some(subst)
    }

    /// Check if a type contains a generic parameter name (for named arg inference).
    fn type_contains_generic_param_name(ty: &TypeId, param_name: &NString) -> bool {
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

        // Collect which generic params appear in which fields in a single pass
        // to avoid O(g × f) iteration.
        struct GenericFieldInfo {
            index: u32,
            appears: bool,
        }
        let mut param_info: BTreeMap<NString, GenericFieldInfo> = BTreeMap::new();
        for (param_name, param_default) in generics.iter() {
            // If the generic has a concrete type (from default), use its index.
            // Otherwise we rely on name matching during unify.
            let index = param_default
                .as_ref()
                .and_then(|tid| {
                    if let Type::GenericParam { index, .. } = &**tid {
                        Some(*index)
                    } else {
                        None
                    }
                })
                .unwrap_or(0);
            param_info.insert(param_name.clone(), GenericFieldInfo { index, appears: false });
        }

        for (field_name, field_value_id) in field_values {
            if let Some(field) = struct_def.fields.get(field_name) {
                // Mark which generic params appear in this field's type
                for (param_name, info) in param_info.iter_mut() {
                    if Self::type_contains_generic_param(&field.ty, param_name) {
                        info.appears = true;
                    }
                }

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
        for info in param_info.values() {
            if info.appears && !subst.mapping.contains_key(&info.index) {
                return None;
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

    /// Infer concrete type arguments for generic struct parameters from parent constraints.
    /// This handles the case where the struct expression has a type annotation,
    /// e.g., `let x: Pair<i32> = Pair { first: 1, second: 2 };`
    /// In this case, the constraint `Equal(Parameterized { base: Struct { def: Pair }, args: [i32] })`
    /// is on the struct object's ValueId, and we extract the args from it.
    ///
    /// To map positional type args to generic parameter indices, we look at the struct's
    /// field types to find GenericParam types, then match their names to the ordered
    /// list of generic parameter names.
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

        // Build a name->index mapping from the struct's field types
        // by extracting GenericParam indices from field types
        let mut param_name_to_index: BTreeMap<NString, u32> = BTreeMap::new();
        for field in struct_def.fields.values() {
            Self::collect_generic_params_from_type(&field.ty, &mut param_name_to_index);
        }

        // Build ordered list of param names matching declaration order in generics map
        let ordered_param_names: Vec<&NString> = generics.keys().collect();

        // Look at constraints on this value
        let constraints = self.constraints.get(value_id)?;

        for constraint in constraints {
            let ty = constraint.type_id();
            // The constraint type could be a Parameterized wrapping a Struct, or a Struct directly
            let (generic_args, struct_def_from_constraint) = match &*ty {
                Type::Parameterized { base, args, .. } => {
                    if let Type::Struct { def, .. } = &**base {
                        (Some(args.positional.clone()), Some(def.clone()))
                    } else {
                        (None, None)
                    }
                }
                Type::Struct { def, .. } => {
                    // Direct struct type without params
                    (None, Some(def.clone()))
                }
                _ => (None, None),
            };

            if let Some(args) = generic_args {
                if let Some(constraint_def) = struct_def_from_constraint {
                    // Verify the struct def matches (same identity)
                    if constraint_def.as_usize() != struct_def_id.as_usize() {
                        continue;
                    }

                    if args.len() != ordered_param_names.len() {
                        continue;
                    }

                    let mut subst = Substitution::default();
                    for (i, param_name) in ordered_param_names.iter().enumerate() {
                        // Try matching by unqualified name from the generics map key
                        if let Some(idx) = param_name_to_index.get(*param_name) {
                            subst.mapping.insert(*idx, args[i]);
                        } else {
                            // Fallback: try matching by position if field types use qualified names
                            // (e.g. generics key is "T" but field type has GenericParam name "pkg::Pair::T")
                            if i < args.len() {
                                // Use position i as the index - generics were inserted in declaration order
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

    /// Recursively collect GenericParam name-to-index mappings from a type.
    fn collect_generic_params_from_type(ty: &TypeId, mapping: &mut BTreeMap<NString, u32>) {
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

    /// Monomorphize a generic struct by creating a concrete copy with substituted field types.
    /// Includes cycle detection via depth tracking to prevent infinite recursion. (#18)
    pub(crate) fn monomorphize_struct(&mut self, struct_id: &StructDefId, subst: &Substitution) -> StructDefId {
        // Cycle detection: check depth limit
        if self.mono_depth >= MAX_MONO_DEPTH {
            panic!("monomorphization depth limit ({}) exceeded for struct", MAX_MONO_DEPTH);
        }

        // Check cache first
        let cache_key = self.struct_mono_cache_key(struct_id, subst);
        if let Some(cached_id) = self.struct_mono_cache.get(&cache_key) {
            return cached_id.clone();
        }

        // Cycle detection: check if this monomorphization is already in progress
        if !self.mono_in_progress.insert(cache_key) {
            // Already being monomorphized - return original to break cycle
            return struct_id.clone();
        }

        self.mono_depth += 1;

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
        // Cache for future identical instantiations
        self.struct_mono_cache
            .insert(cache_key, StructMonoCacheValue::from(mono_id.clone()));

        self.mono_depth -= 1;
        self.mono_in_progress.remove(&cache_key);

        mono_id
    }

    pub(crate) fn monomorphize_function(&mut self, func_id: &FunctionId, subst: &Substitution) -> FunctionId {
        // Cycle detection: check depth limit
        if self.mono_depth >= MAX_MONO_DEPTH {
            panic!(
                "monomorphization depth limit ({}) exceeded for function",
                MAX_MONO_DEPTH
            );
        }

        // Check cache first
        let cache_key = self.mono_cache_key(func_id, subst);
        if let Some(existing) = self.mono_cache.get(&cache_key) {
            return existing.clone();
        }

        // Cycle detection: check if this monomorphization is already in progress
        if !self.mono_in_progress.insert(cache_key) {
            // Already being monomorphized - return original to break cycle
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
            name: mono_name_ns,
            mangled_name: mono_mangled_name,
            generics: None,
            params: new_params,
            return_type: new_return_type,
            body: new_body,
        };

        let mono_id: FunctionId = mono_func.into();
        // Register the monomorphized function in the symbol table
        self.m.add_function(mono_id.clone());
        // Cache for future identical instantiations
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
                let new_init_val = local.initializer.borrow();
                let new_init = self.apply_subst_to_value(&new_init_val, subst);
                BlockElement::Local(LocalVariableId::from(LocalVariable {
                    span: local.span,
                    kind: local.kind.clone(),
                    attributes: local.attributes.clone(),
                    is_mutable: local.is_mutable,
                    name: local.name.clone(),
                    ty: TypeId::from(new_ty),
                    initializer: ValueId::from(new_init),
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
