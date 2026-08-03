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

        Value::Call { callee, args, .. } => Value::Call {
            span: SrcPos::default(),
            callee: recurse(callee, subst),
            args: apply_subst_to_args(args, subst),
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
        Arguments, FunctionType, Lifetime, Lit, LiteralId, Store, StructDef, StructDefId, Type, TypeId, Value, ValueId,
        using_storage,
    };
    use nitrate_nstring::NString;
    use nitrate_tree::SrcPos;
    use std::collections::BTreeMap;
    use std::num::NonZeroU32;

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
    fn unify_does_not_overwrite_existing() {
        with_store(|| {
            let t = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            let first = Type::I32 { span: sp() };
            let second = Type::U64 { span: sp() };
            let mut subst = Substitution::default();
            unify_types_with_subst(&first, &t, &mut subst);
            unify_types_with_subst(&second, &t, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&0).unwrap(),
                TypeId::from(Type::I32 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_two_concrete_does_nothing() {
        let a = Type::I32 { span: sp() };
        let b = Type::I64 { span: sp() };
        let mut subst = Substitution::default();
        unify_types_with_subst(&a, &b, &mut subst);
        assert!(subst.generic_mapping.is_empty());
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

    // ── type_contains_any_generic_param ──────────────────────────

    #[test]
    fn generic_param_contains_self() {
        let ty = Type::GenericParam {
            span: sp(),
            index: 0,
            name: NString::from("T"),
        };
        assert!(type_contains_any_generic_param(&ty));
    }

    #[test]
    fn concrete_does_not_contain_generic() {
        assert!(!type_contains_any_generic_param(&Type::I32 { span: sp() }));
        assert!(!type_contains_any_generic_param(&Type::Bool { span: sp() }));
    }

    #[test]
    fn pointer_to_generic_contains() {
        with_store(|| {
            let pt = Type::Pointer {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 1,
                    name: NString::from("T"),
                }),
            };
            assert!(type_contains_any_generic_param(&pt));
        });
    }

    #[test]
    fn array_of_generic_contains() {
        with_store(|| {
            let arr = Type::Array {
                span: sp(),
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                len: 10,
            };
            assert!(type_contains_any_generic_param(&arr));
        });
    }

    // ── type_contains_generic_param_name ─────────────────────────

    #[test]
    fn param_name_matches() {
        with_store(|| {
            let name = NString::from("T");
            let ty = TypeId::from(Type::GenericParam {
                span: sp(),
                index: 0,
                name: name.clone(),
            });
            assert!(type_contains_generic_param_name(&ty, &name));
        });
    }

    #[test]
    fn param_name_does_not_match_different() {
        with_store(|| {
            let ty = TypeId::from(Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            });
            assert!(!type_contains_generic_param_name(&ty, &NString::from("U")));
        });
    }

    // ── collect_generic_params_from_type ─────────────────────────

    #[test]
    fn collect_single_param() {
        with_store(|| {
            let ty = TypeId::from(Type::GenericParam {
                span: sp(),
                index: 5,
                name: NString::from("T"),
            });
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&ty, &mut map);
            assert_eq!(map.get(&NString::from("T")), Some(&5u32));
        });
    }

    #[test]
    fn collect_does_not_duplicate() {
        with_store(|| {
            let ty = TypeId::from(Type::GenericParam {
                span: sp(),
                index: 3,
                name: NString::from("T"),
            });
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&ty, &mut map);
            collect_generic_params_from_type(&ty, &mut map);
            assert_eq!(map.len(), 1);
        });
    }

    #[test]
    fn collect_from_concrete_is_empty() {
        with_store(|| {
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&TypeId::from(Type::I32 { span: sp() }), &mut map);
            assert!(map.is_empty());
        });
    }

    // ── apply_subst_to_value (leaves) ────────────────────────────

    #[test]
    fn apply_subst_to_value_leaves_identity() {
        with_store(|| {
            let subst = Substitution::default();
            let cases = [
                Value::Unit { span: sp() },
                Value::Bool {
                    span: sp(),
                    value: true,
                },
                Value::I32 { span: sp(), value: 42 },
            ];
            for v in &cases {
                assert_eq!(apply_subst_to_value(v, &subst), *v);
            }
        });
    }

    #[test]
    fn apply_subst_to_value_binary_recurse() {
        with_store(|| {
            let subst = Substitution::default();
            let bin = Value::Binary {
                span: sp(),
                left: ValueId::from(Value::I32 { span: sp(), value: 1 }),
                op: nitrate_hir::BinaryOp::Add,
                right: ValueId::from(Value::I32 { span: sp(), value: 2 }),
            };
            let result = apply_subst_to_value(&bin, &subst);
            if let Value::Binary { left, .. } = result {
                assert_eq!(*left.borrow(), Value::I32 { span: sp(), value: 1 });
            } else {
                panic!("expected Binary");
            }
        });
    }

    // ── More unify branches ──────────────────────────────────

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
    fn unify_inferred_with_concrete() {
        with_store(|| {
            let inferred = Type::Inferred {
                span: sp(),
                id: NonZeroU32::new(7).unwrap(),
                name: None,
            };
            let concrete = Type::F64 { span: sp() };
            let mut subst = Substitution::default();
            unify_types_with_subst(&concrete, &inferred, &mut subst);
            assert_eq!(
                *subst.inferred_mapping.get(&7).unwrap(),
                TypeId::from(Type::F64 { span: sp() })
            );
        });
    }

    #[test]
    fn unify_generic_param_with_concrete_reversed() {
        with_store(|| {
            let t = Type::GenericParam {
                span: sp(),
                index: 1,
                name: NString::from("U"),
            };
            let concrete = Type::Bool { span: sp() };
            let mut subst = Substitution::default();
            unify_types_with_subst(&t, &concrete, &mut subst);
            assert_eq!(
                *subst.generic_mapping.get(&1).unwrap(),
                TypeId::from(Type::Bool { span: sp() })
            );
        });
    }

    // ── apply_subst_to_value more variants ──────────────────

    #[test]
    fn apply_subst_unary_recurse() {
        with_store(|| {
            let subst = Substitution::default();
            let unary = Value::Unary {
                span: sp(),
                op: nitrate_hir::UnaryOp::Sub,
                operand: ValueId::from(Value::I64 { span: sp(), value: 10 }),
            };
            let result = apply_subst_to_value(&unary, &subst);
            if let Value::Unary { operand, .. } = result {
                assert_eq!(*operand.borrow(), Value::I64 { span: sp(), value: 10 });
            } else {
                panic!("expected Unary");
            }
        });
    }

    #[test]
    fn apply_subst_struct_object_no_generics() {
        with_store(|| {
            let subst = Substitution::default();
            let struct_def = StructDefId::from(StructDef {
                span: sp(),
                visibility: nitrate_hir::Visibility::Sec,
                name: NString::from("Foo"),
                attributes: Default::default(),
                fields: BTreeMap::new(),
                generics: None,
                layout: ThinVec::new(),
            });
            let sv = Value::StructObject {
                span: sp(),
                struct_def: struct_def.clone(),
                fields: vec![(NString::from("x"), ValueId::from(Value::I32 { span: sp(), value: 42 }))].into(),
            };
            let result = apply_subst_to_value(&sv, &subst);
            if let Value::StructObject { struct_def: sd, .. } = result {
                assert_eq!(sd.borrow().name, NString::from("Foo"));
            } else {
                panic!("expected StructObject");
            }
        });
    }

    // ── slice_ref/slice_ptr generic detection ────────────────

    #[test]
    fn slice_ptr_to_generic_contains() {
        with_store(|| {
            let sp = Type::SlicePtr {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: true,
                mutable: true,
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            assert!(type_contains_any_generic_param(&sp));
        });
    }

    #[test]
    fn refine_with_generic_base_contains() {
        with_store(|| {
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                min: LiteralId::from(Lit::I8(0)),
                max: LiteralId::from(Lit::I8(100)),
            };
            assert!(type_contains_any_generic_param(&refine));
        });
    }

    #[test]
    fn param_name_matches_in_array() {
        with_store(|| {
            let name = NString::from("T");
            let ty = TypeId::from(Type::Array {
                span: sp(),
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: name.clone(),
                }),
                len: 4,
            });
            assert!(type_contains_generic_param_name(&ty, &name));
        });
    }

    #[test]
    fn collect_multiple_params_from_tuple() {
        with_store(|| {
            let ty = TypeId::from(Type::Tuple {
                span: sp(),
                element_types: vec![
                    TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 0,
                        name: NString::from("A"),
                    }),
                    TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 1,
                        name: NString::from("B"),
                    }),
                ]
                .into(),
            });
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&ty, &mut map);
            assert_eq!(map.get(&NString::from("A")), Some(&0u32));
            assert_eq!(map.get(&NString::from("B")), Some(&1u32));
        });
    }

    #[test]
    fn collect_from_parameterized_type() {
        with_store(|| {
            let ty = TypeId::from(Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("Base"),
                }),
                args: Arguments {
                    positional: vec![TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 1,
                        name: NString::from("Arg"),
                    })]
                    .into(),
                    named: vec![(
                        NString::from("X"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 2,
                            name: NString::from("Named"),
                        }),
                    )]
                    .into(),
                },
            });
            let mut map = BTreeMap::new();
            collect_generic_params_from_type(&ty, &mut map);
            assert_eq!(map.len(), 3);
        });
    }
}
