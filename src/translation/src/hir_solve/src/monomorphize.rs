use crate::solver::Solver;
use crate::substitution::Substitution;
use nitrate_hir::{
    Arguments, BlockElement, Function, FunctionId, LocalVariable, LocalVariableId, Parameter, ParameterId, Type,
    TypeId, Value, ValueId,
};
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use thin_vec::ThinVec;

impl<'m> Solver<'m> {
    pub(crate) fn mono_cache_key(&self, func_id: &FunctionId, subst: &Substitution) -> (usize, Vec<(u32, TypeId)>) {
        let mut sorted_args: Vec<(u32, TypeId)> = subst.mapping.iter().map(|(k, v)| (*k, *v)).collect();
        sorted_args.sort_by_key(|(k, _)| *k);
        (func_id.as_usize(), sorted_args)
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
            let param_type = &*param_type_id;
            Self::unify_types_with_subst(&arg_type, param_type, &mut subst);
        }

        Some(subst)
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
}
