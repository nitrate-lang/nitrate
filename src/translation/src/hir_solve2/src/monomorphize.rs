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
        // ── Leaves (no children to recurse into) ──────────────────
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

        // ── Cast ──────────────────────────────────────────────────
        Value::Cast {
            value: v, target_type, ..
        } => Value::Cast {
            span: SrcPos::default(),
            value: recurse(v, subst),
            target_type: TypeId::from(subst.apply(target_type)),
        },

        // ── StructObject ──────────────────────────────────────────
        Value::StructObject { struct_def, fields, .. } => {
            let nf = apply_subst_to_struct_fields(struct_def, fields, subst);
            Value::StructObject {
                span: SrcPos::default(),
                struct_def: struct_def.clone(),
                fields: nf,
            }
        }

        // ── EnumVariant ───────────────────────────────────────────
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

        // ── Binary ────────────────────────────────────────────────
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

        // ── Unary ─────────────────────────────────────────────────
        Value::Unary { span: _, op, operand } => Value::Unary {
            span: SrcPos::default(),
            op: op.clone(),
            operand: recurse(operand, subst),
        },

        // ── IndexAccess ───────────────────────────────────────────
        Value::IndexAccess {
            span: _,
            collection,
            index,
        } => Value::IndexAccess {
            span: SrcPos::default(),
            collection: recurse(collection, subst),
            index: recurse(index, subst),
        },

        // ── FieldAccess ───────────────────────────────────────────
        Value::FieldAccess {
            span: _,
            expr,
            field_name,
        } => Value::FieldAccess {
            span: SrcPos::default(),
            expr: recurse(expr, subst),
            field_name: field_name.clone(),
        },

        // ── Assign ────────────────────────────────────────────────
        Value::Assign {
            span: _,
            place,
            value: val,
        } => Value::Assign {
            span: SrcPos::default(),
            place: recurse(place, subst),
            value: recurse(val, subst),
        },

        // ── Deref ─────────────────────────────────────────────────
        Value::Deref { span: _, place } => Value::Deref {
            span: SrcPos::default(),
            place: recurse(place, subst),
        },

        // ── Borrow ────────────────────────────────────────────────
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

        // ── List ──────────────────────────────────────────────────
        Value::List { span: _, elements } => Value::List {
            span: SrcPos::default(),
            elements: recurse_elements(elements, subst),
        },

        // ── Tuple ─────────────────────────────────────────────────
        Value::Tuple { span: _, elements } => Value::Tuple {
            span: SrcPos::default(),
            elements: recurse_elements(elements, subst),
        },

        // ── If ────────────────────────────────────────────────────
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

        // ── While ─────────────────────────────────────────────────
        Value::While {
            span: _,
            condition,
            body,
        } => Value::While {
            span: SrcPos::default(),
            condition: recurse(condition, subst),
            body: nitrate_hir::BlockId::from(clone_block_with_subst(body, subst)),
        },

        // ── Loop ──────────────────────────────────────────────────
        Value::Loop { span: _, body } => Value::Loop {
            span: SrcPos::default(),
            body: nitrate_hir::BlockId::from(clone_block_with_subst(body, subst)),
        },

        // ── Return ────────────────────────────────────────────────
        Value::Return { span: _, value: val } => Value::Return {
            span: SrcPos::default(),
            value: recurse(val, subst),
        },

        // ── Block ─────────────────────────────────────────────────
        Value::Block { block, .. } => Value::Block {
            span: SrcPos::default(),
            block: nitrate_hir::BlockId::from(clone_block_with_subst(block, subst)),
        },

        // ── Call ──────────────────────────────────────────────────
        Value::Call { callee, args, .. } => Value::Call {
            span: SrcPos::default(),
            callee: recurse(callee, subst),
            args: apply_subst_to_args(args, subst),
        },

        // ── MethodCall ────────────────────────────────────────────
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

        // ── Range ─────────────────────────────────────────────────
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
