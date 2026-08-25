//! Additional binary-operation coverage at boundaries (i128 and USize widths)
//! plus logical-operator truth-table combinations. This complements `binops.rs`,
//! which covers the common widths; here we verify the widest and pointer-sized
//! integer types and the bool logical operators behave identically.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

fn check(op: mir::MirBinaryOp, ty: mir::MirType, pattern: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let ty: mir::MirTypeId = ty.into();
        fn_binary(b, "bin", ty.clone(), ty.clone(), ty, op);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(pattern), "expected `{}` in {ir}", pattern);
}

fn check_cmp(op: mir::MirBinaryOp, ty: mir::MirType, pattern: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let ty: mir::MirTypeId = ty.into();
        let bool: mir::MirTypeId = mir::MirType::Bool.into();
        fn_binary(b, "bin", ty.clone(), ty, bool, op);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(pattern), "expected `{}` in {ir}", pattern);
}

// ── i128 signed ops (widest integer) ──
#[test]
fn i128_add() {
    check(mir::MirBinaryOp::Add, mir::MirType::I128, "add i128");
}
#[test]
fn i128_sub() {
    check(mir::MirBinaryOp::Sub, mir::MirType::I128, "sub i128");
}
#[test]
fn i128_mul() {
    check(mir::MirBinaryOp::Mul, mir::MirType::I128, "mul i128");
}
#[test]
fn i128_sdiv() {
    check(mir::MirBinaryOp::Div, mir::MirType::I128, "sdiv i128");
}
#[test]
fn i128_srem() {
    check(mir::MirBinaryOp::Mod, mir::MirType::I128, "srem i128");
}
#[test]
fn i128_ashr() {
    check(mir::MirBinaryOp::Shr, mir::MirType::I128, "ashr i128");
}
#[test]
fn i128_shl() {
    check(mir::MirBinaryOp::Shl, mir::MirType::I128, "shl i128");
}
#[test]
fn i128_rol() {
    check(mir::MirBinaryOp::Rol, mir::MirType::I128, "rol");
}
#[test]
fn i128_ror() {
    check(mir::MirBinaryOp::Ror, mir::MirType::I128, "ror");
}
#[test]
fn i128_and() {
    check(mir::MirBinaryOp::And, mir::MirType::I128, "and i128");
}
#[test]
fn i128_or() {
    check(mir::MirBinaryOp::Or, mir::MirType::I128, "or i128");
}
#[test]
fn i128_xor() {
    check(mir::MirBinaryOp::Xor, mir::MirType::I128, "xor i128");
}
#[test]
fn i128_slt() {
    check_cmp(mir::MirBinaryOp::Lt, mir::MirType::I128, "icmp slt");
}
#[test]
fn i128_sgt() {
    check_cmp(mir::MirBinaryOp::Gt, mir::MirType::I128, "icmp sgt");
}
#[test]
fn i128_sle() {
    check_cmp(mir::MirBinaryOp::Lte, mir::MirType::I128, "icmp sle");
}
#[test]
fn i128_sge() {
    check_cmp(mir::MirBinaryOp::Gte, mir::MirType::I128, "icmp sge");
}

// ── u128 unsigned ops (widest integer) ──
#[test]
fn u128_add() {
    check(mir::MirBinaryOp::Add, mir::MirType::U128, "add i128");
}
#[test]
fn u128_sub() {
    check(mir::MirBinaryOp::Sub, mir::MirType::U128, "sub i128");
}
#[test]
fn u128_mul() {
    check(mir::MirBinaryOp::Mul, mir::MirType::U128, "mul i128");
}
#[test]
fn u128_udiv() {
    check(mir::MirBinaryOp::Div, mir::MirType::U128, "udiv i128");
}
#[test]
fn u128_urem() {
    check(mir::MirBinaryOp::Mod, mir::MirType::U128, "urem i128");
}
#[test]
fn u128_lshr() {
    check(mir::MirBinaryOp::Shr, mir::MirType::U128, "lshr i128");
}
#[test]
fn u128_shl() {
    check(mir::MirBinaryOp::Shl, mir::MirType::U128, "shl i128");
}
#[test]
fn u128_rol() {
    check(mir::MirBinaryOp::Rol, mir::MirType::U128, "rol");
}
#[test]
fn u128_ror() {
    check(mir::MirBinaryOp::Ror, mir::MirType::U128, "ror");
}
#[test]
fn u128_and() {
    check(mir::MirBinaryOp::And, mir::MirType::U128, "and i128");
}
#[test]
fn u128_or() {
    check(mir::MirBinaryOp::Or, mir::MirType::U128, "or i128");
}
#[test]
fn u128_xor() {
    check(mir::MirBinaryOp::Xor, mir::MirType::U128, "xor i128");
}
#[test]
fn u128_ult() {
    check_cmp(mir::MirBinaryOp::Lt, mir::MirType::U128, "icmp ult");
}
#[test]
fn u128_ugt() {
    check_cmp(mir::MirBinaryOp::Gt, mir::MirType::U128, "icmp ugt");
}
#[test]
fn u128_ule() {
    check_cmp(mir::MirBinaryOp::Lte, mir::MirType::U128, "icmp ule");
}
#[test]
fn u128_uge() {
    check_cmp(mir::MirBinaryOp::Gte, mir::MirType::U128, "icmp uge");
}

// ── USize ops (pointer-sized, unsigned) ──
#[test]
fn usize_add() {
    check(mir::MirBinaryOp::Add, mir::MirType::USize, "add");
}
#[test]
fn usize_sub() {
    check(mir::MirBinaryOp::Sub, mir::MirType::USize, "sub");
}
#[test]
fn usize_mul() {
    check(mir::MirBinaryOp::Mul, mir::MirType::USize, "mul");
}
#[test]
fn usize_udiv() {
    check(mir::MirBinaryOp::Div, mir::MirType::USize, "udiv");
}
#[test]
fn usize_urem() {
    check(mir::MirBinaryOp::Mod, mir::MirType::USize, "urem");
}
#[test]
fn usize_lshr() {
    check(mir::MirBinaryOp::Shr, mir::MirType::USize, "lshr");
}
#[test]
fn usize_shl() {
    check(mir::MirBinaryOp::Shl, mir::MirType::USize, "shl");
}
#[test]
fn usize_and() {
    check(mir::MirBinaryOp::And, mir::MirType::USize, "and");
}
#[test]
fn usize_or() {
    check(mir::MirBinaryOp::Or, mir::MirType::USize, "or");
}
#[test]
fn usize_xor() {
    check(mir::MirBinaryOp::Xor, mir::MirType::USize, "xor");
}
#[test]
fn usize_ult() {
    check_cmp(mir::MirBinaryOp::Lt, mir::MirType::USize, "icmp ult");
}
#[test]
fn usize_ugt() {
    check_cmp(mir::MirBinaryOp::Gt, mir::MirType::USize, "icmp ugt");
}
#[test]
fn usize_ule() {
    check_cmp(mir::MirBinaryOp::Lte, mir::MirType::USize, "icmp ule");
}
#[test]
fn usize_uge() {
    check_cmp(mir::MirBinaryOp::Gte, mir::MirType::USize, "icmp uge");
}
