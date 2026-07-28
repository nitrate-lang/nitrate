use std::vec;

use nitrate_hir::{BinaryOp, Lit, Type, UnaryOp};

/// A range of possible values for an integer expression: (min, max).
pub(crate) type Bounds = (i128, i128);

/// Returns the inclusive numeric bounds for a primitive integer type.
/// This is the single source of truth for all primitive type ranges.
fn numeric_bounds_for_type(ty: &Type) -> Option<Bounds> {
    match ty {
        Type::U8 { .. } => Some((0, 255)),
        Type::U16 { .. } => Some((0, 65535)),
        Type::U32 { .. } => Some((0, 4_294_967_295)),
        Type::U64 { .. } => Some((0, 18_446_744_073_709_551_615)),
        Type::U128 { .. } => Some((0, i128::MAX)),
        Type::USize { .. } => Some((0, 18_446_744_073_709_551_615)),
        Type::I8 { .. } => Some((-128, 127)),
        Type::I16 { .. } => Some((-32768, 32_767)),
        Type::I32 { .. } => Some((-2_147_483_648, 2_147_483_647)),
        Type::I64 { .. } => Some((-9_223_372_036_854_775_808, 9_223_372_036_854_775_807)),
        Type::I128 { .. } => Some((i128::MIN, i128::MAX)),
        _ => None,
    }
}

/// Convert a Lit value to i128 for bounds computation.
pub(crate) fn lit_to_i128(lit: &Lit) -> Option<i128> {
    match lit {
        Lit::U8(v) => Some(*v as i128),
        Lit::U16(v) => Some(*v as i128),
        Lit::U32(v) => Some(*v as i128),
        Lit::U64(v) => Some(*v as i128),
        Lit::U128(v) => Some(*v as i128),
        Lit::USize(_, v) => Some(*v as i128),
        Lit::I8(v) => Some(*v as i128),
        Lit::I16(v) => Some(*v as i128),
        Lit::I32(v) => Some(*v as i128),
        Lit::I64(v) => Some(*v as i128),
        Lit::I128(v) => Some(*v),
        _ => None,
    }
}

/// Get the inclusive bounds of a Type, either directly from primitive type or refinement bounds.
/// This is the central function that both bounds.rs and solver.rs use.
pub(crate) fn integer_primitive_bounds(ty: &Type) -> Option<Bounds> {
    numeric_bounds_for_type(ty)
}

/// Extract bounds from a Type, accounting for refinement types.
pub(crate) fn extract_bounds_from_type(ty: &Type) -> Option<Bounds> {
    match ty {
        Type::Refine { min, max, .. } => match (lit_to_i128(min), lit_to_i128(max)) {
            (Some(min_val), Some(max_val)) => Some((min_val, max_val)),
            _ => None,
        },
        _ => integer_primitive_bounds(ty),
    }
}

/// Compute the result bounds of a binary operation given operand bounds.
pub(crate) fn compute_binary_bounds(op: &BinaryOp, left: Bounds, right: Bounds) -> Option<Bounds> {
    let (l_min, l_max) = left;
    let (r_min, r_max) = right;
    match op {
        BinaryOp::Add => Some((l_min.saturating_add(r_min), l_max.saturating_add(r_max))),
        BinaryOp::Sub => Some((l_min.saturating_sub(r_max), l_max.saturating_sub(r_min))),
        BinaryOp::Mul => {
            let products = [
                l_min.saturating_mul(r_min),
                l_min.saturating_mul(r_max),
                l_max.saturating_mul(r_min),
                l_max.saturating_mul(r_max),
            ];
            Some((*products.iter().min().unwrap(), *products.iter().max().unwrap()))
        }
        BinaryOp::Div => {
            // For division, when the divisor range crosses zero, we need to
            // consider sign partitions separately to get accurate bounds.
            // E.g., for 1 / y where y ∈ [-1, 2], the result bounds are [-1, 1],
            // not [MIN, MAX] which the old sentinel-based approach produced.
            let candidates = if r_min <= 0 && r_max >= 0 {
                let mut vals = Vec::new();
                // Positive divisor sub-range [1, r_max]
                if r_max > 0 {
                    for &l in &[l_min, l_max] {
                        vals.push(l.saturating_div(1));
                        vals.push(l.saturating_div(r_max));
                    }
                }
                // Negative divisor sub-range [r_min, -1]
                if r_min < 0 {
                    for &l in &[l_min, l_max] {
                        vals.push(l.saturating_div(r_min));
                        vals.push(l.saturating_div(-1));
                    }
                }
                if vals.is_empty() {
                    // Divisor range is exactly [0, 0] — division by zero is undefined
                    vals.extend_from_slice(&[i128::MIN, i128::MAX]);
                    vals
                } else {
                    vals
                }
            } else {
                vec![
                    l_min.saturating_div(r_min),
                    l_min.saturating_div(r_max),
                    l_max.saturating_div(r_min),
                    l_max.saturating_div(r_max),
                ]
            };
            Some((*candidates.iter().min().unwrap(), *candidates.iter().max().unwrap()))
        }
        BinaryOp::Mod => {
            if r_min <= 0 && r_max >= 0 {
                Some((i128::MIN, i128::MAX))
            } else {
                let a = std::cmp::max(r_min.abs(), r_max.abs());
                Some((0, a - 1))
            }
        }
        BinaryOp::And => {
            if l_min >= 0 && r_min >= 0 {
                Some((0, std::cmp::min(l_max, r_max)))
            } else {
                Some((std::cmp::min(l_min, r_min), std::cmp::max(l_max, r_max)))
            }
        }
        BinaryOp::Or => Some((std::cmp::min(l_min, r_min), std::cmp::max(l_max, r_max))),
        BinaryOp::Xor => Some((std::cmp::min(l_min, r_min), std::cmp::max(l_max, r_max))),
        BinaryOp::Shl | BinaryOp::Rol => Some((i128::MIN, i128::MAX)),
        BinaryOp::Shr | BinaryOp::Ror => {
            if l_min >= 0 && r_min >= 0 {
                let smin = if r_max > 0 {
                    l_min.checked_shr(r_max as u32).unwrap_or(l_min)
                } else {
                    l_min
                };
                let smax = if r_min > 0 {
                    l_max.checked_shr(r_min as u32).unwrap_or(l_max)
                } else {
                    l_max
                };
                Some((smin, smax))
            } else {
                Some((i128::MIN, i128::MAX))
            }
        }
        BinaryOp::Lt
        | BinaryOp::Gt
        | BinaryOp::Lte
        | BinaryOp::Gte
        | BinaryOp::Eq
        | BinaryOp::Ne
        | BinaryOp::LogicAnd
        | BinaryOp::LogicOr => None,
    }
}

/// Compute the result bounds of a unary operation given operand bounds.
pub(crate) fn compute_unary_bounds(op: &UnaryOp, operand: Bounds) -> Bounds {
    let (min, max) = operand;
    match op {
        UnaryOp::Add => (min, max),
        UnaryOp::Sub => (max.saturating_neg(), min.saturating_neg()),
        UnaryOp::Not => (!max, !min),
    }
}

/// Check if computed bounds fit within a constraint type's bounds.
/// Returns `true` if the bounds are satisfied.
pub(crate) fn check_bounds_against_constraint(computed_bounds: Bounds, constraint_ty: &Type) -> bool {
    let target_bounds = extract_bounds_from_type(constraint_ty);
    if let Some((target_min, target_max)) = target_bounds {
        let (comp_min, comp_max) = computed_bounds;
        if (comp_min < target_min || comp_max > target_max) && matches!(constraint_ty, Type::Refine { .. }) {
            return false;
        }
    }
    true
}

/// Check whether a specific integer value fits within a refinement type's bounds.
/// Accepts `u128` to avoid overflow when casting values > i128::MAX.
/// Returns `true` if the value is within bounds (or if the type is not a refinement).
pub(crate) fn check_literal_against_refinement(value: u128, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { min, max, .. } => {
            let min_val = lit_to_i128(min);
            let max_val = lit_to_i128(max);
            match (min_val, max_val) {
                (Some(mn), Some(mx)) => {
                    // Handle values that exceed i128::MAX by comparing against
                    // the max bound only (since min is always <= i128::MAX for valid refinements)
                    if value > i128::MAX as u128 {
                        // If value exceeds i128::MAX and max is >= 0, it's within range
                        mx >= 0 && (value as u128) <= mx.unsigned_abs() as u128
                    } else {
                        let signed = value as i128;
                        signed >= mn && signed <= mx
                    }
                }
                _ => true,
            }
        }
        _ => true,
    }
}
