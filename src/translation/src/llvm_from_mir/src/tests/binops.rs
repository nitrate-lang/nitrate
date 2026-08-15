//! Tests for binary operation lowering, with emphasis on signed vs unsigned
//! semantics for division, remainder, right shift, and ordered comparisons.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

macro_rules! int_arith {
    ($($name:ident : $ty:expr, $lhs:expr, $rhs:expr, $op:expr, $pattern:expr;)*) => {
        $(
            #[test]
            fn $name() {
                let h = Harness::new();
                let module = h.build_module(move |b| {
                    let ty_id: mir::MirTypeId = $ty.into();
                    fn_binary(
                        b,
                        "bin",
                        ty_id,
                        $op,
                        mir::Operand::Constant($lhs),
                        mir::Operand::Constant($rhs),
                    );
                });
                let ir = h.ir(&module);
                assert!(h.verify(&module), "module invalid: {ir}");
                assert!(ir.contains($pattern), "expected `{}` in {ir}", $pattern);
            }
        )*
    };
}

macro_rules! float_arith {
    ($($name:ident : $ty:expr, $lhs:expr, $rhs:expr, $op:expr, $pattern:expr;)*) => {
        $(
            #[test]
            fn $name() {
                let h = Harness::new();
                let module = h.build_module(move |b| {
                    let ty_id: mir::MirTypeId = $ty.into();
                    fn_binary(
                        b,
                        "bin",
                        ty_id,
                        $op,
                        mir::Operand::Constant($lhs),
                        mir::Operand::Constant($rhs),
                    );
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
    add_i8:  mir::MirType::I8,  mir::MirLiteral::I8(1),  mir::MirLiteral::I8(2),  mir::MirBinaryOp::Add, "add i8";
    add_i16: mir::MirType::I16, mir::MirLiteral::I16(1), mir::MirLiteral::I16(2), mir::MirBinaryOp::Add, "add i16";
    add_i32: mir::MirType::I32, mir::MirLiteral::I32(1), mir::MirLiteral::I32(2), mir::MirBinaryOp::Add, "add i32";
    add_i64: mir::MirType::I64, mir::MirLiteral::I64(1), mir::MirLiteral::I64(2), mir::MirBinaryOp::Add, "add i64";
    add_u8:  mir::MirType::U8,  mir::MirLiteral::U8(1),  mir::MirLiteral::U8(2),  mir::MirBinaryOp::Add, "add i8";
    add_u16: mir::MirType::U16, mir::MirLiteral::U16(1), mir::MirLiteral::U16(2), mir::MirBinaryOp::Add, "add i16";
    add_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(2), mir::MirBinaryOp::Add, "add i32";
    add_u64: mir::MirType::U64, mir::MirLiteral::U64(1), mir::MirLiteral::U64(2), mir::MirBinaryOp::Add, "add i64";

    sub_i8:  mir::MirType::I8,  mir::MirLiteral::I8(1),  mir::MirLiteral::I8(2),  mir::MirBinaryOp::Sub, "sub i8";
    sub_i16: mir::MirType::I16, mir::MirLiteral::I16(1), mir::MirLiteral::I16(2), mir::MirBinaryOp::Sub, "sub i16";
    sub_i32: mir::MirType::I32, mir::MirLiteral::I32(1), mir::MirLiteral::I32(2), mir::MirBinaryOp::Sub, "sub i32";
    sub_i64: mir::MirType::I64, mir::MirLiteral::I64(1), mir::MirLiteral::I64(2), mir::MirBinaryOp::Sub, "sub i64";
    sub_u8:  mir::MirType::U8,  mir::MirLiteral::U8(1),  mir::MirLiteral::U8(2),  mir::MirBinaryOp::Sub, "sub i8";
    sub_u16: mir::MirType::U16, mir::MirLiteral::U16(1), mir::MirLiteral::U16(2), mir::MirBinaryOp::Sub, "sub i16";
    sub_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(2), mir::MirBinaryOp::Sub, "sub i32";
    sub_u64: mir::MirType::U64, mir::MirLiteral::U64(1), mir::MirLiteral::U64(2), mir::MirBinaryOp::Sub, "sub i64";

    mul_i8:  mir::MirType::I8,  mir::MirLiteral::I8(3),  mir::MirLiteral::I8(4),  mir::MirBinaryOp::Mul, "mul i8";
    mul_i16: mir::MirType::I16, mir::MirLiteral::I16(3), mir::MirLiteral::I16(4), mir::MirBinaryOp::Mul, "mul i16";
    mul_i32: mir::MirType::I32, mir::MirLiteral::I32(3), mir::MirLiteral::I32(4), mir::MirBinaryOp::Mul, "mul i32";
    mul_i64: mir::MirType::I64, mir::MirLiteral::I64(3), mir::MirLiteral::I64(4), mir::MirBinaryOp::Mul, "mul i64";
    mul_u8:  mir::MirType::U8,  mir::MirLiteral::U8(3),  mir::MirLiteral::U8(4),  mir::MirBinaryOp::Mul, "mul i8";
    mul_u16: mir::MirType::U16, mir::MirLiteral::U16(3), mir::MirLiteral::U16(4), mir::MirBinaryOp::Mul, "mul i16";
    mul_u32: mir::MirType::U32, mir::MirLiteral::U32(3), mir::MirLiteral::U32(4), mir::MirBinaryOp::Mul, "mul i32";
    mul_u64: mir::MirType::U64, mir::MirLiteral::U64(3), mir::MirLiteral::U64(4), mir::MirBinaryOp::Mul, "mul i64";
}

// ── Float ops ──
float_arith! {
    fadd_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Add, "fadd float";
    fadd_f64: mir::MirType::F64, mir::MirLiteral::F64(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F64(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Add, "fadd double";
    fsub_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Sub, "fsub float";
    fsub_f64: mir::MirType::F64, mir::MirLiteral::F64(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F64(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Sub, "fsub double";
    fmul_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Mul, "fmul float";
    fmul_f64: mir::MirType::F64, mir::MirLiteral::F64(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F64(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Mul, "fmul double";
    fdiv_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Div, "fdiv float";
    fdiv_f64: mir::MirType::F64, mir::MirLiteral::F64(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F64(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Div, "fdiv double";
    frem_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Mod, "frem float";
    frem_f64: mir::MirType::F64, mir::MirLiteral::F64(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F64(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Mod, "frem double";
}

// ── Bitwise ops ──
int_arith! {
    and_i8:  mir::MirType::I8,  mir::MirLiteral::I8(3),  mir::MirLiteral::I8(1),  mir::MirBinaryOp::And, "and i8";
    and_i16: mir::MirType::I16, mir::MirLiteral::I16(3), mir::MirLiteral::I16(1), mir::MirBinaryOp::And, "and i16";
    and_i32: mir::MirType::I32, mir::MirLiteral::I32(3), mir::MirLiteral::I32(1), mir::MirBinaryOp::And, "and i32";
    and_i64: mir::MirType::I64, mir::MirLiteral::I64(3), mir::MirLiteral::I64(1), mir::MirBinaryOp::And, "and i64";
    and_u8:  mir::MirType::U8,  mir::MirLiteral::U8(3),  mir::MirLiteral::U8(1),  mir::MirBinaryOp::And, "and i8";
    and_u16: mir::MirType::U16, mir::MirLiteral::U16(3), mir::MirLiteral::U16(1), mir::MirBinaryOp::And, "and i16";
    and_u32: mir::MirType::U32, mir::MirLiteral::U32(3), mir::MirLiteral::U32(1), mir::MirBinaryOp::And, "and i32";
    and_u64: mir::MirType::U64, mir::MirLiteral::U64(3), mir::MirLiteral::U64(1), mir::MirBinaryOp::And, "and i64";

    or_i8:  mir::MirType::I8,  mir::MirLiteral::I8(2),  mir::MirLiteral::I8(1),  mir::MirBinaryOp::Or, "or i8";
    or_i16: mir::MirType::I16, mir::MirLiteral::I16(2), mir::MirLiteral::I16(1), mir::MirBinaryOp::Or, "or i16";
    or_i32: mir::MirType::I32, mir::MirLiteral::I32(2), mir::MirLiteral::I32(1), mir::MirBinaryOp::Or, "or i32";
    or_i64: mir::MirType::I64, mir::MirLiteral::I64(2), mir::MirLiteral::I64(1), mir::MirBinaryOp::Or, "or i64";
    or_u8:  mir::MirType::U8,  mir::MirLiteral::U8(2),  mir::MirLiteral::U8(1),  mir::MirBinaryOp::Or, "or i8";
    or_u16: mir::MirType::U16, mir::MirLiteral::U16(2), mir::MirLiteral::U16(1), mir::MirBinaryOp::Or, "or i16";
    or_u32: mir::MirType::U32, mir::MirLiteral::U32(2), mir::MirLiteral::U32(1), mir::MirBinaryOp::Or, "or i32";
    or_u64: mir::MirType::U64, mir::MirLiteral::U64(2), mir::MirLiteral::U64(1), mir::MirBinaryOp::Or, "or i64";

    xor_i8:  mir::MirType::I8,  mir::MirLiteral::I8(3),  mir::MirLiteral::I8(1),  mir::MirBinaryOp::Xor, "xor i8";
    xor_i16: mir::MirType::I16, mir::MirLiteral::I16(3), mir::MirLiteral::I16(1), mir::MirBinaryOp::Xor, "xor i16";
    xor_i32: mir::MirType::I32, mir::MirLiteral::I32(3), mir::MirLiteral::I32(1), mir::MirBinaryOp::Xor, "xor i32";
    xor_i64: mir::MirType::I64, mir::MirLiteral::I64(3), mir::MirLiteral::I64(1), mir::MirBinaryOp::Xor, "xor i64";
    xor_u8:  mir::MirType::U8,  mir::MirLiteral::U8(3),  mir::MirLiteral::U8(1),  mir::MirBinaryOp::Xor, "xor i8";
    xor_u16: mir::MirType::U16, mir::MirLiteral::U16(3), mir::MirLiteral::U16(1), mir::MirBinaryOp::Xor, "xor i16";
    xor_u32: mir::MirType::U32, mir::MirLiteral::U32(3), mir::MirLiteral::U32(1), mir::MirBinaryOp::Xor, "xor i32";
    xor_u64: mir::MirType::U64, mir::MirLiteral::U64(3), mir::MirLiteral::U64(1), mir::MirBinaryOp::Xor, "xor i64";
}

// ── Shifts ──
int_arith! {
    shl_i8:  mir::MirType::I8,  mir::MirLiteral::I8(1),  mir::MirLiteral::I8(1),  mir::MirBinaryOp::Shl, "shl i8";
    shl_i16: mir::MirType::I16, mir::MirLiteral::I16(1), mir::MirLiteral::I16(1), mir::MirBinaryOp::Shl, "shl i16";
    shl_i32: mir::MirType::I32, mir::MirLiteral::I32(1), mir::MirLiteral::I32(1), mir::MirBinaryOp::Shl, "shl i32";
    shl_i64: mir::MirType::I64, mir::MirLiteral::I64(1), mir::MirLiteral::I64(1), mir::MirBinaryOp::Shl, "shl i64";
    shl_u8:  mir::MirType::U8,  mir::MirLiteral::U8(1),  mir::MirLiteral::U8(1),  mir::MirBinaryOp::Shl, "shl i8";
    shl_u16: mir::MirType::U16, mir::MirLiteral::U16(1), mir::MirLiteral::U16(1), mir::MirBinaryOp::Shl, "shl i16";
    shl_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(1), mir::MirBinaryOp::Shl, "shl i32";
    shl_u64: mir::MirType::U64, mir::MirLiteral::U64(1), mir::MirLiteral::U64(1), mir::MirBinaryOp::Shl, "shl i64";
}

// ── Signed division → `sdiv` ──
int_arith! {
    sdiv_i8:  mir::MirType::I8,  mir::MirLiteral::I8(-8),  mir::MirLiteral::I8(2),  mir::MirBinaryOp::Div, "sdiv i8";
    sdiv_i16: mir::MirType::I16, mir::MirLiteral::I16(-8), mir::MirLiteral::I16(2), mir::MirBinaryOp::Div, "sdiv i16";
    sdiv_i32: mir::MirType::I32, mir::MirLiteral::I32(-8), mir::MirLiteral::I32(2), mir::MirBinaryOp::Div, "sdiv i32";
    sdiv_i64: mir::MirType::I64, mir::MirLiteral::I64(-8), mir::MirLiteral::I64(2), mir::MirBinaryOp::Div, "sdiv i64";
}

// ── Unsigned division → `udiv` ──
int_arith! {
    udiv_u8:  mir::MirType::U8,  mir::MirLiteral::U8(8),  mir::MirLiteral::U8(2),  mir::MirBinaryOp::Div, "udiv i8";
    udiv_u16: mir::MirType::U16, mir::MirLiteral::U16(8), mir::MirLiteral::U16(2), mir::MirBinaryOp::Div, "udiv i16";
    udiv_u32: mir::MirType::U32, mir::MirLiteral::U32(8), mir::MirLiteral::U32(2), mir::MirBinaryOp::Div, "udiv i32";
    udiv_u64: mir::MirType::U64, mir::MirLiteral::U64(8), mir::MirLiteral::U64(2), mir::MirBinaryOp::Div, "udiv i64";
}

// ── Signed remainder → `srem`; unsigned → `urem` ──
int_arith! {
    srem_i8:  mir::MirType::I8,  mir::MirLiteral::I8(-8),  mir::MirLiteral::I8(3),  mir::MirBinaryOp::Mod, "srem i8";
    srem_i16: mir::MirType::I16, mir::MirLiteral::I16(-8), mir::MirLiteral::I16(3), mir::MirBinaryOp::Mod, "srem i16";
    srem_i32: mir::MirType::I32, mir::MirLiteral::I32(-8), mir::MirLiteral::I32(3), mir::MirBinaryOp::Mod, "srem i32";
    srem_i64: mir::MirType::I64, mir::MirLiteral::I64(-8), mir::MirLiteral::I64(3), mir::MirBinaryOp::Mod, "srem i64";

    urem_u8:  mir::MirType::U8,  mir::MirLiteral::U8(8),  mir::MirLiteral::U8(3),  mir::MirBinaryOp::Mod, "urem i8";
    urem_u16: mir::MirType::U16, mir::MirLiteral::U16(8), mir::MirLiteral::U16(3), mir::MirBinaryOp::Mod, "urem i16";
    urem_u32: mir::MirType::U32, mir::MirLiteral::U32(8), mir::MirLiteral::U32(3), mir::MirBinaryOp::Mod, "urem i32";
    urem_u64: mir::MirType::U64, mir::MirLiteral::U64(8), mir::MirLiteral::U64(3), mir::MirBinaryOp::Mod, "urem i64";
}

// ── Right shift: signed → `ashr`, unsigned → `lshr` ──
int_arith! {
    ashr_i8:  mir::MirType::I8,  mir::MirLiteral::I8(-8),  mir::MirLiteral::I8(1),  mir::MirBinaryOp::Shr, "ashr i8";
    ashr_i16: mir::MirType::I16, mir::MirLiteral::I16(-8), mir::MirLiteral::I16(1), mir::MirBinaryOp::Shr, "ashr i16";
    ashr_i32: mir::MirType::I32, mir::MirLiteral::I32(-8), mir::MirLiteral::I32(1), mir::MirBinaryOp::Shr, "ashr i32";
    ashr_i64: mir::MirType::I64, mir::MirLiteral::I64(-8), mir::MirLiteral::I64(1), mir::MirBinaryOp::Shr, "ashr i64";

    lshr_u8:  mir::MirType::U8,  mir::MirLiteral::U8(8),  mir::MirLiteral::U8(1),  mir::MirBinaryOp::Shr, "lshr i8";
    lshr_u16: mir::MirType::U16, mir::MirLiteral::U16(8), mir::MirLiteral::U16(1), mir::MirBinaryOp::Shr, "lshr i16";
    lshr_u32: mir::MirType::U32, mir::MirLiteral::U32(8), mir::MirLiteral::U32(1), mir::MirBinaryOp::Shr, "lshr i32";
    lshr_u64: mir::MirType::U64, mir::MirLiteral::U64(8), mir::MirLiteral::U64(1), mir::MirBinaryOp::Shr, "lshr i64";
}

// ── Ordered comparisons ──
int_arith! {
    slt_i32: mir::MirType::I32, mir::MirLiteral::I32(-1), mir::MirLiteral::I32(1), mir::MirBinaryOp::Lt, "icmp slt";
    slt_i64: mir::MirType::I64, mir::MirLiteral::I64(-1), mir::MirLiteral::I64(1), mir::MirBinaryOp::Lt, "icmp slt";
    sgt_i32: mir::MirType::I32, mir::MirLiteral::I32(1),  mir::MirLiteral::I32(-1), mir::MirBinaryOp::Gt, "icmp sgt";
    sgt_i64: mir::MirType::I64, mir::MirLiteral::I64(1),  mir::MirLiteral::I64(-1), mir::MirBinaryOp::Gt, "icmp sgt";
    sle_i32: mir::MirType::I32, mir::MirLiteral::I32(-1), mir::MirLiteral::I32(1), mir::MirBinaryOp::Lte, "icmp sle";
    sle_i64: mir::MirType::I64, mir::MirLiteral::I64(-1), mir::MirLiteral::I64(1), mir::MirBinaryOp::Lte, "icmp sle";
    sge_i32: mir::MirType::I32, mir::MirLiteral::I32(1),  mir::MirLiteral::I32(-1), mir::MirBinaryOp::Gte, "icmp sge";
    sge_i64: mir::MirType::I64, mir::MirLiteral::I64(1),  mir::MirLiteral::I64(-1), mir::MirBinaryOp::Gte, "icmp sge";

    ult_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(2), mir::MirBinaryOp::Lt, "icmp ult";
    ult_u64: mir::MirType::U64, mir::MirLiteral::U64(1), mir::MirLiteral::U64(2), mir::MirBinaryOp::Lt, "icmp ult";
    ugt_u32: mir::MirType::U32, mir::MirLiteral::U32(2), mir::MirLiteral::U32(1), mir::MirBinaryOp::Gt, "icmp ugt";
    ugt_u64: mir::MirType::U64, mir::MirLiteral::U64(2), mir::MirLiteral::U64(1), mir::MirBinaryOp::Gt, "icmp ugt";
    ule_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(2), mir::MirBinaryOp::Lte, "icmp ule";
    ule_u64: mir::MirType::U64, mir::MirLiteral::U64(1), mir::MirLiteral::U64(2), mir::MirBinaryOp::Lte, "icmp ule";
    uge_u32: mir::MirType::U32, mir::MirLiteral::U32(2), mir::MirLiteral::U32(1), mir::MirBinaryOp::Gte, "icmp uge";
    uge_u64: mir::MirType::U64, mir::MirLiteral::U64(2), mir::MirLiteral::U64(1), mir::MirBinaryOp::Gte, "icmp uge";
}

// ── Equality ──
int_arith! {
    eq_i8:  mir::MirType::I8,  mir::MirLiteral::I8(1),  mir::MirLiteral::I8(1),  mir::MirBinaryOp::Eq, "icmp eq";
    eq_i16: mir::MirType::I16, mir::MirLiteral::I16(1), mir::MirLiteral::I16(1), mir::MirBinaryOp::Eq, "icmp eq";
    eq_i32: mir::MirType::I32, mir::MirLiteral::I32(1), mir::MirLiteral::I32(1), mir::MirBinaryOp::Eq, "icmp eq";
    eq_i64: mir::MirType::I64, mir::MirLiteral::I64(1), mir::MirLiteral::I64(1), mir::MirBinaryOp::Eq, "icmp eq";
    eq_u8:  mir::MirType::U8,  mir::MirLiteral::U8(1),  mir::MirLiteral::U8(1),  mir::MirBinaryOp::Eq, "icmp eq";
    eq_u16: mir::MirType::U16, mir::MirLiteral::U16(1), mir::MirLiteral::U16(1), mir::MirBinaryOp::Eq, "icmp eq";
    eq_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(1), mir::MirBinaryOp::Eq, "icmp eq";
    eq_u64: mir::MirType::U64, mir::MirLiteral::U64(1), mir::MirLiteral::U64(1), mir::MirBinaryOp::Eq, "icmp eq";

    ne_i8:  mir::MirType::I8,  mir::MirLiteral::I8(1),  mir::MirLiteral::I8(2),  mir::MirBinaryOp::Ne, "icmp ne";
    ne_i16: mir::MirType::I16, mir::MirLiteral::I16(1), mir::MirLiteral::I16(2), mir::MirBinaryOp::Ne, "icmp ne";
    ne_i32: mir::MirType::I32, mir::MirLiteral::I32(1), mir::MirLiteral::I32(2), mir::MirBinaryOp::Ne, "icmp ne";
    ne_i64: mir::MirType::I64, mir::MirLiteral::I64(1), mir::MirLiteral::I64(2), mir::MirBinaryOp::Ne, "icmp ne";
    ne_u8:  mir::MirType::U8,  mir::MirLiteral::U8(1),  mir::MirLiteral::U8(2),  mir::MirBinaryOp::Ne, "icmp ne";
    ne_u16: mir::MirType::U16, mir::MirLiteral::U16(1), mir::MirLiteral::U16(2), mir::MirBinaryOp::Ne, "icmp ne";
    ne_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(2), mir::MirBinaryOp::Ne, "icmp ne";
    ne_u64: mir::MirType::U64, mir::MirLiteral::U64(1), mir::MirLiteral::U64(2), mir::MirBinaryOp::Ne, "icmp ne";
}

// ── Float comparisons ──
float_arith! {
    olt_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Lt, "fcmp olt";
    olt_f64: mir::MirType::F64, mir::MirLiteral::F64(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F64(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Lt, "fcmp olt";
    ogt_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirBinaryOp::Gt, "fcmp ogt";
    ogt_f64: mir::MirType::F64, mir::MirLiteral::F64(ordered_float::OrderedFloat(2.0)), mir::MirLiteral::F64(ordered_float::OrderedFloat(1.0)), mir::MirBinaryOp::Gt, "fcmp ogt";
    ole_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Lte, "fcmp ole";
    oge_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirBinaryOp::Gte, "fcmp oge";
    oeq_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirBinaryOp::Eq, "fcmp oeq";
    one_f32: mir::MirType::F32, mir::MirLiteral::F32(ordered_float::OrderedFloat(1.0)), mir::MirLiteral::F32(ordered_float::OrderedFloat(2.0)), mir::MirBinaryOp::Ne, "fcmp one";
}

// ── Logical ops ──
int_arith! {
    logic_and_bool: mir::MirType::Bool, mir::MirLiteral::Bool(true), mir::MirLiteral::Bool(true), mir::MirBinaryOp::LogicAnd, "and i1";
    logic_or_bool:  mir::MirType::Bool, mir::MirLiteral::Bool(true), mir::MirLiteral::Bool(false), mir::MirBinaryOp::LogicOr, "or i1";
}

// ── Rotations ──
int_arith! {
    rol_i32: mir::MirType::I32, mir::MirLiteral::I32(1), mir::MirLiteral::I32(1), mir::MirBinaryOp::Rol, "rol";
    ror_i32: mir::MirType::I32, mir::MirLiteral::I32(1), mir::MirLiteral::I32(1), mir::MirBinaryOp::Ror, "ror";
    rol_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(1), mir::MirBinaryOp::Rol, "rol";
    ror_u32: mir::MirType::U32, mir::MirLiteral::U32(1), mir::MirLiteral::U32(1), mir::MirBinaryOp::Ror, "ror";
}
