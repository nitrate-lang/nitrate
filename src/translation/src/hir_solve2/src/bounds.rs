use std::vec;

use nitrate_hir::{BinaryOp, Type, UnaryOp};

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
        let clamped_hi = hi.max(0) as u128;
        Self { lo, hi: clamped_hi }
    }
    pub fn unsigned(lo: u128, hi: u128) -> Self {
        let clamped_lo = lo.min(i128::MAX as u128) as i128;
        Self { lo: clamped_lo, hi }
    }
}

fn next_all_ones(v: u128) -> u128 {
    if v == 0 {
        return 0;
    }
    let mut result = v;
    result |= result >> 1;
    result |= result >> 2;
    result |= result >> 4;
    result |= result >> 8;
    result |= result >> 16;
    result |= result >> 32;
    result |= result >> 64;
    result
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

pub(crate) fn lit_to_i128(lit: &nitrate_hir::Lit) -> Option<i128> {
    use nitrate_hir::Lit;
    match lit {
        Lit::U8(v) => Some(*v as i128),
        Lit::U16(v) => Some(*v as i128),
        Lit::U32(v) => Some(*v as i128),
        Lit::U64(v) => Some(*v as i128),
        Lit::U128(v) => Some((*v).min(i128::MAX as u128) as i128),
        Lit::USize(_, v) => Some(*v as i128),
        Lit::I8(v) => Some(*v as i128),
        Lit::I16(v) => Some(*v as i128),
        Lit::I32(v) => Some(*v as i128),
        Lit::I64(v) => Some(*v as i128),
        Lit::I128(v) => Some(*v),
        _ => None,
    }
}

pub(crate) fn lit_to_u128(lit: &nitrate_hir::Lit) -> Option<u128> {
    use nitrate_hir::Lit;
    match lit {
        Lit::U8(v) => Some(*v as u128),
        Lit::U16(v) => Some(*v as u128),
        Lit::U32(v) => Some(*v as u128),
        Lit::U64(v) => Some(*v as u128),
        Lit::U128(v) => Some(*v),
        Lit::USize(_, v) => Some(*v as u128),
        Lit::I8(v) => std::convert::TryFrom::try_from(*v).ok(),
        Lit::I16(v) => std::convert::TryFrom::try_from(*v).ok(),
        Lit::I32(v) => std::convert::TryFrom::try_from(*v).ok(),
        Lit::I64(v) => std::convert::TryFrom::try_from(*v).ok(),
        Lit::I128(v) => std::convert::TryFrom::try_from(*v).ok(),
        _ => None,
    }
}

pub(crate) fn extract_bounds_from_type(ty: &Type) -> Option<Bounds> {
    match ty {
        Type::Refine { min, max, .. } => match (lit_to_i128(min), lit_to_u128(max)) {
            (Some(min_val), Some(max_val)) => Some(Bounds::new(min_val, max_val)),
            _ => None,
        },
        _ => numeric_bounds_for_type(ty),
    }
}

// ── compute_binary_bounds and helpers ──────────────────────────────

/// Operand bounds prepared for binary operation computation.
struct BinOpInput {
    l_min: i128,
    l_max: i128,
    r_min: i128,
    r_max: i128,
    left_hi: u128,
    right_hi: u128,
    is_unsigned_both: bool,
}

impl BinOpInput {
    fn new(left: Bounds, right: Bounds) -> Self {
        let l_min = left.lo;
        let r_min = right.lo;
        let l_max = (left.hi.min(i128::MAX as u128)) as i128;
        let r_max = (right.hi.min(i128::MAX as u128)) as i128;
        let is_unsigned_both = l_min >= 0 && r_min >= 0;
        Self {
            l_min,
            l_max,
            r_min,
            r_max,
            left_hi: left.hi,
            right_hi: right.hi,
            is_unsigned_both,
        }
    }
}

pub(crate) fn compute_binary_bounds(op: &BinaryOp, left: Bounds, right: Bounds) -> Option<Bounds> {
    let inp = BinOpInput::new(left, right);
    match op {
        BinaryOp::Add => compute_add_bounds(&inp),
        BinaryOp::Sub => compute_sub_bounds(&inp),
        BinaryOp::Mul => compute_mul_bounds(left, right, &inp),
        BinaryOp::Div => compute_div_bounds(&inp),
        BinaryOp::Mod => compute_mod_bounds(&inp),
        BinaryOp::And => compute_and_bounds(&inp),
        BinaryOp::Or => compute_or_bounds(&inp),
        BinaryOp::Xor => compute_xor_bounds(&inp),
        BinaryOp::Shl | BinaryOp::Rol => Some(Bounds::new(i128::MIN, i128::MAX as u128)),
        BinaryOp::Shr | BinaryOp::Ror => compute_shift_bounds(left, right, &inp),
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

fn compute_add_bounds(inp: &BinOpInput) -> Option<Bounds> {
    let hi = inp.left_hi.saturating_add(inp.right_hi);
    Some(Bounds::new(inp.l_min.saturating_add(inp.r_min), hi))
}

fn compute_sub_bounds(inp: &BinOpInput) -> Option<Bounds> {
    let lo = inp.l_min.saturating_sub(inp.r_max);
    let hi = if inp.is_unsigned_both && inp.l_min >= 0 && inp.r_min >= 0 {
        let r_lo_unsigned = if inp.r_min >= 0 { inp.r_min as u128 } else { 0 };
        if inp.left_hi >= r_lo_unsigned {
            inp.left_hi - r_lo_unsigned
        } else {
            0
        }
    } else {
        inp.l_max.saturating_sub(inp.r_min).max(0) as u128
    };
    Some(Bounds::new(lo, hi))
}

fn compute_mul_bounds(left: Bounds, right: Bounds, inp: &BinOpInput) -> Option<Bounds> {
    let products_i128 = [
        inp.l_min.saturating_mul(inp.r_min),
        inp.l_min.saturating_mul(inp.r_max),
        inp.l_max.saturating_mul(inp.r_min),
        inp.l_max.saturating_mul(inp.r_max),
    ];
    let mut products_u128 = Vec::new();
    if inp.is_unsigned_both {
        let (ul_min, ul_max, ur_min, ur_max) = (left.lo as u128, left.hi, right.lo as u128, right.hi);
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
        let max_u128 = (max as u128).max(min.max(0) as u128);
        Some(Bounds::new(min, max_u128))
    } else {
        let min_u128 = *products_u128.iter().min().unwrap();
        let min_i128_final = if min_u128 <= i128::MAX as u128 {
            min_u128 as i128
        } else {
            i128::MAX
        };
        let max_u128 = *products_u128.iter().max().unwrap();
        Some(Bounds::new(min_i128_final, max_u128))
    }
}

fn compute_div_bounds(inp: &BinOpInput) -> Option<Bounds> {
    let candidates = if inp.r_min <= 0 && inp.r_max >= 0 {
        let mut vals = Vec::new();
        if inp.r_max > 0 {
            for &l in &[inp.l_min, inp.l_max] {
                vals.push(l.saturating_div(1));
                vals.push(l.saturating_div(inp.r_max));
            }
        }
        if inp.r_min < 0 {
            for &l in &[inp.l_min, inp.l_max] {
                vals.push(l.saturating_div(inp.r_min));
                vals.push(l.saturating_div(-1));
            }
        }
        if vals.is_empty() {
            vals.extend_from_slice(&[i128::MIN, i128::MAX]);
        }
        vals
    } else {
        vec![
            inp.l_min.saturating_div(inp.r_min),
            inp.l_min.saturating_div(inp.r_max),
            inp.l_max.saturating_div(inp.r_min),
            inp.l_max.saturating_div(inp.r_max),
        ]
    };
    let min = *candidates.iter().min().unwrap();
    let max = *candidates.iter().max().unwrap();
    Some(Bounds::new(min, max as u128))
}

fn compute_mod_bounds(inp: &BinOpInput) -> Option<Bounds> {
    if inp.r_min <= 0 && inp.r_max >= 0 {
        return Some(Bounds::new(i128::MIN, i128::MAX as u128));
    }
    let abs_max = std::cmp::max(inp.r_min.saturating_abs(), inp.r_max.saturating_abs());
    if abs_max <= 0 {
        return Some(Bounds::new(i128::MIN, i128::MAX as u128));
    }
    let mag = (abs_max - 1) as i128;
    if inp.l_min >= 0 {
        Some(Bounds::new(0, mag.max(0) as u128))
    } else if inp.l_max <= 0 {
        Some(Bounds::new(mag.saturating_neg(), 0))
    } else {
        Some(Bounds::new(mag.saturating_neg(), mag.max(0) as u128))
    }
}

fn compute_and_bounds(inp: &BinOpInput) -> Option<Bounds> {
    if inp.l_min >= 0 && inp.r_min >= 0 {
        Some(Bounds::new(0, std::cmp::min(inp.left_hi, inp.right_hi)))
    } else {
        Some(Bounds::new(
            std::cmp::min(inp.l_min, inp.r_min),
            std::cmp::max(inp.left_hi, inp.right_hi),
        ))
    }
}

fn compute_or_bounds(inp: &BinOpInput) -> Option<Bounds> {
    if inp.l_min >= 0 && inp.r_min >= 0 {
        let lo = std::cmp::max(inp.l_min, inp.r_min) as u128;
        let max_val = std::cmp::max(inp.left_hi, inp.right_hi);
        let hi = next_all_ones(max_val);
        Some(Bounds::new(lo as i128, hi))
    } else {
        let max_val = std::cmp::max(inp.left_hi, inp.right_hi);
        let hi = next_all_ones(max_val);
        Some(Bounds::new(std::cmp::min(inp.l_min, inp.r_min), hi))
    }
}

fn compute_xor_bounds(inp: &BinOpInput) -> Option<Bounds> {
    let max_val = std::cmp::max(inp.left_hi, inp.right_hi);
    let hi = next_all_ones(max_val);
    if inp.l_min >= 0 && inp.r_min >= 0 {
        Some(Bounds::new(0, hi))
    } else {
        Some(Bounds::new(std::cmp::min(inp.l_min, inp.r_min), hi))
    }
}

fn compute_shift_bounds(left: Bounds, right: Bounds, inp: &BinOpInput) -> Option<Bounds> {
    if inp.l_min >= 0 && inp.r_min >= 0 {
        let smin = if inp.r_max > 0 && inp.right_hi > 0 {
            let shift = std::cmp::min(inp.right_hi, 127) as u32;
            (left.lo as u128).checked_shr(shift).unwrap_or(0) as i128
        } else {
            inp.l_min
        };
        let smax = if left.hi > 0 {
            let shift = if inp.r_min > 0 {
                inp.r_min as u32
            } else if right.lo > 0 {
                right.lo as u32
            } else {
                0
            };
            if shift > 0 {
                left.hi.checked_shr(shift).unwrap_or(0)
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

// ── Unary bounds ────────────────────────────────────────────────────

pub(crate) fn compute_unary_bounds(op: &UnaryOp, operand: Bounds) -> Bounds {
    let (lo, hi) = (operand.lo, operand.hi);
    let is_unsigned = lo >= 0;
    match op {
        UnaryOp::Add => Bounds::new(lo, hi),
        UnaryOp::Sub => compute_neg_bounds(lo, hi, is_unsigned),
        UnaryOp::Not => compute_not_bounds(lo, hi, is_unsigned),
    }
}

fn compute_neg_bounds(lo: i128, hi: u128, is_unsigned: bool) -> Bounds {
    if is_unsigned {
        let new_lo = if hi > i128::MAX as u128 {
            i128::MIN
        } else {
            (hi as i128).wrapping_neg()
        };
        let new_hi = lo.saturating_neg() as u128;
        Bounds::new(new_lo, new_hi)
    } else {
        let hi_i128 = (hi.min(i128::MAX as u128)) as i128;
        let new_lo = hi_i128.saturating_neg();
        let new_hi = lo.saturating_neg() as u128;
        Bounds::new(new_lo, new_hi)
    }
}

fn compute_not_bounds(lo: i128, hi: u128, is_unsigned: bool) -> Bounds {
    if is_unsigned {
        let new_lo = (!hi) as i128;
        let new_hi = !(lo as u128);
        Bounds::new(new_lo, new_hi)
    } else {
        let hi_i128 = (hi.min(i128::MAX as u128)) as i128;
        Bounds::new(!hi_i128, (!lo) as u128)
    }
}

// ── Constraint checks ───────────────────────────────────────────────

pub(crate) fn check_bounds_against_constraint(computed_bounds: Bounds, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { .. } => {
            if let Some(target) = extract_bounds_from_type(constraint_ty) {
                let (comp_min, comp_max) = (computed_bounds.lo, computed_bounds.hi);
                let (target_min, target_max) = (target.lo, target.hi);
                !(comp_min < target_min || comp_max > target_max)
            } else {
                false
            }
        }
        _ => true,
    }
}

pub(crate) fn check_literal_against_refinement(value: u128, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { min, max, .. } => {
            let mn = match lit_to_i128(min) {
                Some(v) => v,
                None => return true,
            };
            match lit_to_u128(max) {
                Some(mx) => {
                    if value > mx {
                        return false;
                    }
                    if value <= i128::MAX as u128 {
                        let signed = value as i128;
                        if signed < mn {
                            return false;
                        }
                    }
                    true
                }
                None => {
                    if value > i128::MAX as u128 {
                        return false;
                    }
                    let signed = value as i128;
                    if let Some(mx_i128) = lit_to_i128(max) {
                        signed >= mn && signed <= mx_i128
                    } else {
                        true
                    }
                }
            }
        }
        _ => true,
    }
}
