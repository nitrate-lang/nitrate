use std::vec;

use nitrate_hir::{BinaryOp, Lit, Type, UnaryOp};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) struct Bounds {
    pub lo: i128,
    pub hi: u128,
}

impl Bounds {
    pub fn new(lo: i128, hi: u128) -> Self {
        Self { lo, hi }
    }

    pub fn signed(lo: i128, hi: i128) -> Self {
        Self { lo, hi: hi as u128 }
    }

    pub fn unsigned(lo: u128, hi: u128) -> Self {
        Self { lo: lo as i128, hi }
    }
}

fn numeric_bounds_for_type(ty: &Type) -> Option<Bounds> {
    match ty {
        Type::U8 { .. } => Some(Bounds::unsigned(0, 255)),
        Type::U16 { .. } => Some(Bounds::unsigned(0, 65535)),
        Type::U32 { .. } => Some(Bounds::unsigned(0, 4_294_967_295)),
        Type::U64 { .. } => Some(Bounds::unsigned(0, 18_446_744_073_709_551_615)),
        Type::U128 { .. } => Some(Bounds::unsigned(0, u128::MAX)),
        Type::USize { .. } => Some(Bounds::unsigned(0, 18_446_744_073_709_551_615)),
        Type::I8 { .. } => Some(Bounds::signed(-128, 127)),
        Type::I16 { .. } => Some(Bounds::signed(-32768, 32_767)),
        Type::I32 { .. } => Some(Bounds::signed(-2_147_483_648, 2_147_483_647)),
        Type::I64 { .. } => Some(Bounds::signed(-9_223_372_036_854_775_808, 9_223_372_036_854_775_807)),
        Type::I128 { .. } => Some(Bounds::signed(i128::MIN, i128::MAX)),
        _ => None,
    }
}

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

pub(crate) fn lit_to_u128(lit: &Lit) -> Option<u128> {
    match lit {
        Lit::U8(v) => Some(*v as u128),
        Lit::U16(v) => Some(*v as u128),
        Lit::U32(v) => Some(*v as u128),
        Lit::U64(v) => Some(*v as u128),
        Lit::U128(v) => Some(*v),
        Lit::USize(_, v) => Some(*v as u128),
        Lit::I8(v) => Some(*v as u128),
        Lit::I16(v) => Some(*v as u128),
        Lit::I32(v) => Some(*v as u128),
        Lit::I64(v) => Some(*v as u128),
        Lit::I128(v) => Some(*v as u128),
        _ => None,
    }
}

pub(crate) fn integer_primitive_bounds(ty: &Type) -> Option<Bounds> {
    numeric_bounds_for_type(ty)
}

pub(crate) fn extract_bounds_from_type(ty: &Type) -> Option<Bounds> {
    match ty {
        Type::Refine { min, max, .. } => match (lit_to_i128(min), lit_to_u128(max)) {
            (Some(min_val), Some(max_val)) => Some(Bounds::new(min_val, max_val)),
            _ => None,
        },
        _ => integer_primitive_bounds(ty),
    }
}

pub(crate) fn compute_binary_bounds(op: &BinaryOp, left: Bounds, right: Bounds) -> Option<Bounds> {
    let (l_min, l_max_i128) = (left.lo, left.hi as i128);
    let (r_min, r_max_i128) = (right.lo, right.hi as i128);
    let is_unsigned_both = left.lo >= 0 && right.lo >= 0;

    match op {
        BinaryOp::Add => {
            let hi = if is_unsigned_both {
                let hi128 = left.hi.saturating_add(right.hi);
                if hi128 > i128::MAX as u128 { hi128 } else { hi128 }
            } else {
                left.hi.saturating_add(right.hi)
            };
            Some(Bounds::new(l_min.saturating_add(r_min), hi))
        }
        BinaryOp::Sub => {
            let lo = if left.hi >= right.hi {
                l_min.saturating_sub(r_max_i128)
            } else {
                l_min.saturating_sub(r_max_i128)
            };
            let hi = if is_unsigned_both && l_min >= 0 && r_min >= 0 {
                if left.hi >= right.lo as u128 {
                    left.hi - right.lo as u128
                } else {
                    0
                }
            } else {
                l_max_i128.saturating_sub(r_min) as u128
            };
            Some(Bounds::new(lo, hi))
        }
        BinaryOp::Mul => {
            let mut products_u128 = Vec::new();
            let products_i128 = [
                l_min.saturating_mul(r_min),
                l_min.saturating_mul(r_max_i128),
                l_max_i128.saturating_mul(r_min),
                l_max_i128.saturating_mul(r_max_i128),
            ];
            if is_unsigned_both {
                let ul_min = left.lo as u128;
                let ul_max = left.hi;
                let ur_min = right.lo as u128;
                let ur_max = right.hi;
                products_u128 = vec![
                    ul_min.saturating_mul(ur_min),
                    ul_min.saturating_mul(ur_max),
                    ul_max.saturating_mul(ur_min),
                    ul_max.saturating_mul(ur_max),
                ];
            }
            if products_u128.is_empty() {
                let min = *products_i128.iter().min().unwrap();
                let max = *products_i128.iter().max().unwrap();
                Some(Bounds::new(min, max as u128))
            } else {
                let min_i128 = *products_i128.iter().min().unwrap();
                let max_u128 = *products_u128.iter().max().unwrap();
                Some(Bounds::new(min_i128, max_u128))
            }
        }
        BinaryOp::Div => {
            let candidates = if r_min <= 0 && r_max_i128 >= 0 {
                let mut vals: Vec<i128> = Vec::new();
                if r_max_i128 > 0 {
                    for &l in &[l_min, l_max_i128] {
                        vals.push(l.saturating_div(1));
                        vals.push(l.saturating_div(r_max_i128));
                    }
                }
                if r_min < 0 {
                    for &l in &[l_min, l_max_i128] {
                        vals.push(l.saturating_div(r_min));
                        vals.push(l.saturating_div(-1));
                    }
                }
                if vals.is_empty() {
                    vals.extend_from_slice(&[i128::MIN, i128::MAX]);
                }
                vals
            } else {
                vec![
                    l_min.saturating_div(r_min),
                    l_min.saturating_div(r_max_i128),
                    l_max_i128.saturating_div(r_min),
                    l_max_i128.saturating_div(r_max_i128),
                ]
            };
            let min = *candidates.iter().min().unwrap();
            let max = *candidates.iter().max().unwrap();
            Some(Bounds::new(min, max as u128))
        }
        BinaryOp::Mod => {
            if r_min <= 0 && r_max_i128 >= 0 {
                Some(Bounds::new(i128::MIN, i128::MAX as u128))
            } else {
                let a = std::cmp::max(r_min.abs(), r_max_i128.abs());
                Some(Bounds::new(0, (a - 1) as u128))
            }
        }
        BinaryOp::And => {
            if l_min >= 0 && r_min >= 0 {
                Some(Bounds::new(0, std::cmp::min(left.hi, right.hi)))
            } else {
                Some(Bounds::new(
                    std::cmp::min(l_min, r_min),
                    std::cmp::max(left.hi, right.hi),
                ))
            }
        }
        BinaryOp::Or => Some(Bounds::new(
            std::cmp::min(l_min, r_min),
            std::cmp::max(left.hi, right.hi),
        )),
        BinaryOp::Xor => Some(Bounds::new(
            std::cmp::min(l_min, r_min),
            std::cmp::max(left.hi, right.hi),
        )),
        BinaryOp::Shl | BinaryOp::Rol => Some(Bounds::new(i128::MIN, i128::MAX as u128)),
        BinaryOp::Shr | BinaryOp::Ror => {
            if l_min >= 0 && r_min >= 0 {
                let smin = if r_max_i128 > 0 && right.hi > 0 {
                    let shift = std::cmp::min(right.hi, 127) as u32;
                    (left.lo as u128).checked_shr(shift).unwrap_or(0) as i128
                } else {
                    l_min
                };
                let smax = if left.hi > 0 {
                    let shift = if r_min > 0 {
                        r_min as u32
                    } else if right.lo as i128 > 0 {
                        right.lo as u32
                    } else {
                        0
                    };
                    if shift > 0 {
                        left.hi.checked_shr(shift).unwrap_or(0).max(1)
                    } else {
                        left.hi
                    }
                } else {
                    0
                };
                Some(Bounds::new(smin, smax))
            } else {
                Some(Bounds::new(i128::MIN, i128::MAX as u128))
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

pub(crate) fn compute_unary_bounds(op: &UnaryOp, operand: Bounds) -> Bounds {
    let (min, max) = (operand.lo, operand.hi);
    let max_i128 = max as i128;
    match op {
        UnaryOp::Add => Bounds::new(min, max),
        UnaryOp::Sub => Bounds::new(
            max_i128.saturating_neg(),
            if min >= 0 {
                (0i128).saturating_sub(min) as u128
            } else {
                min.saturating_neg() as u128
            },
        ),
        UnaryOp::Not => Bounds::new(!max_i128, (!min) as u128),
    }
}

pub(crate) fn check_bounds_against_constraint(computed_bounds: Bounds, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { .. } => {
            let target_bounds = extract_bounds_from_type(constraint_ty);
            if let Some(target) = target_bounds {
                let (comp_min, comp_max) = (computed_bounds.lo, computed_bounds.hi);
                let (target_min, target_max) = (target.lo, target.hi);
                if comp_min < target_min || comp_max > target_max {
                    return false;
                }
            }
            true
        }
        _ => true,
    }
}

pub(crate) fn check_literal_against_refinement(value: u128, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { min, max, .. } => {
            let min_val = lit_to_i128(min);
            let max_val = lit_to_i128(max);
            match (min_val, max_val) {
                (Some(mn), Some(mx_i128)) => {
                    if value > i128::MAX as u128 && mx_i128 < 0 {
                        return false;
                    }
                    if value > i128::MAX as u128 {
                        let mx_u128 = lit_to_u128(max).unwrap_or(0);
                        value <= mx_u128
                    } else {
                        let signed = value as i128;
                        signed >= mn && signed <= mx_i128
                    }
                }
                _ => true,
            }
        }
        _ => true,
    }
}
