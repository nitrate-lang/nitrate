//! Generic function and struct monomorphization.
//!
//! When a generic function or struct is used with concrete type arguments,
//! the monomorphizer creates a specialized copy with all generic parameters
//! substituted. Copies are cached to avoid duplicate instantiations.

use crate::constraints::ConstraintGraph;
use crate::substitution::Substitution;
use nitrate_hir::{
    Arguments, BlockElement, Function, FunctionId, LocalVariable, LocalVariableId, Parameter, ParameterId, StructDef,
    StructDefId, StructField, StructMemoryLayoutCell, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_hir_type::HirGetType;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::{BTreeMap, HashMap, HashSet};

/// Maximum monomorphization depth to prevent infinite recursion.
const MAX_MONO_DEPTH: u32 = 64;

/// A reference to a trait (name + id), used for trait-bound constraints.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) struct TraitRef {
    pub name: NString,
}

/// Compact cache key for monomorphization, avoiding heap allocation.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub(crate) struct MonoCacheKey(u64);

impl MonoCacheKey {
    pub fn new(original_id: usize, subst_type_args: &[(u32, TypeId)]) -> Self {
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

/// Monomorphization state: caches, counter, and depth tracking.
pub(crate) struct Monomorphizer {
    /// Monotonically increasing counter for naming monomorphized copies.
    pub counter: u32,
    /// Cache: (original_func_hash) → monomorphized FunctionId.
    pub func_cache: HashMap<MonoCacheKey, FunctionId>,
    /// Cache: (original_struct_hash) → monomorphized StructDefId.
    pub struct_cache: HashMap<MonoCacheKey, StructDefId>,
    /// Current monomorphization depth (for recursion detection).
    pub depth: u32,
    /// Set of cache keys currently being monomorphized (prevents cycles).
    pub in_progress: HashSet<MonoCacheKey>,
}

impl Monomorphizer {
    pub fn new() -> Self {
        Self {
            counter: 0,
            func_cache: HashMap::new(),
            struct_cache: HashMap::new(),
            depth: 0,
            in_progress: HashSet::new(),
        }
    }

    /// Build a cache key for a function monomorphization.
    pub fn func_cache_key(func_id: &FunctionId, subst: &Substitution) -> MonoCacheKey {
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        MonoCacheKey::new(func_id.as_usize(), &sorted_args)
    }

    /// Build a cache key for a struct monomorphization.
    pub fn struct_cache_key(struct_id: &StructDefId, subst: &Substitution) -> MonoCacheKey {
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        MonoCacheKey::new(struct_id.as_usize(), &sorted_args)
    }

    /// Infer generic type arguments from a function call's positional arguments.
    pub fn infer_from_call(
        callee_func_id: &FunctionId,
        positional_args: &[ValueId],
        symbol_tab: &SymbolTab,
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
            let arg_type = arg_value_id.borrow().determine_type(symbol_tab).ok()?;
            unify_types_with_subst(&arg_type, param_type_id, &mut subst);
        }

        if subst.mapping.is_empty() {
            return None;
        }

        Some(subst)
    }

    /// Infer generic type arguments from struct field values.
    pub fn infer_from_struct_fields(
        struct_def_id: &StructDefId,
        field_values: &[(NString, ValueId)],
        symbol_tab: &SymbolTab,
    ) -> Option<Substitution> {
        let struct_def = struct_def_id.borrow();
        let generics = struct_def.generics.as_ref()?;

        if generics.is_empty() {
            return Some(Substitution::default());
        }

        let mut subst = Substitution::default();
        let mut any_concrete_type_found = false;

        // Track which generic params appear in field types.
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
                    if type_contains_generic_param_name(&field.ty, param_name) {
                        info.appears = true;
                    }
                }

                let field_type = &*field.ty;
                if let Ok(arg_type) = field_value_id.borrow().determine_type(symbol_tab) {
                    if arg_type.is_inferred() {
                        continue;
                    }
                    any_concrete_type_found = true;
                    unify_types_with_subst(&arg_type, field_type, &mut subst);
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

    /// Monomorphize a generic function, creating a concrete copy.
    pub fn monomorphize_function(
        &mut self,
        func_id: &FunctionId,
        subst: &Substitution,
        symbol_tab: &mut SymbolTab,
    ) -> FunctionId {
        if self.depth >= MAX_MONO_DEPTH {
            panic!("monomorphization depth limit ({MAX_MONO_DEPTH}) exceeded for function");
        }

        let cache_key = Self::func_cache_key(func_id, subst);
        if let Some(existing) = self.func_cache.get(&cache_key) {
            return existing.clone();
        }

        if !self.in_progress.insert(cache_key) {
            return func_id.clone();
        }

        self.depth += 1;
        self.counter += 1;

        let func = func_id.borrow();
        let mono_name = format!("{}::<mono-{}>", func.name, self.counter);
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

        let new_body = func
            .body
            .as_ref()
            .map(|body| body.iter().map(|element| clone_block_element(element, subst)).collect());

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
        symbol_tab.add_function(mono_id.clone());
        self.func_cache.insert(cache_key, mono_id.clone());

        self.depth -= 1;
        self.in_progress.remove(&cache_key);

        mono_id
    }

    /// Monomorphize a generic struct, creating a concrete copy.
    pub fn monomorphize_struct(
        &mut self,
        struct_id: &StructDefId,
        subst: &Substitution,
        symbol_tab: &mut SymbolTab,
    ) -> StructDefId {
        if self.depth >= MAX_MONO_DEPTH {
            panic!("monomorphization depth limit ({MAX_MONO_DEPTH}) exceeded for struct");
        }

        let cache_key = Self::struct_cache_key(struct_id, subst);
        if let Some(cached_id) = self.struct_cache.get(&cache_key) {
            return cached_id.clone();
        }

        if !self.in_progress.insert(cache_key) {
            return struct_id.clone();
        }

        self.depth += 1;
        self.counter += 1;

        let struct_def = struct_id.borrow();
        let mono_name = format!("{}::<mono-{}>", struct_def.name, self.counter);
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
        symbol_tab.add_struct(mono_id.clone());
        self.struct_cache.insert(cache_key, mono_id.clone());

        self.depth -= 1;
        self.in_progress.remove(&cache_key);

        mono_id
    }
}

/// Unify two types, extracting generic parameter bindings into a substitution.
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
        (Type::Pointer { to: a_to, .. }, Type::Pointer { to: p_to, .. }) => {
            unify_types_with_subst(a_to, p_to, subst);
        }
        (Type::SlicePtr { element_type: a_e, .. }, Type::SlicePtr { element_type: p_e, .. }) => {
            unify_types_with_subst(a_e, p_e, subst);
        }
        (Type::SliceRef { element_type: a_e, .. }, Type::SliceRef { element_type: p_e, .. }) => {
            unify_types_with_subst(a_e, p_e, subst);
        }
        (Type::Reference { to: a_to, .. }, Type::Reference { to: p_to, .. }) => {
            unify_types_with_subst(a_to, p_to, subst);
        }
        (Type::Array { element_type: a_e, .. }, Type::Array { element_type: p_e, .. }) => {
            unify_types_with_subst(a_e, p_e, subst);
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
                unify_types_with_subst(a_et, p_et, subst);
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
            unify_types_with_subst(&a_ft.return_type, &p_ft.return_type, subst);
            for ((_, a_p), (_, p_p)) in a_ft.params.iter().zip(p_ft.params.iter()) {
                unify_types_with_subst(a_p, p_p, subst);
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

/// Check if a type (by reference) contains a specific generic parameter name.
fn type_contains_generic_param_name(ty: &TypeId, param_name: &NString) -> bool {
    match &**ty {
        Type::GenericParam { name, .. } => name == param_name,
        Type::Array { element_type, .. } => type_contains_generic_param_name(element_type, param_name),
        Type::Tuple { element_types, .. } => element_types
            .iter()
            .any(|et| type_contains_generic_param_name(et, param_name)),
        Type::Reference { to, .. } | Type::Pointer { to, .. } => type_contains_generic_param_name(to, param_name),
        Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
            type_contains_generic_param_name(element_type, param_name)
        }
        _ => false,
    }
}

/// Clone a block element for monomorphization, applying a substitution.
fn clone_block_element(element: &BlockElement, subst: &Substitution) -> BlockElement {
    match element {
        BlockElement::Expr(expr_id) => {
            let value = expr_id.borrow();
            let new_value = apply_subst_to_value(&value, subst);
            BlockElement::Expr(ValueId::from(new_value))
        }
        BlockElement::Local(local_id) => {
            let local = local_id.borrow();
            let new_ty = subst.apply(&local.ty);
            let new_init = local.initializer.as_ref().map(|init_id| {
                let init_val = init_id.borrow();
                ValueId::from(apply_subst_to_value(&init_val, subst))
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

/// Apply a substitution to a value, for cloning monomorphized bodies.
fn apply_subst_to_value(value: &Value, subst: &Substitution) -> Value {
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
                let new_fields: thin_vec::ThinVec<(NString, ValueId)> = fields
                    .iter()
                    .map(|(name, val_id)| {
                        let val = val_id.borrow();
                        let new_val = apply_subst_to_value(&val, subst);
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
