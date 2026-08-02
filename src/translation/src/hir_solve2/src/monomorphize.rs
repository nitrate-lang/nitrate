use crate::substitution::Substitution;
use nitrate_hir::{Arguments, BlockElement, LocalVariable, LocalVariableId, Type, TypeId, Value, ValueId};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::BTreeMap;
use thin_vec::ThinVec;

pub(crate) const MAX_MONO_DEPTH: u32 = 64;

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
        (Type::Pointer { to: a_to, .. }, Type::Pointer { to: p_to, .. }) => unify_types_with_subst(a_to, p_to, subst),
        (Type::SlicePtr { element_type: a_e, .. }, Type::SlicePtr { element_type: p_e, .. }) => {
            unify_types_with_subst(a_e, p_e, subst)
        }
        (Type::SliceRef { element_type: a_e, .. }, Type::SliceRef { element_type: p_e, .. }) => {
            unify_types_with_subst(a_e, p_e, subst)
        }
        (Type::Reference { to: a_to, .. }, Type::Reference { to: p_to, .. }) => {
            unify_types_with_subst(a_to, p_to, subst)
        }
        (Type::Array { element_type: a_e, .. }, Type::Array { element_type: p_e, .. }) => {
            unify_types_with_subst(a_e, p_e, subst)
        }
        (
            Type::Tuple {
                element_types: a_ets, ..
            },
            Type::Tuple {
                element_types: p_ets, ..
            },
        ) => {
            for (a, p) in a_ets.iter().zip(p_ets.iter()) {
                unify_types_with_subst(a, p, subst);
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
        (
            Type::Parameterized {
                base: a_base,
                args: a_args,
                ..
            },
            Type::Parameterized {
                base: p_base,
                args: p_args,
                ..
            },
        ) => {
            unify_types_with_subst(a_base, p_base, subst);
            for (a, p) in a_args.positional.iter().zip(p_args.positional.iter()) {
                unify_types_with_subst(a, p, subst);
            }
            // Match named arguments by name, not by position. Zip would
            // pair in iteration order, which is wrong if orderings differ.
            for (a_k, a_v) in a_args.named.iter() {
                if let Some((_, p_v)) = p_args.named.iter().find(|(k, _)| k == a_k) {
                    unify_types_with_subst(a_v, p_v, subst);
                }
            }
        }
        _ => {}
    }
}

pub(crate) fn type_contains_any_generic_param(ty: &Type) -> bool {
    match ty {
        Type::GenericParam { .. } => true,
        Type::Array { element_type, .. } => type_contains_any_generic_param(element_type),
        Type::Tuple { element_types, .. } => element_types.iter().any(|et| type_contains_any_generic_param(et)),
        Type::Reference { to, .. } | Type::Pointer { to, .. } => type_contains_any_generic_param(to),
        Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
            type_contains_any_generic_param(element_type)
        }
        Type::Refine { base, .. } => type_contains_any_generic_param(base),
        Type::Function { function_type, .. } => {
            type_contains_any_generic_param(&function_type.return_type)
                || function_type
                    .params
                    .iter()
                    .any(|(_, p)| type_contains_any_generic_param(p))
        }
        Type::Parameterized { base, args, .. } => {
            type_contains_any_generic_param(base)
                || args.positional.iter().any(|a| type_contains_any_generic_param(a))
                || args.named.iter().any(|(_, v)| type_contains_any_generic_param(v))
        }
        _ => false,
    }
}

pub(crate) fn type_contains_generic_param_name(ty: &TypeId, param_name: &NString) -> bool {
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
        Type::Refine { base, .. } => type_contains_generic_param_name(base, param_name),
        Type::Function { function_type, .. } => {
            type_contains_generic_param_name(&function_type.return_type, param_name)
                || function_type
                    .params
                    .iter()
                    .any(|(_, p)| type_contains_generic_param_name(p, param_name))
        }
        Type::Parameterized { base, args, .. } => {
            type_contains_generic_param_name(base, param_name)
                || args
                    .positional
                    .iter()
                    .any(|a| type_contains_generic_param_name(a, param_name))
                || args
                    .named
                    .iter()
                    .any(|(_, v)| type_contains_generic_param_name(v, param_name))
        }
        _ => false,
    }
}

pub(crate) fn collect_generic_params_from_type(ty: &TypeId, mapping: &mut BTreeMap<NString, u32>) {
    match &**ty {
        Type::GenericParam { index, name, .. } => {
            mapping.entry(name.clone()).or_insert(*index);
        }
        Type::Array { element_type, .. } => collect_generic_params_from_type(element_type, mapping),
        Type::Tuple { element_types, .. } => {
            for et in element_types {
                collect_generic_params_from_type(et, mapping);
            }
        }
        Type::Reference { to, .. } | Type::Pointer { to, .. } => collect_generic_params_from_type(to, mapping),
        Type::SliceRef { element_type, .. } | Type::SlicePtr { element_type, .. } => {
            collect_generic_params_from_type(element_type, mapping)
        }
        Type::Refine { base, .. } => collect_generic_params_from_type(base, mapping),
        Type::Function { function_type, .. } => {
            collect_generic_params_from_type(&function_type.return_type, mapping);
            for (_, p) in &function_type.params {
                collect_generic_params_from_type(p, mapping);
            }
        }
        Type::Parameterized { base, args, .. } => {
            collect_generic_params_from_type(base, mapping);
            for a in &args.positional {
                collect_generic_params_from_type(a, mapping);
            }
            for (_, v) in &args.named {
                collect_generic_params_from_type(v, mapping);
            }
        }
        _ => {}
    }
}

pub(crate) fn clone_block_element(element: &BlockElement, subst: &Substitution) -> BlockElement {
    match element {
        BlockElement::Expr(id) => {
            let v = id.borrow();
            let nv = apply_subst_to_value(&v, subst);
            BlockElement::Expr(ValueId::from(nv))
        }
        BlockElement::Local(lid) => {
            let l = lid.borrow();
            let nt = subst.apply(&l.ty);
            let ni = l.initializer.as_ref().map(|iid| {
                let iv = iid.borrow();
                ValueId::from(apply_subst_to_value(&iv, subst))
            });
            BlockElement::Local(LocalVariableId::from(LocalVariable {
                span: l.span,
                kind: l.kind.clone(),
                attributes: l.attributes.clone(),
                is_mutable: l.is_mutable,
                name: l.name.clone(),
                ty: TypeId::from(nt),
                initializer: ni,
            }))
        }
    }
}

pub(crate) fn apply_subst_to_value(value: &Value, subst: &Substitution) -> Value {
    match value {
        Value::Cast {
            value: v, target_type, ..
        } => {
            let nt = subst.apply(target_type);
            // Recurse into the casted value so that nested generics (e.g. T in `T as i32`)
            // are also substituted.
            let nv = apply_subst_to_value(&v.borrow(), subst);
            Value::Cast {
                span: ByteSpan::default(),
                value: ValueId::from(nv),
                target_type: TypeId::from(nt),
            }
        }
        Value::StructObject { struct_def, fields, .. } => {
            if struct_def.borrow().generics.is_some() {
                let nf: ThinVec<(NString, ValueId)> = fields
                    .iter()
                    .map(|(n, vid)| {
                        let v = vid.borrow();
                        (n.clone(), ValueId::from(apply_subst_to_value(&v, subst)))
                    })
                    .collect();
                Value::StructObject {
                    span: ByteSpan::default(),
                    struct_def: struct_def.clone(),
                    fields: nf,
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
            // Substitute the callee and all arguments so that nested
            // generic calls inside monomorphized code get rewritten.
            let new_callee = apply_subst_to_value(&callee.borrow(), subst);
            let new_positional: ThinVec<ValueId> = args
                .positional
                .iter()
                .map(|a| ValueId::from(apply_subst_to_value(&a.borrow(), subst)))
                .collect();
            let new_named: ThinVec<(NString, ValueId)> = args
                .named
                .iter()
                .map(|(n, a)| (n.clone(), ValueId::from(apply_subst_to_value(&a.borrow(), subst))))
                .collect();
            Value::Call {
                span: ByteSpan::default(),
                callee: ValueId::from(new_callee),
                args: Arguments {
                    positional: new_positional,
                    named: new_named,
                },
            }
        }
        Value::FunctionSymbol { id, .. } => Value::FunctionSymbol {
            span: ByteSpan::default(),
            id: id.clone(),
        },
        Value::Block { block, .. } => {
            // Substitute all block elements so that nested generics within
            // block expressions inside monomorphized code are rewritten.
            let new_elements: Vec<BlockElement> = block
                .borrow()
                .elements
                .iter()
                .map(|el| clone_block_element(el, subst))
                .collect();
            let new_block = nitrate_hir::Block {
                span: block.borrow().span,
                safety: block.borrow().safety.clone(),
                elements: new_elements,
            };
            Value::Block {
                span: ByteSpan::default(),
                block: nitrate_hir::BlockId::from(new_block),
            }
        }
        val => val.clone(),
    }
}
