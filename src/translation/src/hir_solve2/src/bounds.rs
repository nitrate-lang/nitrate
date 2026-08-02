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
        // In release builds, clamp hi to 0 if negative to avoid producing
        // a nonsensical Bounds with a u128::MAX upper bound from wrap.
        let clamped_hi = hi.max(0) as u128;
        Self { lo, hi: clamped_hi }
    }
    pub fn unsigned(lo: u128, hi: u128) -> Self {
        // If lo exceeds i128::MAX, clamp to i128::MAX rather than
        // wrapping to a negative i128 via `as i128` cast.
        let clamped_lo = lo.min(i128::MAX as u128) as i128;
        Self { lo: clamped_lo, hi }
    }
}

/// Returns the smallest `u128` value consisting of all 1 bits that is
/// greater than or equal to `v`. For example:
/// - next_all_ones(0) = 0 (0b0)
/// - next_all_ones(1) = 1 (0b1)
/// - next_all_ones(2) = 3 (0b11)
/// - next_all_ones(3) = 3 (0b11)
/// - next_all_ones(4) = 7 (0b111)
/// - next_all_ones(5) = 7 (0b111)
/// - next_all_ones(u128::MAX) = u128::MAX
fn next_all_ones(v: u128) -> u128 {
    if v == 0 {
        return 0;
    }
    let mut result = v;
    // Set all bits to the right of (and including) the highest set bit
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

pub(crate) fn compute_binary_bounds(op: &BinaryOp, left: Bounds, right: Bounds) -> Option<Bounds> {
    let (l_min, l_max_i128) = (left.lo, (left.hi.min(i128::MAX as u128)) as i128);
    let (r_min, r_max_i128) = (right.lo, (right.hi.min(i128::MAX as u128)) as i128);
    let is_unsigned_both = left.lo >= 0 && right.lo >= 0;
    match op {
        BinaryOp::Add => {
            let hi = left.hi.saturating_add(right.hi);
            Some(Bounds::new(l_min.saturating_add(r_min), hi))
        }
        BinaryOp::Sub => {
            let lo = l_min.saturating_sub(r_max_i128);
            let hi = if is_unsigned_both && l_min >= 0 && r_min >= 0 {
                let r_lo_unsigned = if right.lo >= 0 { right.lo as u128 } else { 0 };
                if left.hi >= r_lo_unsigned {
                    left.hi - r_lo_unsigned
                } else {
                    0
                }
            } else {
                l_max_i128.saturating_sub(r_min).max(0) as u128
            };
            Some(Bounds::new(lo, hi))
        }
        BinaryOp::Mul => {
            let products_i128 = [
                l_min.saturating_mul(r_min),
                l_min.saturating_mul(r_max_i128),
                l_max_i128.saturating_mul(r_min),
                l_max_i128.saturating_mul(r_max_i128),
            ];
            let mut products_u128 = Vec::new();
            if is_unsigned_both {
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
                // Guard against inverted bounds from saturating arithmetic:
                // if min > max_{u128}, clamp max to at least min's unsigned
                // representation so lo <= hi holds.
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
        BinaryOp::Div => {
            let candidates = if r_min <= 0 && r_max_i128 >= 0 {
                let mut vals = Vec::new();
                if r_max_i128 > 0 {
                    for &l in &[l_min, l_max_i128] {
                        vals.push(l.saturating_div(1));
                        vals.push(l.saturating_div(r_max_i128));
                    }
                }
                if r_min < 0 {
                    for &l in &[l_min, l_max_i128] {
                        vals.push(l.saturating_div(r_min));
                        // NOTE: Division by -1 can overflow for i128::MIN.
                        // We use saturating_div here which produces i128::MAX for
                        // i128::MIN / -1, a safe over-approximation for bounds analysis.
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
                // Divisor range does not cross zero.  The sign of `x % y`
                // follows the dividend `x` (truncation-toward-zero semantics).
                // |x % y| < |y_max| = abs_max, so the result lies in
                // [-(abs_max-1), abs_max-1] depending on the dividend's sign.
                let abs_max = std::cmp::max(r_min.saturating_abs(), r_max_i128.saturating_abs());
                if abs_max <= 0 {
                    return Some(Bounds::new(i128::MIN, i128::MAX as u128));
                }
                let mag = (abs_max - 1) as i128;
                if l_min >= 0 {
                    Some(Bounds::new(0, mag.max(0) as u128))
                } else if l_max_i128 <= 0 {
                    Some(Bounds::new(mag.saturating_neg(), 0))
                } else {
                    Some(Bounds::new(mag.saturating_neg(), mag.max(0) as u128))
                }
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
        BinaryOp::Or => {
            // Unsigned OR result: lower bound is max of lower bounds (unsigned),
            // since OR can only set bits, never clear them.
            // The upper bound is the smallest all-ones bitmask covering the
            // maximum operand value, since OR can set any bit that either
            // operand has. E.g., max(1,2) = 2 but 1|2 = 3 (bitmask 0b11).
            if l_min >= 0 && r_min >= 0 {
                let lo = std::cmp::max(l_min, r_min) as u128;
                let max_val = std::cmp::max(left.hi, right.hi);
                let hi = next_all_ones(max_val);
                Some(Bounds::new(lo as i128, hi))
            } else {
                let max_val = std::cmp::max(left.hi, right.hi);
                let hi = next_all_ones(max_val);
                Some(Bounds::new(std::cmp::min(l_min, r_min), hi))
            }
        }
        BinaryOp::Xor => {
            // XOR can produce values exceeding both input upper bounds.
            // E.g., 1 XOR 2 = 3 while max(1,2) = 2. The tightest correct
            // upper bound for unsigned operands is the smallest all-ones
            // bitmask covering the maximum operand.
            if l_min >= 0 && r_min >= 0 {
                let max_val = std::cmp::max(left.hi, right.hi);
                let hi = next_all_ones(max_val);
                Some(Bounds::new(0, hi))
            } else {
                let max_val = std::cmp::max(left.hi, right.hi);
                let hi = next_all_ones(max_val);
                Some(Bounds::new(std::cmp::min(l_min, r_min), hi))
            }
        }
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
    let (lo, hi) = (operand.lo, operand.hi);
    let is_unsigned = lo >= 0;
    match op {
        UnaryOp::Add => Bounds::new(lo, hi),
        UnaryOp::Sub => {
            if is_unsigned {
                // Negating an unsigned value range [lo, hi] produces the signed
                // range [-hi, -lo].  The lower bound is the negation of the
                // maximum unsigned value, clamped to I128::MIN if it exceeds
                // the representable signed range.  The upper bound is the
                // negation of the minimum (lo), which for unsigned lo=0
                // produces 0; for lo>0 the upper bound must be at least the
                // negated lo as an unsigned value.
                let new_lo = if hi > i128::MAX as u128 {
                    i128::MIN
                } else {
                    // Negate hi: -(hi as i128).  Use wrapping_neg so that
                    // i128::MIN wraps to itself rather than panicking in debug.
                    (hi as i128).wrapping_neg()
                };
                // Upper bound: negating lo (the smallest unsigned value)
                // gives the largest signed result.  If lo == 0, negating
                // gives 0.  Otherwise -(lo as i128) is negative, and as a
                // u128 it wraps.  Use the same saturating_neg pattern as the
                // signed branch for consistency.
                let new_hi = lo.saturating_neg() as u128;
                Bounds::new(new_lo, new_hi)
            } else {
                let hi_i128 = (hi.min(i128::MAX as u128)) as i128;
                let new_lo = hi_i128.saturating_neg();
                let new_hi = lo.saturating_neg() as u128;
                Bounds::new(new_lo, new_hi)
            }
        }
        UnaryOp::Not => {
            if is_unsigned {
                // For unsigned NOT: ~x lies in [~hi, ~lo] where both are
                // u128 values.  The lower bound (~hi) may be negative when
                // cast to i128, which is correct — it represents large
                // unsigned values in the upper half of u128 space.
                // Do NOT clip new_hi to i128::MAX; values in
                // (i128::MAX, u128::MAX] are perfectly valid results.
                let new_lo = (!hi) as i128;
                let new_hi = !(lo as u128);
                Bounds::new(new_lo, new_hi)
            } else {
                let hi_i128 = (hi.min(i128::MAX as u128)) as i128;
                Bounds::new(!hi_i128, (!lo) as u128)
            }
        }
    }
}

pub(crate) fn check_bounds_against_constraint(computed_bounds: Bounds, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { .. } => {
            if let Some(target) = extract_bounds_from_type(constraint_ty) {
                let (comp_min, comp_max) = (computed_bounds.lo, computed_bounds.hi);
                let (target_min, target_max) = (target.lo, target.hi);
                !(comp_min < target_min || comp_max > target_max)
            } else {
                // When bounds extraction fails (e.g. min/max are non-literal expressions),
                // we cannot verify the constraint - conservatively report failure
                false
            }
        }
        _ => true,
    }
}

pub(crate) fn check_literal_against_refinement(value: u128, constraint_ty: &Type) -> bool {
    match constraint_ty {
        Type::Refine { min, max, .. } => {
            // Use lit_to_u128 for the max bound to avoid clamping u128::MAX to
            // i128::MAX (which would make the upper-bound check overly permissive).
            let mn = match lit_to_i128(min) {
                Some(v) => v,
                None => return true, // can't check without a concrete min
            };
            match lit_to_u128(max) {
                Some(mx) => {
                    if value > mx {
                        return false;
                    }
                    // Min check: the literal must be >= mn (signed comparison).
                    // If value fits in i128, compare directly; otherwise it's a
                    // large unsigned value which is always >= any i128 min.
                    if value <= i128::MAX as u128 {
                        let signed = value as i128;
                        if signed < mn {
                            return false;
                        }
                    }
                    true
                }
                None => {
                    // max couldn't be converted to u128 (likely a negative literal).
                    // A negative max means only negative signed values can satisfy
                    // the refinement. A large u128 value can never be negative.
                    if value > i128::MAX as u128 {
                        return false;
                    }
                    let signed = value as i128;
                    // Use lit_to_i128 for the negative max comparison
                    if let Some(mx_i128) = lit_to_i128(max) {
                        signed >= mn && signed <= mx_i128
                    } else {
                        true // can't check without concrete bounds
                    }
                }
            }
        }
        _ => true,
    }
}
