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

// ── Tests ────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use nitrate_hir::{BinaryOp, Lit, LiteralId, Store, Type, TypeId, UnaryOp, using_storage};
    use nitrate_tree::SrcPos;

    fn sp() -> SrcPos {
        SrcPos::default()
    }

    // ── Bounds constructors ───────────────────────────────────────

    #[test]
    fn bounds_new() {
        let b = Bounds::new(-10, 100);
        assert_eq!(b.lo, -10);
        assert_eq!(b.hi, 100);
    }

    #[test]
    fn bounds_new_min_max() {
        let b = Bounds::new(i128::MIN, u128::MAX);
        assert_eq!(b.lo, i128::MIN);
        assert_eq!(b.hi, u128::MAX);
    }

    #[test]
    fn bounds_signed_clamps_negative_hi() {
        let b = Bounds::signed(-100, -10);
        assert_eq!(b.hi, 0);
    }

    #[test]
    fn bounds_unsigned_clamps_large_lo() {
        let b = Bounds::unsigned(u128::MAX, u128::MAX);
        assert_eq!(b.lo, i128::MAX as i128);
    }

    // ── lit_to_i128 ──────────────────────────────────────────────

    #[test]
    fn lit_to_i128_u8() {
        assert_eq!(lit_to_i128(&Lit::U8(42)), Some(42));
        assert_eq!(lit_to_i128(&Lit::U8(255)), Some(255));
    }

    #[test]
    fn lit_to_i128_i8() {
        assert_eq!(lit_to_i128(&Lit::I8(-128)), Some(-128));
        assert_eq!(lit_to_i128(&Lit::I8(127)), Some(127));
    }

    #[test]
    fn lit_to_i128_u128_clamped() {
        assert_eq!(lit_to_i128(&Lit::U128(u128::MAX)), Some(i128::MAX));
    }

    #[test]
    fn lit_to_i128_non_integer_returns_none() {
        assert_eq!(lit_to_i128(&Lit::Unit), None);
        assert_eq!(lit_to_i128(&Lit::Bool(true)), None);
    }

    // ── lit_to_u128 ──────────────────────────────────────────────

    #[test]
    fn lit_to_u128_unsigned() {
        assert_eq!(lit_to_u128(&Lit::U8(255)), Some(255));
        assert_eq!(lit_to_u128(&Lit::U128(u128::MAX)), Some(u128::MAX));
    }

    #[test]
    fn lit_to_u128_signed_positive() {
        assert_eq!(lit_to_u128(&Lit::I8(42)), Some(42));
    }

    #[test]
    fn lit_to_u128_signed_negative_returns_none() {
        assert_eq!(lit_to_u128(&Lit::I8(-1)), None);
    }

    #[test]
    fn lit_to_u128_non_integer_returns_none() {
        assert_eq!(lit_to_u128(&Lit::Unit), None);
    }

    // ── extract_bounds_from_type ─────────────────────────────────

    #[test]
    fn extract_bounds_u8() {
        let b = extract_bounds_from_type(&Type::U8 { span: sp() }).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 255);
    }

    #[test]
    fn extract_bounds_i8() {
        let b = extract_bounds_from_type(&Type::I8 { span: sp() }).unwrap();
        assert_eq!(b.lo, -128);
        assert_eq!(b.hi, 127);
    }

    #[test]
    fn extract_bounds_i32() {
        let b = extract_bounds_from_type(&Type::I32 { span: sp() }).unwrap();
        assert_eq!(b.lo, -2_147_483_648);
        assert_eq!(b.hi, 2_147_483_647);
    }

    #[test]
    fn extract_bounds_bool_returns_none() {
        assert_eq!(extract_bounds_from_type(&Type::Bool { span: sp() }), None);
    }

    // ── Full numeric range coverage ───────────────────────────

    #[test]
    fn extract_bounds_u16() {
        let b = extract_bounds_from_type(&Type::U16 { span: sp() }).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 65535);
    }

    #[test]
    fn extract_bounds_u32() {
        let b = extract_bounds_from_type(&Type::U32 { span: sp() }).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 4_294_967_295);
    }

    #[test]
    fn extract_bounds_u64() {
        let b = extract_bounds_from_type(&Type::U64 { span: sp() }).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 18_446_744_073_709_551_615);
    }

    #[test]
    fn extract_bounds_u128() {
        let b = extract_bounds_from_type(&Type::U128 { span: sp() }).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, u128::MAX);
    }

    #[test]
    fn extract_bounds_usize() {
        let b = extract_bounds_from_type(&Type::USize { span: sp() }).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 18_446_744_073_709_551_615);
    }

    #[test]
    fn extract_bounds_i16() {
        let b = extract_bounds_from_type(&Type::I16 { span: sp() }).unwrap();
        assert_eq!(b.lo, -32768);
        assert_eq!(b.hi, 32767);
    }

    #[test]
    fn extract_bounds_i64() {
        let b = extract_bounds_from_type(&Type::I64 { span: sp() }).unwrap();
        assert_eq!(b.lo, -9_223_372_036_854_775_808);
        assert_eq!(b.hi, 9_223_372_036_854_775_807);
    }

    #[test]
    fn extract_bounds_i128() {
        let b = extract_bounds_from_type(&Type::I128 { span: sp() }).unwrap();
        assert_eq!(b.lo, i128::MIN);
        assert_eq!(b.hi, i128::MAX as u128);
    }

    // ── Full lit_to_i128 coverage ────────────────────────────

    #[test]
    fn lit_to_i128_u16() {
        assert_eq!(lit_to_i128(&Lit::U16(1000)), Some(1000));
        assert_eq!(lit_to_i128(&Lit::U16(65535)), Some(65535));
    }

    #[test]
    fn lit_to_i128_u32() {
        assert_eq!(lit_to_i128(&Lit::U32(100_000)), Some(100_000));
    }

    #[test]
    fn lit_to_i128_u64() {
        assert_eq!(lit_to_i128(&Lit::U64(1_000_000)), Some(1_000_000));
    }

    #[test]
    fn lit_to_i128_usize() {
        assert_eq!(lit_to_i128(&Lit::USize(64, 100)), Some(100));
    }

    #[test]
    fn lit_to_i128_i16() {
        assert_eq!(lit_to_i128(&Lit::I16(-32768)), Some(-32768));
    }

    #[test]
    fn lit_to_i128_i32() {
        assert_eq!(lit_to_i128(&Lit::I32(-1_000_000)), Some(-1_000_000));
    }

    #[test]
    fn lit_to_i128_i64() {
        assert_eq!(lit_to_i128(&Lit::I64(42)), Some(42));
    }

    #[test]
    fn lit_to_i128_i128() {
        assert_eq!(lit_to_i128(&Lit::I128(i128::MAX)), Some(i128::MAX));
        assert_eq!(lit_to_i128(&Lit::I128(i128::MIN)), Some(i128::MIN));
    }

    // ── Full lit_to_u128 coverage ────────────────────────────

    #[test]
    fn lit_to_u128_u16() {
        assert_eq!(lit_to_u128(&Lit::U16(65535)), Some(65535));
    }

    #[test]
    fn lit_to_u128_u32() {
        assert_eq!(lit_to_u128(&Lit::U32(1_000_000)), Some(1_000_000));
    }

    #[test]
    fn lit_to_u128_u64() {
        assert_eq!(lit_to_u128(&Lit::U64(1_000_000_000)), Some(1_000_000_000));
    }

    #[test]
    fn lit_to_u128_usize() {
        assert_eq!(lit_to_u128(&Lit::USize(64, 42)), Some(42));
    }

    #[test]
    fn lit_to_u128_i16_positive() {
        assert_eq!(lit_to_u128(&Lit::I16(1000)), Some(1000));
    }

    #[test]
    fn lit_to_u128_i32_positive() {
        assert_eq!(lit_to_u128(&Lit::I32(100_000)), Some(100_000));
    }

    #[test]
    fn lit_to_u128_i64_positive() {
        assert_eq!(lit_to_u128(&Lit::I64(1_000_000)), Some(1_000_000));
    }

    #[test]
    fn lit_to_u128_i128_positive() {
        assert_eq!(lit_to_u128(&Lit::I128(42)), Some(42));
    }

    // ── More boundary edge cases ─────────────────────────────

    #[test]
    fn sub_bounds_signed() {
        let b = compute_binary_bounds(&BinaryOp::Sub, Bounds::signed(-10, 20), Bounds::signed(-5, 10)).unwrap();
        assert_eq!(b.lo, -20);
        assert_eq!(b.hi, 25);
    }

    #[test]
    fn sub_bounds_unsigned_both_different() {
        let b = compute_binary_bounds(&BinaryOp::Sub, Bounds::signed(-10, 20), Bounds::signed(-50, -1)).unwrap();
        assert_eq!(b.lo, -10);
        assert_eq!(b.hi, 70);
    }

    #[test]
    fn div_bounds_negative_divisor() {
        let b = compute_binary_bounds(&BinaryOp::Div, Bounds::signed(10, 20), Bounds::signed(-10, -1)).unwrap();
        assert_eq!(b.lo, -20);
        assert_eq!(b.hi, u128::MAX);
    }

    #[test]
    fn div_bounds_crosses_zero() {
        let b = compute_binary_bounds(&BinaryOp::Div, Bounds::signed(-100, 100), Bounds::signed(-10, 10)).unwrap();
        assert_eq!(b.lo, -100);
    }

    #[test]
    fn div_bounds_signed_both_negative() {
        // signed(-100,-10) has hi=0 (clamped), l_max=0; signed(-5,-2) has hi=0, r_max=0
        // crossing zero: r_min=-5<=0 && r_max=0>=0
        // vals with l_max=0: [20, 0, 100, 0], min=0, max=100
        let b = compute_binary_bounds(&BinaryOp::Div, Bounds::signed(-100, -10), Bounds::signed(-5, -2)).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 100);
    }

    #[test]
    fn mod_bounds_negative_left() {
        let b = compute_binary_bounds(&BinaryOp::Mod, Bounds::signed(-100, -1), Bounds::signed(9, 10)).unwrap();
        assert_eq!(b.lo, -9);
        assert_eq!(b.hi, 0);
    }

    #[test]
    fn mod_bounds_mixed_left() {
        let b = compute_binary_bounds(&BinaryOp::Mod, Bounds::signed(-50, 50), Bounds::signed(9, 10)).unwrap();
        assert_eq!(b.lo, -9);
        assert_eq!(b.hi, 9);
    }

    #[test]
    fn mod_bounds_crosses_zero() {
        let b = compute_binary_bounds(&BinaryOp::Mod, Bounds::signed(0, 100), Bounds::signed(-5, 5)).unwrap();
        assert_eq!(b.lo, i128::MIN);
        assert_eq!(b.hi, i128::MAX as u128);
    }

    #[test]
    fn and_bounds_signed() {
        let b = compute_binary_bounds(&BinaryOp::And, Bounds::signed(-10, 10), Bounds::signed(-5, 5)).unwrap();
        assert_eq!(b.lo, -10);
        assert_eq!(b.hi, 10);
    }

    #[test]
    fn and_bounds_one_unsigned_one_signed() {
        let b = compute_binary_bounds(&BinaryOp::And, Bounds::unsigned(0, 255), Bounds::signed(-1, 0)).unwrap();
        assert_eq!(b.lo, -1);
        assert_eq!(b.hi, 255);
    }

    #[test]
    fn or_bounds_signed() {
        let b = compute_binary_bounds(&BinaryOp::Or, Bounds::signed(-10, -1), Bounds::signed(-5, -1)).unwrap();
        assert_eq!(b.lo, -10);
    }

    #[test]
    fn or_bounds_large_value() {
        // 1023 is already all-ones in its bit range (0b1111111111)
        // next_all_ones(1023) = 1023
        let b = compute_binary_bounds(&BinaryOp::Or, Bounds::unsigned(0, 1023), Bounds::unsigned(0, 1023)).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 1023);
    }

    #[test]
    fn xor_bounds_signed() {
        let b = compute_binary_bounds(&BinaryOp::Xor, Bounds::signed(-8, -1), Bounds::signed(-4, -1)).unwrap();
        assert_eq!(b.lo, -8);
        assert_eq!(b.hi, 0);
    }

    #[test]
    fn shift_bounds_unsigned() {
        let b = compute_binary_bounds(&BinaryOp::Shr, Bounds::unsigned(256, 1024), Bounds::unsigned(1, 2)).unwrap();
        assert_eq!(b.lo, 64);
        assert_eq!(b.hi, 512);
    }

    #[test]
    fn shift_bounds_signed_operand() {
        let b = compute_binary_bounds(&BinaryOp::Shr, Bounds::signed(-1, 0), Bounds::unsigned(1, 1)).unwrap();
        assert_eq!(b.lo, i128::MIN);
        assert_eq!(b.hi, i128::MAX as u128);
    }

    #[test]
    fn shift_bounds_ror() {
        let b = compute_binary_bounds(&BinaryOp::Ror, Bounds::unsigned(256, 1024), Bounds::unsigned(0, 0)).unwrap();
        assert_eq!(b.lo, 256);
        assert_eq!(b.hi, 1024);
    }

    #[test]
    fn rol_returns_full_range() {
        let b = compute_binary_bounds(&BinaryOp::Rol, Bounds::unsigned(1, 10), Bounds::unsigned(0, 63)).unwrap();
        assert_eq!(b.lo, i128::MIN);
        assert_eq!(b.hi, i128::MAX as u128);
    }

    #[test]
    fn extract_bounds_refine() {
        let store = Store::new();
        using_storage(&store, || {
            let min_lit = LiteralId::from(Lit::I8(10));
            let max_lit = LiteralId::from(Lit::I8(20));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::I8 { span: sp() }),
                min: min_lit,
                max: max_lit,
            };
            let b = extract_bounds_from_type(&refine).unwrap();
            assert_eq!(b.lo, 10);
            assert_eq!(b.hi, 20);
        });
    }

    // ── BinOpInput ───────────────────────────────────────────────

    #[test]
    fn binop_input_both_unsigned() {
        let inp = BinOpInput::new(Bounds::unsigned(0, 100), Bounds::unsigned(0, 50));
        assert!(inp.is_unsigned_both);
        assert_eq!(inp.left_hi, 100);
        assert_eq!(inp.right_hi, 50);
    }

    #[test]
    fn binop_input_mixed_sign() {
        let inp = BinOpInput::new(Bounds::signed(-10, 10), Bounds::unsigned(0, 50));
        assert!(!inp.is_unsigned_both);
    }

    // ── compute_add_bounds ───────────────────────────────────────

    #[test]
    fn add_bounds() {
        let b = compute_binary_bounds(&BinaryOp::Add, Bounds::unsigned(0, 10), Bounds::unsigned(0, 20)).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 30);
    }

    // ── compute_sub_bounds ───────────────────────────────────────

    #[test]
    fn sub_bounds() {
        let b = compute_binary_bounds(&BinaryOp::Sub, Bounds::unsigned(10, 100), Bounds::unsigned(0, 5)).unwrap();
        assert_eq!(b.lo, 5);
        assert_eq!(b.hi, 100);
    }

    // ── compute_mul_bounds ───────────────────────────────────────

    #[test]
    fn mul_bounds_unsigned() {
        let b = compute_binary_bounds(&BinaryOp::Mul, Bounds::unsigned(2, 10), Bounds::unsigned(3, 5)).unwrap();
        assert_eq!(b.lo, 6);
        assert_eq!(b.hi, 50);
    }

    // ── compute_div_bounds ───────────────────────────────────────

    #[test]
    fn div_bounds_positive() {
        let b = compute_binary_bounds(&BinaryOp::Div, Bounds::unsigned(10, 100), Bounds::unsigned(2, 5)).unwrap();
        assert_eq!(b.lo, 2);
        assert_eq!(b.hi, 50);
    }

    // ── compute_mod_bounds ───────────────────────────────────────

    #[test]
    fn mod_bounds_positive() {
        let b = compute_binary_bounds(&BinaryOp::Mod, Bounds::unsigned(0, 100), Bounds::unsigned(9, 10)).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 9);
    }

    // ── compute_and_bounds ───────────────────────────────────────

    #[test]
    fn and_bounds_unsigned() {
        let b = compute_binary_bounds(&BinaryOp::And, Bounds::unsigned(0, 0xFF), Bounds::unsigned(0, 0x0F)).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 0x0F);
    }

    // ── compute_or_bounds ────────────────────────────────────────

    #[test]
    fn or_bounds_unsigned() {
        let b = compute_binary_bounds(&BinaryOp::Or, Bounds::unsigned(4, 4), Bounds::unsigned(0, 0)).unwrap();
        assert_eq!(b.lo, 4);
    }

    // ── compute_xor_bounds ───────────────────────────────────────

    #[test]
    fn xor_bounds_unsigned() {
        let b = compute_binary_bounds(&BinaryOp::Xor, Bounds::unsigned(0, 7), Bounds::unsigned(0, 7)).unwrap();
        assert_eq!(b.lo, 0);
        assert_eq!(b.hi, 7);
    }

    // ── Shl/Rol return full range ────────────────────────────────

    #[test]
    fn shl_returns_full_range() {
        let b = compute_binary_bounds(&BinaryOp::Shl, Bounds::unsigned(1, 10), Bounds::unsigned(0, 63)).unwrap();
        assert_eq!(b.lo, i128::MIN);
        assert_eq!(b.hi, i128::MAX as u128);
    }

    // ── Comparison ops return None ───────────────────────────────

    #[test]
    fn comparison_ops_return_none() {
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Lt, Bounds::signed(0, 10), Bounds::signed(0, 5)),
            None
        );
        assert_eq!(
            compute_binary_bounds(&BinaryOp::Gt, Bounds::signed(0, 10), Bounds::signed(0, 5)),
            None
        );
    }

    // ── compute_unary_bounds ─────────────────────────────────────

    #[test]
    fn unary_add_identity() {
        let b = compute_unary_bounds(&UnaryOp::Add, Bounds::signed(-10, 10));
        assert_eq!(b.lo, -10);
        assert_eq!(b.hi, 10);
    }

    #[test]
    fn unary_sub_signed() {
        let b = compute_unary_bounds(&UnaryOp::Sub, Bounds::signed(-10, 10));
        assert_eq!(b.lo, -10);
        assert_eq!(b.hi, 10);
    }

    #[test]
    fn unary_not_signed() {
        let b = compute_unary_bounds(&UnaryOp::Not, Bounds::signed(-128, 127));
        assert_eq!(b.lo, -128);
        assert_eq!(b.hi, 127);
    }

    // ── check_bounds_against_constraint ──────────────────────────

    #[test]
    fn check_bounds_within_refinement() {
        let store = Store::new();
        using_storage(&store, || {
            let min_lit = LiteralId::from(Lit::I8(0));
            let max_lit = LiteralId::from(Lit::I8(100));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::I8 { span: sp() }),
                min: min_lit,
                max: max_lit,
            };
            assert!(check_bounds_against_constraint(Bounds::new(10, 50), &refine));
        });
    }

    #[test]
    fn check_bounds_outside_refinement_low() {
        let store = Store::new();
        using_storage(&store, || {
            let min_lit = LiteralId::from(Lit::I8(10));
            let max_lit = LiteralId::from(Lit::I8(100));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::I8 { span: sp() }),
                min: min_lit,
                max: max_lit,
            };
            assert!(!check_bounds_against_constraint(Bounds::new(5, 50), &refine));
        });
    }

    #[test]
    fn check_bounds_non_refine_always_true() {
        assert!(check_bounds_against_constraint(
            Bounds::new(i128::MIN, u128::MAX),
            &Type::Bool { span: sp() }
        ));
    }

    // ── check_literal_against_refinement ─────────────────────────

    #[test]
    fn check_literal_within_refinement() {
        let store = Store::new();
        using_storage(&store, || {
            let min_lit = LiteralId::from(Lit::I8(10));
            let max_lit = LiteralId::from(Lit::I8(20));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::I8 { span: sp() }),
                min: min_lit,
                max: max_lit,
            };
            assert!(check_literal_against_refinement(15, &refine));
        });
    }

    #[test]
    fn check_literal_below_refinement() {
        let store = Store::new();
        using_storage(&store, || {
            let min_lit = LiteralId::from(Lit::I8(10));
            let max_lit = LiteralId::from(Lit::I8(20));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::I8 { span: sp() }),
                min: min_lit,
                max: max_lit,
            };
            assert!(!check_literal_against_refinement(5, &refine));
        });
    }

    #[test]
    fn check_literal_non_refine_always_true() {
        assert!(check_literal_against_refinement(0, &Type::Bool { span: sp() }));
    }
}
