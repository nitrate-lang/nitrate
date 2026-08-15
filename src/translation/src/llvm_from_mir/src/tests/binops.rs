//! Tests for binary operation lowering, with emphasis on signed vs unsigned
//! semantics for division, remainder, right shift, and ordered comparisons.
//!
//! All operands are function parameters (never constants) so LLVM cannot
//! constant-fold the operation, leaving the emitted instruction visible.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

macro_rules! int_arith {
    ($($name:ident : $ity:expr, $rty:expr, $op:expr, $pattern:expr;)*) => {
        $(
            #[test]
            fn $name() {
                let h = Harness::new();
                let module = h.build_module(move |b| {
                    let ity: mir::MirTypeId = $ity.into();
                    let rty: mir::MirTypeId = $rty.into();
                    fn_binary(b, "bin", ity.clone(), ity, rty, $op);
                });
                let ir = h.ir(&module);
                assert!(h.verify(&module), "module invalid: {ir}");
                assert!(ir.contains($pattern), "expected `{}` in {ir}", $pattern);
            }
        )*
    };
}

macro_rules! float_arith {
    ($($name:ident : $fty:expr, $op:expr, $pattern:expr;)*) => {
        $(
            #[test]
            fn $name() {
                let h = Harness::new();
                let module = h.build_module(move |b| {
                    let fty: mir::MirTypeId = $fty.into();
                    fn_binary(b, "bin", fty.clone(), fty, fty, $op);
                });
                let ir = h.ir(&module);
                assert!(h.verify(&module), "module invalid: {ir}");
                assert!(ir.contains($pattern), "expected `{}` in {ir}", $pattern);
            }
        )*
    };
}

// ── Add / Sub / Mul across integer widths ──
int_arith! {
    add_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Add, "add i8";
    add_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Add, "add i16";
    add_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Add, "add i32";
    add_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Add, "add i64";
    add_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Add, "add i8";
    add_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Add, "add i16";
    add_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Add, "add i32";
    add_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Add, "add i64";

    sub_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Sub, "sub i8";
    sub_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Sub, "sub i16";
    sub_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Sub, "sub i32";
    sub_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Sub, "sub i64";
    sub_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Sub, "sub i8";
    sub_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Sub, "sub i16";
    sub_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Sub, "sub i32";
    sub_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Sub, "sub i64";

    mul_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Mul, "mul i8";
    mul_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Mul, "mul i16";
    mul_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Mul, "mul i32";
    mul_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Mul, "mul i64";
    mul_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Mul, "mul i8";
    mul_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Mul, "mul i16";
    mul_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Mul, "mul i32";
    mul_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Mul, "mul i64";
}

// ── Float ops ──
float_arith! {
    fadd_f32: mir::MirType::F32, mir::MirBinaryOp::Add, "fadd float";
    fadd_f64: mir::MirType::F64, mir::MirBinaryOp::Add, "fadd double";
    fsub_f32: mir::MirType::F32, mir::MirBinaryOp::Sub, "fsub float";
    fsub_f64: mir::MirType::F64, mir::MirBinaryOp::Sub, "fsub double";
    fmul_f32: mir::MirType::F32, mir::MirBinaryOp::Mul, "fmul float";
    fmul_f64: mir::MirType::F64, mir::MirBinaryOp::Mul, "fmul double";
    fdiv_f32: mir::MirType::F32, mir::MirBinaryOp::Div, "fdiv float";
    fdiv_f64: mir::MirType::F64, mir::MirBinaryOp::Div, "fdiv double";
    frem_f32: mir::MirType::F32, mir::MirBinaryOp::Mod, "frem float";
    frem_f64: mir::MirType::F64, mir::MirBinaryOp::Mod, "frem double";
}

// ── Bitwise ops ──
int_arith! {
    and_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::And, "and i8";
    and_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::And, "and i16";
    and_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::And, "and i32";
    and_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::And, "and i64";
    and_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::And, "and i8";
    and_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::And, "and i16";
    and_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::And, "and i32";
    and_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::And, "and i64";

    or_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Or, "or i8";
    or_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Or, "or i16";
    or_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Or, "or i32";
    or_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Or, "or i64";
    or_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Or, "or i8";
    or_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Or, "or i16";
    or_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Or, "or i32";
    or_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Or, "or i64";

    xor_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Xor, "xor i8";
    xor_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Xor, "xor i16";
    xor_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Xor, "xor i32";
    xor_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Xor, "xor i64";
    xor_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Xor, "xor i8";
    xor_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Xor, "xor i16";
    xor_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Xor, "xor i32";
    xor_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Xor, "xor i64";
}

// ── Shifts ──
int_arith! {
    shl_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Shl, "shl i8";
    shl_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Shl, "shl i16";
    shl_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Shl, "shl i32";
    shl_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Shl, "shl i64";
    shl_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Shl, "shl i8";
    shl_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Shl, "shl i16";
    shl_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Shl, "shl i32";
    shl_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Shl, "shl i64";
}

// ── Signed division → `sdiv` ──
int_arith! {
    sdiv_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Div, "sdiv i8";
    sdiv_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Div, "sdiv i16";
    sdiv_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Div, "sdiv i32";
    sdiv_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Div, "sdiv i64";
}

// ── Unsigned division → `udiv` ──
int_arith! {
    udiv_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Div, "udiv i8";
    udiv_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Div, "udiv i16";
    udiv_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Div, "udiv i32";
    udiv_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Div, "udiv i64";
}

// ── Signed remainder → `srem`; unsigned → `urem` ──
int_arith! {
    srem_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Mod, "srem i8";
    srem_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Mod, "srem i16";
    srem_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Mod, "srem i32";
    srem_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Mod, "srem i64";

    urem_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Mod, "urem i8";
    urem_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Mod, "urem i16";
    urem_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Mod, "urem i32";
    urem_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Mod, "urem i64";
}

// ── Right shift: signed → `ashr`, unsigned → `lshr` ──
int_arith! {
    ashr_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Shr, "ashr i8";
    ashr_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Shr, "ashr i16";
    ashr_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Shr, "ashr i32";
    ashr_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Shr, "ashr i64";

    lshr_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Shr, "lshr i8";
    lshr_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Shr, "lshr i16";
    lshr_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Shr, "lshr i32";
    lshr_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Shr, "lshr i64";
}

// ── Ordered comparisons (result always bool) ──
int_arith! {
    slt_i32: mir::MirType::I32, mir::MirType::Bool, mir::MirBinaryOp::Lt, "icmp slt";
    slt_i64: mir::MirType::I64, mir::MirType::Bool, mir::MirBinaryOp::Lt, "icmp slt";
    sgt_i32: mir::MirType::I32, mir::MirType::Bool, mir::MirBinaryOp::Gt, "icmp sgt";
    sgt_i64: mir::MirType::I64, mir::MirType::Bool, mir::MirBinaryOp::Gt, "icmp sgt";
    sle_i32: mir::MirType::I32, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp sle";
    sle_i64: mir::MirType::I64, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp sle";
    sge_i32: mir::MirType::I32, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp sge";
    sge_i64: mir::MirType::I64, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp sge";

    ult_u32: mir::MirType::U32, mir::MirType::Bool, mir::MirBinaryOp::Lt, "icmp ult";
    ult_u64: mir::MirType::U64, mir::MirType::Bool, mir::MirBinaryOp::Lt, "icmp ult";
    ugt_u32: mir::MirType::U32, mir::MirType::Bool, mir::MirBinaryOp::Gt, "icmp ugt";
    ugt_u64: mir::MirType::U64, mir::MirType::Bool, mir::MirBinaryOp::Gt, "icmp ugt";
    ule_u32: mir::MirType::U32, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp ule";
    ule_u64: mir::MirType::U64, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp ule";
    uge_u32: mir::MirType::U32, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp uge";
    uge_u64: mir::MirType::U64, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp uge";
}

// ── Equality (result always bool) ──
int_arith! {
    eq_i8:  mir::MirType::I8,  mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    eq_i16: mir::MirType::I16, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    eq_i32: mir::MirType::I32, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    eq_i64: mir::MirType::I64, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    eq_u8:  mir::MirType::U8,  mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    eq_u16: mir::MirType::U16, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    eq_u32: mir::MirType::U32, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    eq_u64: mir::MirType::U64, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";

    ne_i8:  mir::MirType::I8,  mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
    ne_i16: mir::MirType::I16, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
    ne_i32: mir::MirType::I32, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
    ne_i64: mir::MirType::I64, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
    ne_u8:  mir::MirType::U8,  mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
    ne_u16: mir::MirType::U16, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
    ne_u32: mir::MirType::U32, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
    ne_u64: mir::MirType::U64, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
}

// ── Float comparisons (result always bool) ──

fn float_cmp_bool(fty: mir::MirType, op: mir::MirBinaryOp, pattern: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let fty: mir::MirTypeId = fty.into();
        let bool_ty: mir::MirTypeId = mir::MirType::Bool.into();
        fn_binary(b, "fcmp", fty.clone(), fty, bool_ty, op);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(pattern), "expected `{}` in {ir}", pattern);
}

macro_rules! float_cmp {
    ($($name:ident : $fty:expr, $op:expr, $pattern:expr;)*) => {
        $(
            #[test]
            fn $name() {
                float_cmp_bool($fty, $op, $pattern);
            }
        )*
    };
}

float_cmp! {
    olt_f32: mir::MirType::F32, mir::MirBinaryOp::Lt, "fcmp olt";
    olt_f64: mir::MirType::F64, mir::MirBinaryOp::Lt, "fcmp olt";
    ogt_f32: mir::MirType::F32, mir::MirBinaryOp::Gt, "fcmp ogt";
    ogt_f64: mir::MirType::F64, mir::MirBinaryOp::Gt, "fcmp ogt";
    ole_f32: mir::MirType::F32, mir::MirBinaryOp::Lte, "fcmp ole";
    ole_f64: mir::MirType::F64, mir::MirBinaryOp::Lte, "fcmp ole";
    oge_f32: mir::MirType::F32, mir::MirBinaryOp::Gte, "fcmp oge";
    oge_f64: mir::MirType::F64, mir::MirBinaryOp::Gte, "fcmp oge";
    oeq_f32: mir::MirType::F32, mir::MirBinaryOp::Eq, "fcmp oeq";
    oeq_f64: mir::MirType::F64, mir::MirBinaryOp::Eq, "fcmp oeq";
    one_f32: mir::MirType::F32, mir::MirBinaryOp::Ne, "fcmp one";
    one_f64: mir::MirType::F64, mir::MirBinaryOp::Ne, "fcmp one";
}

// ── Logical ops on bool values (result bool) ──
#[test]
fn logic_and_bool_result() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let bool_ty: mir::MirTypeId = mir::MirType::Bool.into();
        fn_binary(
            b,
            "land",
            bool_ty.clone(),
            bool_ty.clone(),
            bool_ty,
            mir::MirBinaryOp::LogicAnd,
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("and i1"), "expected logical and: {ir}");
}

#[test]
fn logic_or_bool_result() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let bool_ty: mir::MirTypeId = mir::MirType::Bool.into();
        fn_binary(
            b,
            "lor",
            bool_ty.clone(),
            bool_ty.clone(),
            bool_ty,
            mir::MirBinaryOp::LogicOr,
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("or i1"), "expected logical or: {ir}");
}

// ── Rotations ──
int_arith! {
    rol_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Rol, "rol";
    ror_i32: mir::MirType::I32, mir::MirType::I32, mir::MirBinaryOp::Ror, "ror";
    rol_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Rol, "rol";
    ror_u32: mir::MirType::U32, mir::MirType::U32, mir::MirBinaryOp::Ror, "ror";
    rol_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Rol, "rol";
    ror_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Ror, "ror";
    rol_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Rol, "rol";
    ror_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Ror, "ror";
}
