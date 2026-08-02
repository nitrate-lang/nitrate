//! Value rewriting pass: resolves inferred literals to concrete types and
//! desugars range expressions into struct objects.
//!
//! This pass runs after constraint solving. It uses the resolved type
//! bindings from the constraint graph to replace `InferredInteger` and
//! `InferredFloat` with concrete integer/float values, following Rust-style
//! defaulting rules:
//!
//! - **Unconstrained integers** default to `i32`
//! - **Unconstrained floats** default to `f64`
//! - When constraints exist, the narrowest type that satisfies all
//!   constraints is chosen

use crate::constraints::{CanonicalType, ConstraintGraph};
use crate::diagnosis::TypeErr;
use crate::walk::NodeTypes;
use nitrate_hir::{
    BlockElement, BlockId, Lit, LiteralId, PtrSize, SymbolTab, Type, TypeId, Value, ValueId, get_storage,
};
use nitrate_tree::ByteSpan;
use ordered_float::OrderedFloat;
use std::collections::HashSet;
use std::vec;

/// Rewrite all inferred literals and range expressions in a function body.
pub(crate) fn rewrite_body(
    body: &mut [BlockElement],
    node_types: &mut NodeTypes,
    graph: &mut ConstraintGraph,
    symbol_tab: &SymbolTab,
    errors: &mut HashSet<TypeErr>,
) {
    for element in body.iter_mut() {
        rewrite_block_element(element, node_types, graph, symbol_tab, errors);
    }
}

fn rewrite_block_element(
    element: &mut BlockElement,
    node_types: &mut NodeTypes,
    graph: &mut ConstraintGraph,
    symbol_tab: &SymbolTab,
    errors: &mut HashSet<TypeErr>,
) {
    match element {
        BlockElement::Expr(expr_id) => {
            rewrite_value(expr_id, node_types, graph, symbol_tab, errors);
        }
        BlockElement::Local(local_var) => {
            let lv = local_var.borrow();
            if let Some(init_id) = &lv.initializer {
                rewrite_value(&init_id.clone(), node_types, graph, symbol_tab, errors);
            }
        }
    }
}

fn rewrite_value(
    id: &ValueId,
    node_types: &mut NodeTypes,
    graph: &mut ConstraintGraph,
    symbol_tab: &SymbolTab,
    errors: &mut HashSet<TypeErr>,
) {
    // First, try to resolve inferred literals at this node.
    let resolved_type = node_types.resolve(id, graph);

    {
        let value = id.borrow();
        let span = value.span();

        match &*value {
            Value::InferredInteger { value: v, .. } => {
                let v = **v;
                let target_ty = if let Some(ty) = resolved_type {
                    ty
                } else {
                    // Rust-style default: unconstrained integers → i32
                    TypeId::from(Type::I32 {
                        span: ByteSpan::default(),
                    })
                };

                resolve_integer_literal(id, v, &target_ty, span, errors);
                return;
            }
            Value::InferredFloat { value: v, .. } => {
                let v = v.0;
                let target_ty = if let Some(ty) = resolved_type {
                    ty
                } else {
                    // Rust-style default: unconstrained floats → f64
                    TypeId::from(Type::F64 {
                        span: ByteSpan::default(),
                    })
                };

                resolve_float_literal(id, v, &target_ty, span, errors);
                return;
            }
            Value::Range {
                span,
                start,
                end,
                inclusive,
            } => {
                // Desugar range into StructObject.
                let has_start = start.is_some();
                let has_end = end.is_some();
                let inclusive = *inclusive;
                let span = *span;

                let new_value = crate::range::make_range_struct_object(
                    symbol_tab,
                    span,
                    start.clone(),
                    end.clone(),
                    inclusive,
                    has_start,
                    has_end,
                );
                drop(value);
                id.replace(new_value);
                return;
            }
            _ => {}
        }
    }

    // Recurse into children.
    let children: Vec<ValueId> = {
        let v = id.borrow();
        match &*v {
            Value::Block { block, .. } => {
                let mut elements = block.borrow_mut().elements.clone();
                for element in elements.iter_mut() {
                    rewrite_block_element(element, node_types, graph, symbol_tab, errors);
                }
                return;
            }
            Value::StructObject { fields, .. } => fields.iter().map(|(_, v)| v.clone()).collect(),
            Value::EnumVariant { value: v, .. } => vec![v.clone()],
            Value::Binary { left, right, .. } => vec![left.clone(), right.clone()],
            Value::Unary { operand, .. } => vec![operand.clone()],
            Value::IndexAccess { collection, index, .. } => vec![collection.clone(), index.clone()],
            Value::FieldAccess { expr, .. } => vec![expr.clone()],
            Value::Assign { place, value: val, .. } => vec![place.clone(), val.clone()],
            Value::Deref { place, .. } => vec![place.clone()],
            Value::Cast { value: val, .. } => vec![val.clone()],
            Value::Borrow { place, .. } => vec![place.clone()],
            Value::List { elements, .. } => elements.iter().cloned().collect(),
            Value::Tuple { elements, .. } => elements.iter().cloned().collect(),
            Value::If {
                condition,
                true_branch,
                false_branch,
                ..
            } => {
                let mut children = vec![condition.clone()];
                rewrite_body(
                    &mut true_branch.borrow_mut().elements,
                    node_types,
                    graph,
                    symbol_tab,
                    errors,
                );
                if let Some(fb) = false_branch {
                    rewrite_body(&mut fb.borrow_mut().elements, node_types, graph, symbol_tab, errors);
                }
                children
            }
            Value::While { condition, body, .. } => {
                rewrite_body(&mut body.borrow_mut().elements, node_types, graph, symbol_tab, errors);
                vec![condition.clone()]
            }
            Value::Loop { body, .. } => {
                rewrite_body(&mut body.borrow_mut().elements, node_types, graph, symbol_tab, errors);
                return;
            }
            Value::Return { value: val, .. } => vec![val.clone()],
            Value::Call { callee, args, .. } => {
                let mut ids = vec![callee.clone()];
                ids.extend(args.positional.iter().cloned());
                ids.extend(args.named.iter().map(|(_, v)| v.clone()));
                ids
            }
            Value::MethodCall { object, args, .. } => {
                let mut ids = vec![object.clone()];
                ids.extend(args.positional.iter().cloned());
                ids.extend(args.named.iter().map(|(_, v)| v.clone()));
                ids
            }
            _ => vec![],
        }
    };

    for child in &children {
        rewrite_value(child, node_types, graph, symbol_tab, errors);
    }
}

/// Resolve an `InferredInteger` to a concrete integer value.
///
/// The target type determines which integer variant to use. If the value
/// doesn't fit in the target type, an error is reported and the literal
/// is left unchanged.
fn resolve_integer_literal(
    id: &ValueId,
    value: u128,
    target_ty: &TypeId,
    span: ByteSpan,
    errors: &mut HashSet<TypeErr>,
) {
    // Strip Refine wrapper to get the base type for literal resolution.
    let effective_ty = match &**target_ty {
        Type::Refine { base, .. } => *base,
        _ => *target_ty,
    };

    let new_value = match &*effective_ty {
        Type::I8 { .. } => match i8::try_from(value) {
            Ok(v) => Value::I8 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::I16 { .. } => match i16::try_from(value) {
            Ok(v) => Value::I16 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::I32 { .. } => match i32::try_from(value) {
            Ok(v) => Value::I32 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::I64 { .. } => match i64::try_from(value) {
            Ok(v) => Value::I64 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::I128 { .. } => match i128::try_from(value) {
            Ok(v) => Value::I128 {
                span,
                value: Box::new(v),
            },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::U8 { .. } => match u8::try_from(value) {
            Ok(v) => Value::U8 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::U16 { .. } => match u16::try_from(value) {
            Ok(v) => Value::U16 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::U32 { .. } => match u32::try_from(value) {
            Ok(v) => Value::U32 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::U64 { .. } => match u64::try_from(value) {
            Ok(v) => Value::U64 { span, value: v },
            Err(_) => {
                errors.insert(TypeErr::IntegerLiteralOutOfRange {
                    span,
                    value,
                    target_type: effective_ty,
                });
                return;
            }
        },
        Type::U128 { .. } => Value::U128 {
            span,
            value: Box::new(value),
        },
        Type::USize { .. } => {
            // Platform-dependent: truncate to 64-bit for now.
            match u64::try_from(value) {
                Ok(v) => Value::USize {
                    span,
                    bits: 64,
                    value: v,
                },
                Err(_) => {
                    errors.insert(TypeErr::IntegerLiteralOutOfRange {
                        span,
                        value,
                        target_type: effective_ty,
                    });
                    return;
                }
            }
        }
        _ => {
            errors.insert(TypeErr::IntegerLiteralUnsatisfiable {
                span,
                value,
                unsatisfiable_type: effective_ty,
            });
            return;
        }
    };

    id.replace(new_value);
}

/// Resolve an `InferredFloat` to a concrete float value.
fn resolve_float_literal(id: &ValueId, value: f64, target_ty: &TypeId, span: ByteSpan, errors: &mut HashSet<TypeErr>) {
    let effective_ty = match &**target_ty {
        Type::Refine { base, .. } => *base,
        _ => *target_ty,
    };

    let new_value = match &*effective_ty {
        Type::F32 { .. } => Value::F32 {
            span,
            value: OrderedFloat(value as f32),
        },
        Type::F64 { .. } => Value::F64 {
            span,
            value: OrderedFloat(value),
        },
        Type::InferredFloat { .. } => {
            // Still inferred — default to f64 (Rust-style).
            Value::F64 {
                span,
                value: OrderedFloat(value),
            }
        }
        _ => {
            errors.insert(TypeErr::FloatLiteralUnsatisfiable {
                span,
                value: OrderedFloat(value),
                unsatisfiable_type: effective_ty,
            });
            return;
        }
    };

    id.replace(new_value);
}

/// Compute the narrowest integer type that can hold a given value.
/// Prefers signed types over unsigned for positive values ≤ i32::MAX.
pub(crate) fn narrowest_fitting_type(value: u128) -> TypeId {
    // Rust-style defaulting: try signed types first, then unsigned.
    if let Ok(v) = i32::try_from(value) {
        return TypeId::from(Type::I32 {
            span: ByteSpan::default(),
        });
    }
    if let Ok(v) = i64::try_from(value) {
        return TypeId::from(Type::I64 {
            span: ByteSpan::default(),
        });
    }
    if let Ok(v) = u64::try_from(value) {
        return TypeId::from(Type::U64 {
            span: ByteSpan::default(),
        });
    }
    TypeId::from(Type::U128 {
        span: ByteSpan::default(),
    })
}
