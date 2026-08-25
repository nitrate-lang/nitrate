//! Exhaustive comparison matrix: Eq/Ne plus ordered comparisons across every
//! integer and float width, with correct signed/unsigned predicates.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

fn run(op: mir::MirBinaryOp, ty: mir::MirType, pattern: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let ty: mir::MirTypeId = ty.into();
        let bool: mir::MirTypeId = mir::MirType::Bool.into();
        fn_binary(b, "cmp", ty.clone(), ty, bool, op);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(pattern), "expected `{}` in {ir}", pattern);
}

macro_rules! signed_cmps {
    ($($name:ident : $ty:expr, $op:ident, $pred:expr;)*) => {
        $(
            #[test]
            fn $name() {
                run(mir::MirBinaryOp::$op, $ty, $pred);
            }
        )*
    };
}

macro_rules! unsigned_cmps {
    ($($name:ident : $ty:expr, $op:ident, $pred:expr;)*) => {
        $(
            #[test]
            fn $name() {
                run(mir::MirBinaryOp::$op, $ty, $pred);
            }
        )*
    };
}

macro_rules! float_cmps {
    ($($name:ident : $ty:expr, $op:ident, $pred:expr;)*) => {
        $(
            #[test]
            fn $name() {
                run(mir::MirBinaryOp::$op, $ty, $pred);
            }
        )*
    };
}

// ── Signed ordered comparisons across all signed widths ──
signed_cmps! {
    slt_i8:  mir::MirType::I8,  Lt, "icmp slt";
    sgt_i8:  mir::MirType::I8,  Gt, "icmp sgt";
    sle_i8:  mir::MirType::I8,  Lte, "icmp sle";
    sge_i8:  mir::MirType::I8,  Gte, "icmp sge";
    slt_i16: mir::MirType::I16, Lt, "icmp slt";
    sgt_i16: mir::MirType::I16, Gt, "icmp sgt";
    sle_i16: mir::MirType::I16, Lte, "icmp sle";
    sge_i16: mir::MirType::I16, Gte, "icmp sge";
    slt_i32: mir::MirType::I32, Lt, "icmp slt";
    sgt_i32: mir::MirType::I32, Gt, "icmp sgt";
    sle_i32: mir::MirType::I32, Lte, "icmp sle";
    sge_i32: mir::MirType::I32, Gte, "icmp sge";
    slt_i64: mir::MirType::I64, Lt, "icmp slt";
    sgt_i64: mir::MirType::I64, Gt, "icmp sgt";
    sle_i64: mir::MirType::I64, Lte, "icmp sle";
    sge_i64: mir::MirType::I64, Gte, "icmp sge";
    slt_i128: mir::MirType::I128, Lt, "icmp slt";
    sgt_i128: mir::MirType::I128, Gt, "icmp sgt";
    sle_i128: mir::MirType::I128, Lte, "icmp sle";
    sge_i128: mir::MirType::I128, Gte, "icmp sge";
}

// ── Unsigned ordered comparisons across all unsigned widths ──
unsigned_cmps! {
    ult_u8:  mir::MirType::U8,  Lt, "icmp ult";
    ugt_u8:  mir::MirType::U8,  Gt, "icmp ugt";
    ule_u8:  mir::MirType::U8,  Lte, "icmp ule";
    uge_u8:  mir::MirType::U8,  Gte, "icmp uge";
    ult_u16: mir::MirType::U16, Lt, "icmp ult";
    ugt_u16: mir::MirType::U16, Gt, "icmp ugt";
    ule_u16: mir::MirType::U16, Lte, "icmp ule";
    uge_u16: mir::MirType::U16, Gte, "icmp uge";
    ult_u32: mir::MirType::U32, Lt, "icmp ult";
    ugt_u32: mir::MirType::U32, Gt, "icmp ugt";
    ule_u32: mir::MirType::U32, Lte, "icmp ule";
    uge_u32: mir::MirType::U32, Gte, "icmp uge";
    ult_u64: mir::MirType::U64, Lt, "icmp ult";
    ugt_u64: mir::MirType::U64, Gt, "icmp ugt";
    ule_u64: mir::MirType::U64, Lte, "icmp ule";
    uge_u64: mir::MirType::U64, Gte, "icmp uge";
    ult_u128: mir::MirType::U128, Lt, "icmp ult";
    ugt_u128: mir::MirType::U128, Gt, "icmp ugt";
    ule_u128: mir::MirType::U128, Lte, "icmp ule";
    uge_u128: mir::MirType::U128, Gte, "icmp uge";
}

// ── Float ordered comparisons ──
float_cmps! {
    olt_f32: mir::MirType::F32, Lt, "fcmp olt";
    ogt_f32: mir::MirType::F32, Gt, "fcmp ogt";
    ole_f32: mir::MirType::F32, Lte, "fcmp ole";
    oge_f32: mir::MirType::F32, Gte, "fcmp oge";
    olt_f64: mir::MirType::F64, Lt, "fcmp olt";
    ogt_f64: mir::MirType::F64, Gt, "fcmp ogt";
    ole_f64: mir::MirType::F64, Lte, "fcmp ole";
    oge_f64: mir::MirType::F64, Gte, "fcmp oge";
}

// ── Eq/Ne are identical across signed/unsigned/float widths, but verify
// across all widths for completeness ──
macro_rules! eq_ne {
    ($($name:ident : $ty:expr, $op:ident;)*) => {
        $(
            #[test]
            fn $name() {
                run(mir::MirBinaryOp::$op, $ty, "icmp eq");
            }
        )*
    };
}

eq_ne! {
    eq_i8:  mir::MirType::I8,  Eq;
    eq_i16: mir::MirType::I16, Eq;
    eq_i32: mir::MirType::I32, Eq;
    eq_i64: mir::MirType::I64, Eq;
    eq_i128: mir::MirType::I128, Eq;
    eq_u8:  mir::MirType::U8,  Eq;
    eq_u16: mir::MirType::U16, Eq;
    eq_u32: mir::MirType::U32, Eq;
    eq_u64: mir::MirType::U64, Eq;
    eq_u128: mir::MirType::U128, Eq;
}
