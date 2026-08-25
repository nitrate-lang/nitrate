// Add test helper function that creates a minimal function in the symbol table
use crate::substitution::Substitution;
use nitrate_hir::{Arguments, BlockElement, LocalVariable, LocalVariableId, Type, TypeId, Value, ValueId};
use nitrate_nstring::NString;
use nitrate_tree::SrcPos;
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
                .generic_mapping
                .entry(*index)
                .or_insert_with(|| TypeId::from(concrete.clone()));
        }
        (concrete, Type::Inferred { id, .. }) => {
            subst
                .inferred_mapping
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
                .generic_mapping
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
            for (a_k, a_v) in a_args.named.iter() {
                if let Some((_, p_v)) = p_args.named.iter().find(|(k, _)| k == a_k) {
                    unify_types_with_subst(a_v, p_v, subst);
                }
            }
        }
        _ => {}
    }
}

/// Whether every binding in the substitution maps to a fully concrete type.
/// A substitution that still maps some generic parameter to another generic
/// parameter (or to an inference variable) must not be used to monomorphize:
/// doing so would fabricate a "concrete" copy that still contains abstract
/// type variables, corrupting downstream layout and codegen.
pub(crate) fn substitution_is_concrete(subst: &Substitution) -> bool {
    subst
        .generic_mapping
        .values()
        .all(|t| !type_contains_any_generic_param(t) && !t.is_inferred())
}

/// Default an inference-placeholder literal to its canonical type for the
/// purpose of generic unification. An otherwise-unconstrained integer literal
/// defaults to `I32` and a float literal to `F64` — matching what
/// `finalize_inferred_literals` does for literals that end the solve with no
/// constraints. This lets `identity(42)` drive `T := i32` rather than stalling
/// on an unresolved `InferredInteger`.
pub(crate) fn default_inferred_literal(ty: &Type) -> Type {
    match ty {
        Type::InferredInteger { span } => Type::I32 { span: *span },
        Type::InferredFloat { span } => Type::F64 { span: *span },
        _ => ty.clone(),
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

// ── clone_block_element ─────────────────────────────────────────────

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

// ── apply_subst_to_value and helpers ────────────────────────────────

/// Recurses into a borrowed value and applies substitution.
fn recurse(value: &ValueId, subst: &Substitution) -> ValueId {
    ValueId::from(apply_subst_to_value(&value.borrow(), subst))
}

/// Applies substitution to a collection of values.
fn recurse_elements(elements: &ThinVec<ValueId>, subst: &Substitution) -> ThinVec<ValueId> {
    elements.iter().map(|e| recurse(e, subst)).collect()
}

/// Clones a block with substitution applied to its elements.
fn clone_block_with_subst(block: &nitrate_hir::BlockId, subst: &Substitution) -> nitrate_hir::Block {
    let b = block.borrow();
    let new_elements: Vec<BlockElement> = b.elements.iter().map(|el| clone_block_element(el, subst)).collect();
    nitrate_hir::Block {
        span: b.span,
        safety: b.safety.clone(),
        elements: new_elements,
    }
}

/// Applies substitution to Arguments, producing new Arguments.
fn apply_subst_to_args(args: &Arguments<ValueId>, subst: &Substitution) -> Arguments<ValueId> {
    Arguments {
        positional: recurse_elements(&args.positional, subst),
        named: args.named.iter().map(|(n, a)| (n.clone(), recurse(a, subst))).collect(),
    }
}

pub(crate) fn apply_subst_to_value(value: &Value, subst: &Substitution) -> Value {
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
        | Value::InferredFloat { .. }
        | Value::FunctionSymbol { .. }
        | Value::GlobalVariableSymbol { .. }
        | Value::LocalVariableSymbol { .. }
        | Value::ParameterSymbol { .. }
        | Value::Break { .. }
        | Value::Continue { .. } => value.clone(),

        Value::Cast {
            value: v, target_type, ..
        } => Value::Cast {
            span: SrcPos::default(),
            value: recurse(v, subst),
            target_type: TypeId::from(subst.apply(target_type)),
        },

        Value::StructObject { struct_def, fields, .. } => {
            let nf = apply_subst_to_struct_fields(struct_def, fields, subst);
            Value::StructObject {
                span: SrcPos::default(),
                struct_def: struct_def.clone(),
                fields: nf,
            }
        }

        Value::EnumVariant {
            span: _,
            enum_def,
            variant,
            value: inner,
        } => Value::EnumVariant {
            span: SrcPos::default(),
            enum_def: enum_def.clone(),
            variant: variant.clone(),
            value: recurse(inner, subst),
        },

        Value::Binary {
            span: _,
            left,
            op,
            right,
        } => Value::Binary {
            span: SrcPos::default(),
            left: recurse(left, subst),
            op: op.clone(),
            right: recurse(right, subst),
        },

        Value::Unary { span: _, op, operand } => Value::Unary {
            span: SrcPos::default(),
            op: op.clone(),
            operand: recurse(operand, subst),
        },

        Value::IndexAccess {
            span: _,
            collection,
            index,
        } => Value::IndexAccess {
            span: SrcPos::default(),
            collection: recurse(collection, subst),
            index: recurse(index, subst),
        },

        Value::FieldAccess {
            span: _,
            expr,
            field_name,
        } => Value::FieldAccess {
            span: SrcPos::default(),
            expr: recurse(expr, subst),
            field_name: field_name.clone(),
        },

        Value::Assign {
            span: _,
            place,
            value: val,
        } => Value::Assign {
            span: SrcPos::default(),
            place: recurse(place, subst),
            value: recurse(val, subst),
        },

        Value::Deref { span: _, place } => Value::Deref {
            span: SrcPos::default(),
            place: recurse(place, subst),
        },

        Value::Borrow {
            span: _,
            exclusive,
            mutable,
            place,
        } => Value::Borrow {
            span: SrcPos::default(),
            exclusive: *exclusive,
            mutable: *mutable,
            place: recurse(place, subst),
        },

        Value::List { span: _, elements } => Value::List {
            span: SrcPos::default(),
            elements: recurse_elements(elements, subst),
        },

        Value::Tuple { span: _, elements } => Value::Tuple {
            span: SrcPos::default(),
            elements: recurse_elements(elements, subst),
        },

        Value::If {
            span: _,
            condition,
            true_branch,
            false_branch,
        } => Value::If {
            span: SrcPos::default(),
            condition: recurse(condition, subst),
            true_branch: nitrate_hir::BlockId::from(clone_block_with_subst(true_branch, subst)),
            false_branch: false_branch
                .as_ref()
                .map(|fb| nitrate_hir::BlockId::from(clone_block_with_subst(fb, subst))),
        },

        Value::While {
            span: _,
            condition,
            body,
        } => Value::While {
            span: SrcPos::default(),
            condition: recurse(condition, subst),
            body: nitrate_hir::BlockId::from(clone_block_with_subst(body, subst)),
        },

        Value::Loop { span: _, body } => Value::Loop {
            span: SrcPos::default(),
            body: nitrate_hir::BlockId::from(clone_block_with_subst(body, subst)),
        },

        Value::Return { span: _, value: val } => Value::Return {
            span: SrcPos::default(),
            value: recurse(val, subst),
        },

        Value::Block { block, .. } => Value::Block {
            span: SrcPos::default(),
            block: nitrate_hir::BlockId::from(clone_block_with_subst(block, subst)),
        },

        Value::Call { callee, args, type_args, .. } => Value::Call {
            span: SrcPos::default(),
            callee: recurse(callee, subst),
            args: apply_subst_to_args(args, subst),
            type_args: Arguments {
                positional: type_args
                    .positional
                    .iter()
                    .map(|t| TypeId::from(subst.apply(t)))
                    .collect(),
                named: type_args
                    .named
                    .iter()
                    .map(|(k, v)| (k.clone(), TypeId::from(subst.apply(v))))
                    .collect(),
            },
        },

        Value::MethodCall {
            span: _,
            object,
            method_name,
            args,
        } => Value::MethodCall {
            span: SrcPos::default(),
            object: recurse(object, subst),
            method_name: method_name.clone(),
            args: apply_subst_to_args(args, subst),
        },

        Value::Range {
            span: _,
            start,
            end,
            inclusive,
        } => Value::Range {
            span: SrcPos::default(),
            start: start.as_ref().map(|s| recurse(s, subst)),
            end: end.as_ref().map(|e| recurse(e, subst)),
            inclusive: *inclusive,
        },
    }
}

fn apply_subst_to_struct_fields(
    struct_def: &nitrate_hir::StructDefId,
    fields: &ThinVec<(NString, ValueId)>,
    subst: &Substitution,
) -> ThinVec<(NString, ValueId)> {
    let has_generics = struct_def.borrow().generics.is_some();
    fields
        .iter()
        .map(|(n, vid)| {
            let v = vid.borrow();
            let new_v = if has_generics {
                apply_subst_to_value(&v, subst)
            } else {
                v.clone()
            };
            (n.clone(), ValueId::from(new_v))
        })
        .collect()
}

// ── Tests ────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use nitrate_hir::{
        Arguments, BlockId, Function, FunctionId, Lifetime, Store, Type, TypeId, Value, ValueId, Visibility,
        using_storage,
    };
    use nitrate_nstring::NString;
    use nitrate_tree::SrcPos;

    fn sp() -> SrcPos {
        SrcPos::default()
    }

    fn with_store<R>(f: impl FnOnce() -> R) -> R {
        let store = Store::new();
        using_storage(&store, f)
    }

    // ── MonoCacheKey ─────────────────────────────────────────────

    #[test]
    fn mono_cache_key_same_inputs_equal() {
        with_store(|| {
            let k1 = MonoCacheKey::new(42, &[(1, TypeId::from(Type::I32 { span: sp() }))]);
            let k2 = MonoCacheKey::new(42, &[(1, TypeId::from(Type::I32 { span: sp() }))]);
            assert_eq!(k1, k2);
        });
    }

    #[test]
    fn mono_cache_key_different_id_not_equal() {
        with_store(|| {
            let k1 = MonoCacheKey::new(1, &[(0, TypeId::from(Type::I32 { span: sp() }))]);
            let k2 = MonoCacheKey::new(2, &[(0, TypeId::from(Type::I32 { span: sp() }))]);
            assert_ne!(k1, k2);
        });
    }

    #[test]
    fn mono_cache_key_different_subst_not_equal() {
        with_store(|| {
            let k1 = MonoCacheKey::new(1, &[(0, TypeId::from(Type::I32 { span: sp() }))]);
            let k2 = MonoCacheKey::new(1, &[(0, TypeId::from(Type::U8 { span: sp() }))]);
            assert_ne!(k1, k2);
        });
    }

    #[test]
    fn mono_cache_key_empty_subst() {
        let k = MonoCacheKey::new(100, &[]);
        assert_eq!(k, MonoCacheKey::new(100, &[]));
    }

    // ── unify_types_with_subst ───────────────────────────────────

    #[test]
    fn unify_generic_param_with_concrete() {
        with_store(|| {
            let t = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let concrete = Type::I32 { span: sp() };
            let mut subst = Substitution::default();
            unify_types_with_subst(&concrete, &t, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_inferred_with_concrete() {
        with_store(|| {
            let concrete = Type::I32 { span: sp() };
            let inferred = Type::Inferred {
                span: sp(),
                id: std::num::NonZeroU32::new(5).unwrap(),
                name: None,
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&concrete, &inferred, &mut subst);
            assert_eq!(
                *subst.inferred_mapping.get(&5).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_pointer_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let ptr_param = Type::Pointer {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(inner),
            };
            let ptr_conc = Type::Pointer {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::I32 { span: sp() }),
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&ptr_conc, &ptr_param, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_slice_ptr_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let sp_param = Type::SlicePtr {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(inner),
            };
            let sp_conc = Type::SlicePtr {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::I32 { span: sp() }),
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&sp_conc, &sp_param, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_slice_ref_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let sr_param = Type::SliceRef {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(inner),
            };
            let sr_conc = Type::SliceRef {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::F64 { span: sp() }),
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&sr_conc, &sr_param, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::F64 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_reference_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let ref_param = Type::Reference {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(inner),
            };
            let ref_conc = Type::Reference {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::I32 { span: sp() }),
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&ref_conc, &ref_param, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_array_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let arr_param = Type::Array {
                span: sp(),
                element_type: TypeId::from(inner),
                len: 10,
            };
            let arr_conc = Type::Array {
                span: sp(),
                element_type: TypeId::from(Type::I32 { span: sp() }),
                len: 10,
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&arr_conc, &arr_param, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_tuple_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let tuple_param = Type::Tuple {
                span: sp(),
                element_types: vec![TypeId::from(inner)].into(),
            };
            let tuple_conc = Type::Tuple {
                span: sp(),
                element_types: vec![TypeId::from(Type::I32 { span: sp() })].into(),
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&tuple_conc, &tuple_param, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_function_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let func_param = Type::Function {
                span: sp(),
                function_type: Box::new(nitrate_hir::FunctionType {
                    attributes: Default::default(),
                    params: vec![(NString::from("x"), TypeId::from(inner.clone()))].into(),
                    return_type: TypeId::from(inner),
                }),
            };
            let func_conc = Type::Function {
                span: sp(),
                function_type: Box::new(nitrate_hir::FunctionType {
                    attributes: Default::default(),
                    params: vec![(NString::from("x"), TypeId::from(Type::I32 { span: sp() }))].into(),
                    return_type: TypeId::from(Type::I32 { span: sp() }),
                }),
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&func_conc, &func_param, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_reverse_generic_param() {
        with_store(|| {
            let t = Type::GenericParam {
                span: sp(),
                index: 1,
                name: NString::from("T"),
            };
            let concrete = Type::I32 { span: sp() };
            let mut subst = Substitution::default();
            unify_types_with_subst(&t, &concrete, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&1).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_parameterized_recurses() {
        with_store(|| {
            let inner = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let param_ty = Type::Parameterized {
                span: sp(),
                base: TypeId::from(inner.clone()),
                args: Arguments {
                    positional: vec![TypeId::from(inner.clone())].into(),
                    named: vec![(NString::from("X"), TypeId::from(inner))].into(),
                },
            };
            let conc_ty = Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::I32 { span: sp() }),
                args: Arguments {
                    positional: vec![TypeId::from(Type::I32 { span: sp() })].into(),
                    named: vec![(NString::from("X"), TypeId::from(Type::I32 { span: sp() }))].into(),
                },
            };
            let mut subst = Substitution::default();
            unify_types_with_subst(&conc_ty, &param_ty, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_no_match_returns_none() {
        with_store(|| {
            let t1 = Type::Bool { span: sp() };
            let t2 = Type::Unit { span: sp() };
            let mut subst = Substitution::default();
            unify_types_with_subst(&t1, &t2, &mut subst);
            assert!(subst.generic_mapping.is_empty());
            assert!(subst.inferred_mapping.is_empty());
        });
    }

    // ── type_contains_any_generic_param coverage ──────────────

    #[test]
    fn type_contains_generic_param_function() {
        with_store(|| {
            let ft = Type::Function {
                span: sp(),
                function_type: Box::new(nitrate_hir::FunctionType {
                    attributes: Default::default(),
                    params: vec![(
                        NString::from("x"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 0,
                            name: NString::from("T"),
                        }),
                    )]
                    .into(),
                    return_type: TypeId::from(Type::I32 { span: sp() }),
                }),
            };
            assert!(type_contains_any_generic_param(&ft));
        });
    }

    #[test]
    fn type_contains_generic_param_parameterized() {
        with_store(|| {
            let pt = Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::I32 { span: sp() }),
                args: Arguments {
                    positional: vec![TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 0,
                        name: NString::from("T"),
                    })]
                    .into(),
                    named: vec![].into(),
                },
            };
            assert!(type_contains_any_generic_param(&pt));
        });
    }

    #[test]
    fn type_contains_generic_param_refine() {
        with_store(|| {
            let rt = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                min: nitrate_hir::LiteralId::from(nitrate_hir::Lit::I8(0)),
                max: nitrate_hir::LiteralId::from(nitrate_hir::Lit::I8(100)),
            };
            assert!(type_contains_any_generic_param(&rt));
        });
    }

    #[test]
    fn type_contains_generic_param_slice_ref() {
        with_store(|| {
            let st = Type::SliceRef {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            assert!(type_contains_any_generic_param(&st));
        });
    }

    #[test]
    fn type_contains_generic_param_slice_ptr() {
        with_store(|| {
            let st = Type::SlicePtr {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            assert!(type_contains_any_generic_param(&st));
        });
    }

    #[test]
    fn type_contains_generic_param_reference() {
        with_store(|| {
            let rt = Type::Reference {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            assert!(type_contains_any_generic_param(&rt));
        });
    }

    #[test]
    fn type_contains_generic_param_pointer() {
        with_store(|| {
            let pt = Type::Pointer {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            assert!(type_contains_any_generic_param(&pt));
        });
    }

    #[test]
    fn type_contains_generic_param_array() {
        with_store(|| {
            let at = Type::Array {
                span: sp(),
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                len: 5,
            };
            assert!(type_contains_any_generic_param(&at));
        });
    }

    #[test]
    fn type_contains_generic_param_tuple() {
        with_store(|| {
            let tt = Type::Tuple {
                span: sp(),
                element_types: vec![TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                })]
                .into(),
            };
            assert!(type_contains_any_generic_param(&tt));
        });
    }

    #[test]
    fn type_contains_generic_param_named_in_parameterized() {
        with_store(|| {
            let pt = Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::I32 { span: sp() }),
                args: Arguments {
                    positional: vec![].into(),
                    named: vec![(
                        NString::from("X"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 0,
                            name: NString::from("T"),
                        }),
                    )]
                    .into(),
                },
            };
            assert!(type_contains_any_generic_param(&pt));
        });
    }

    // ── type_contains_generic_param_name coverage ─────────────

    #[test]
    fn type_contains_generic_param_name_function() {
        with_store(|| {
            let ft = Type::Function {
                span: sp(),
                function_type: Box::new(nitrate_hir::FunctionType {
                    attributes: Default::default(),
                    params: vec![(
                        NString::from("x"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 0,
                            name: NString::from("T"),
                        }),
                    )]
                    .into(),
                    return_type: TypeId::from(Type::I32 { span: sp() }),
                }),
            };
            let tid = TypeId::from(ft);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_parameterized() {
        with_store(|| {
            let pt = Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::I32 { span: sp() }),
                args: Arguments {
                    positional: vec![TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 0,
                        name: NString::from("T"),
                    })]
                    .into(),
                    named: vec![(
                        NString::from("X"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 1,
                            name: NString::from("U"),
                        }),
                    )]
                    .into(),
                },
            };
            let tid = TypeId::from(pt);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
            assert!(type_contains_generic_param_name(&tid, &NString::from("U")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_refine() {
        with_store(|| {
            let rt = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                min: nitrate_hir::LiteralId::from(nitrate_hir::Lit::I8(0)),
                max: nitrate_hir::LiteralId::from(nitrate_hir::Lit::I8(100)),
            };
            let tid = TypeId::from(rt);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_slice_ref() {
        with_store(|| {
            let st = Type::SliceRef {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let tid = TypeId::from(st);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_slice_ptr() {
        with_store(|| {
            let st = Type::SlicePtr {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let tid = TypeId::from(st);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_array() {
        with_store(|| {
            let at = Type::Array {
                span: sp(),
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                len: 5,
            };
            let tid = TypeId::from(at);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_tuple() {
        with_store(|| {
            let tt = Type::Tuple {
                span: sp(),
                element_types: vec![TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                })]
                .into(),
            };
            let tid = TypeId::from(tt);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_reference() {
        with_store(|| {
            let rt = Type::Reference {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let tid = TypeId::from(rt);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    #[test]
    fn type_contains_generic_param_name_pointer() {
        with_store(|| {
            let pt = Type::Pointer {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let tid = TypeId::from(pt);
            assert!(type_contains_generic_param_name(&tid, &NString::from("T")));
        });
    }

    // ── collect_generic_params_from_type coverage ─────────────

    #[test]
    fn collect_generic_params_from_function() {
        with_store(|| {
            let ft = Type::Function {
                span: sp(),
                function_type: Box::new(nitrate_hir::FunctionType {
                    attributes: Default::default(),
                    params: vec![(
                        NString::from("x"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 0,
                            name: NString::from("T"),
                        }),
                    )]
                    .into(),
                    return_type: TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 1,
                        name: NString::from("U"),
                    }),
                }),
            };
            let tid = TypeId::from(ft);
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&tid, &mut map);
            assert_eq!(map.get(&NString::from("T")), Some(&0));
            assert_eq!(map.get(&NString::from("U")), Some(&1));
        });
    }

    #[test]
    fn collect_generic_params_from_parameterized() {
        with_store(|| {
            let pt = Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                args: Arguments {
                    positional: vec![TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 1,
                        name: NString::from("U"),
                    })]
                    .into(),
                    named: vec![].into(),
                },
            };
            let tid = TypeId::from(pt);
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&tid, &mut map);
            assert_eq!(map.get(&NString::from("T")), Some(&0));
            assert_eq!(map.get(&NString::from("U")), Some(&1));
        });
    }

    #[test]
    fn collect_generic_params_from_parameterized_named() {
        with_store(|| {
            let pt = Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::I32 { span: sp() }),
                args: Arguments {
                    positional: vec![].into(),
                    named: vec![(
                        NString::from("X"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 0,
                            name: NString::from("T"),
                        }),
                    )]
                    .into(),
                },
            };
            let tid = TypeId::from(pt);
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&tid, &mut map);
            assert_eq!(map.get(&NString::from("T")), Some(&0));
        });
    }

    #[test]
    fn collect_generic_params_from_refine() {
        with_store(|| {
            let rt = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                min: nitrate_hir::LiteralId::from(nitrate_hir::Lit::I8(0)),
                max: nitrate_hir::LiteralId::from(nitrate_hir::Lit::I8(100)),
            };
            let tid = TypeId::from(rt);
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&tid, &mut map);
            assert_eq!(map.get(&NString::from("T")), Some(&0));
        });
    }

    // ── clone_block_element coverage ──────────────────────────

    #[test]
    fn clone_block_element_expr() {
        with_store(|| {
            let vid = ValueId::from(Value::I32 { span: sp(), value: 42 });
            let elem = BlockElement::Expr(vid);
            let subst = Substitution::default();
            let result = clone_block_element(&elem, &subst);
            assert!(matches!(result, BlockElement::Expr(_)));
        });
    }

    #[test]
    fn clone_block_element_local() {
        with_store(|| {
            let lv = LocalVariableId::from(LocalVariable {
                span: sp(),
                kind: nitrate_hir::LocalKind::Let,
                attributes: Default::default(),
                is_mutable: false,
                name: NString::from("x"),
                ty: TypeId::from(Type::I32 { span: sp() }),
                initializer: None,
            });
            let elem = BlockElement::Local(lv);
            let subst = Substitution::default();
            let result = clone_block_element(&elem, &subst);
            assert!(matches!(result, BlockElement::Local(_)));
        });
    }

    // ── apply_subst_to_struct_fields ──────────────────────────

    #[test]
    fn apply_subst_to_struct_fields_no_generics() {
        with_store(|| {
            let sd = nitrate_hir::StructDefId::from(nitrate_hir::StructDef {
                span: sp(),
                visibility: Visibility::Sec,
                name: NString::from("NoGen"),
                attributes: Default::default(),
                fields: BTreeMap::new(),
                generics: None,
                layout: ThinVec::new(),
            });
            let fields: ThinVec<(NString, ValueId)> =
                vec![(NString::from("x"), ValueId::from(Value::I32 { span: sp(), value: 10 }))].into();
            let subst = Substitution::default();
            let result = apply_subst_to_struct_fields(&sd, &fields.clone(), &subst);
            assert_eq!(result.len(), 1);
        });
    }

    #[test]
    fn apply_subst_to_struct_fields_with_generics() {
        with_store(|| {
            let mut generics = BTreeMap::new();
            generics.insert(NString::from("T"), Some(TypeId::from(Type::I32 { span: sp() })));
            let sd = nitrate_hir::StructDefId::from(nitrate_hir::StructDef {
                span: sp(),
                visibility: Visibility::Sec,
                name: NString::from("HasGen"),
                attributes: Default::default(),
                fields: BTreeMap::new(),
                generics: Some(generics),
                layout: ThinVec::new(),
            });
            let fields: ThinVec<(NString, ValueId)> =
                vec![(NString::from("x"), ValueId::from(Value::I32 { span: sp(), value: 10 }))].into();
            let subst = Substitution::default();
            let result = apply_subst_to_struct_fields(&sd, &fields.clone(), &subst);
            assert_eq!(result.len(), 1);
        });
    }

    // ── apply_subst_to_value full coverage ───────────────────

    #[test]
    fn apply_subst_cast() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let v = Value::Cast {
                span: sp(),
                value: ValueId::from(Value::I32 { span: sp(), value: 1 }),
                target_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let result = apply_subst_to_value(&v, &s);
            match result {
                Value::Cast { target_type, .. } => {
                    assert_eq!(*target_type, Type::I32 { span: sp() });
                }
                _ => panic!("expected Cast"),
            }
        });
    }

    #[test]
    fn apply_subst_struct_object() {
        with_store(|| {
            let s = Substitution::default();
            let sd = nitrate_hir::StructDefId::from(nitrate_hir::StructDef {
                span: sp(),
                visibility: Visibility::Sec,
                name: NString::from("S"),
                attributes: Default::default(),
                fields: BTreeMap::new(),
                generics: None,
                layout: ThinVec::new(),
            });
            let v = Value::StructObject {
                span: sp(),
                struct_def: sd.clone(),
                fields: vec![(NString::from("x"), ValueId::from(Value::I32 { span: sp(), value: 1 }))].into(),
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::StructObject { .. }));
        });
    }

    #[test]
    fn apply_subst_enum_variant() {
        with_store(|| {
            let s = Substitution::default();
            let ed = nitrate_hir::EnumDefId::from(nitrate_hir::EnumDef {
                span: sp(),
                visibility: Visibility::Sec,
                name: NString::from("E"),
                attributes: Default::default(),
                variants: ThinVec::new(),
                generics: None,
            });
            let v = Value::EnumVariant {
                span: sp(),
                enum_def: ed,
                variant: NString::from("V"),
                value: ValueId::from(Value::Unit { span: sp() }),
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::EnumVariant { .. }));
        });
    }

    #[test]
    fn apply_subst_index_access() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::IndexAccess {
                span: sp(),
                collection: ValueId::from(Value::Unit { span: sp() }),
                index: ValueId::from(Value::I32 { span: sp(), value: 0 }),
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::IndexAccess { .. }));
        });
    }

    #[test]
    fn apply_subst_field_access() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::FieldAccess {
                span: sp(),
                expr: ValueId::from(Value::Unit { span: sp() }),
                field_name: NString::from("x"),
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::FieldAccess { .. }));
        });
    }

    #[test]
    fn apply_subst_assign() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::Assign {
                span: sp(),
                place: ValueId::from(Value::Unit { span: sp() }),
                value: ValueId::from(Value::I32 { span: sp(), value: 1 }),
            };
            let result = apply_subst_to_value(&v, &s);
            if let Value::Assign { value, .. } = result {
                assert_eq!(*value.borrow(), Value::I32 { span: sp(), value: 1 });
            } else {
                panic!("expected Assign");
            }
        });
    }

    #[test]
    fn apply_subst_deref() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::Deref {
                span: sp(),
                place: ValueId::from(Value::I32 { span: sp(), value: 1 }),
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::Deref { .. }));
        });
    }

    #[test]
    fn apply_subst_borrow() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::Borrow {
                span: sp(),
                exclusive: true,
                mutable: true,
                place: ValueId::from(Value::I8 { span: sp(), value: 42 }),
            };
            let result = apply_subst_to_value(&v, &s);
            if let Value::Borrow { exclusive, mutable, .. } = result {
                assert!(exclusive);
                assert!(mutable);
            } else {
                panic!("expected Borrow");
            }
        });
    }

    #[test]
    fn apply_subst_list() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::List {
                span: sp(),
                elements: vec![
                    ValueId::from(Value::I32 { span: sp(), value: 1 }),
                    ValueId::from(Value::I32 { span: sp(), value: 2 }),
                ]
                .into(),
            };
            let result = apply_subst_to_value(&v, &s);
            if let Value::List { elements, .. } = result {
                assert_eq!(elements.len(), 2);
            } else {
                panic!("expected List");
            }
        });
    }

    #[test]
    fn apply_subst_tuple_value() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::Tuple {
                span: sp(),
                elements: vec![
                    ValueId::from(Value::I32 { span: sp(), value: 1 }),
                    ValueId::from(Value::I32 { span: sp(), value: 2 }),
                ]
                .into(),
            };
            let result = apply_subst_to_value(&v, &s);
            if let Value::Tuple { elements, .. } = result {
                assert_eq!(elements.len(), 2);
            } else {
                panic!("expected Tuple");
            }
        });
    }

    #[test]
    fn apply_subst_if() {
        with_store(|| {
            let s = Substitution::default();
            let inner = BlockId::from(nitrate_hir::Block {
                span: sp(),
                safety: nitrate_hir::BlockSafety::Safe,
                elements: vec![BlockElement::Expr(ValueId::from(Value::Unit { span: sp() }))],
            });
            let v = Value::If {
                span: sp(),
                condition: ValueId::from(Value::Bool {
                    span: sp(),
                    value: true,
                }),
                true_branch: inner.clone(),
                false_branch: Some(inner),
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::If { .. }));
        });
    }

    #[test]
    fn apply_subst_while() {
        with_store(|| {
            let s = Substitution::default();
            let body = BlockId::from(nitrate_hir::Block {
                span: sp(),
                safety: nitrate_hir::BlockSafety::Safe,
                elements: vec![],
            });
            let v = Value::While {
                span: sp(),
                condition: ValueId::from(Value::Bool {
                    span: sp(),
                    value: true,
                }),
                body,
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::While { .. }));
        });
    }

    #[test]
    fn apply_subst_loop() {
        with_store(|| {
            let s = Substitution::default();
            let body = BlockId::from(nitrate_hir::Block {
                span: sp(),
                safety: nitrate_hir::BlockSafety::Safe,
                elements: vec![],
            });
            let v = Value::Loop { span: sp(), body };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::Loop { .. }));
        });
    }

    #[test]
    fn apply_subst_return_value() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::Return {
                span: sp(),
                value: ValueId::from(Value::I32 { span: sp(), value: 42 }),
            };
            let result = apply_subst_to_value(&v, &s);
            if let Value::Return { value, .. } = result {
                assert_eq!(*value.borrow(), Value::I32 { span: sp(), value: 42 });
            } else {
                panic!("expected Return");
            }
        });
    }

    #[test]
    fn apply_subst_block_value() {
        with_store(|| {
            let s = Substitution::default();
            let inner = BlockId::from(nitrate_hir::Block {
                span: sp(),
                safety: nitrate_hir::BlockSafety::Safe,
                elements: vec![BlockElement::Expr(ValueId::from(Value::Unit { span: sp() }))],
            });
            let v = Value::Block {
                span: sp(),
                block: inner,
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::Block { .. }));
        });
    }

    #[test]
    fn apply_subst_call() {
        with_store(|| {
            let s = Substitution::default();
            let fid = FunctionId::from(Function {
                span: sp(),
                visibility: Visibility::Sec,
                attributes: Default::default(),
                is_unsafe: false,
                name: NString::from("test_func"),
                mangled_name: None,
                generics: None,
                params: vec![],
                return_type: TypeId::from(Type::Unit { span: sp() }),
                body: None,
            });
            let v = Value::Call {
                span: sp(),
                callee: ValueId::from(Value::FunctionSymbol { span: sp(), id: fid }),
                args: Arguments {
                    positional: vec![].into(),
                    named: vec![].into(),
                },
                type_args: Arguments::default(),
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::Call { .. }));
        });
    }

    #[test]
    fn apply_subst_method_call() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::MethodCall {
                span: sp(),
                object: ValueId::from(Value::Unit { span: sp() }),
                method_name: NString::from("foo"),
                args: Arguments {
                    positional: vec![].into(),
                    named: vec![].into(),
                },
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::MethodCall { .. }));
        });
    }

    #[test]
    fn apply_subst_range() {
        with_store(|| {
            let s = Substitution::default();
            let v = Value::Range {
                span: sp(),
                start: Some(ValueId::from(Value::I32 { span: sp(), value: 1 })),
                end: Some(ValueId::from(Value::I32 { span: sp(), value: 10 })),
                inclusive: false,
            };
            let result = apply_subst_to_value(&v, &s);
            assert!(matches!(result, Value::Range { .. }));
        });
    }
}
