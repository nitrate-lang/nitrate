use nitrate_hir::{BinaryOp, Lit, Type, UnaryOp};

/// A range of possible values for an integer expression: (min, max).
pub(crate) type Bounds = (i128, i128);

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

/// Get the inclusive bounds of an integer primitive type.
pub(crate) fn integer_primitive_bounds(ty: &Type) -> Option<Bounds> {
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
            let candidates = if r_min <= 0 && r_max >= 0 {
                vec![
                    if r_min != 0 {
                        l_min.saturating_div(r_min)
                    } else {
                        i128::MAX
                    },
                    if r_max != 0 {
                        l_min.saturating_div(r_max)
                    } else {
                        i128::MAX
                    },
                    if r_min != 0 {
                        l_max.saturating_div(r_min)
                    } else {
                        i128::MIN
                    },
                    if r_max != 0 {
                        l_max.saturating_div(r_max)
                    } else {
                        i128::MIN
                    },
                ]
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
/// Returns `true` if the value is within bounds (or if the type is not a refinement).
pub(crate) fn check_literal_against_refinement(value: i128, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { min, max, .. } => {
            let min_val = lit_to_i128(min);
            let max_val = lit_to_i128(max);
            match (min_val, max_val) {
                (Some(mn), Some(mx)) => value >= mn && value <= mx,
                _ => true,
            }
        }
        _ => true,
    }
}
