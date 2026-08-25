//! Matrix of unary and nullary rvalue coverage across every numeric width.
//!
//! Unary operations use dynamic parameters (no constant folding), and nullary
//! size/alignment is asserted per type with the expected byte constant.

use crate::test_common::{Harness, fn_unary};
use nitrate_mir::prelude as mir;

fn unary_check(op: mir::MirUnaryOp, ty: mir::MirType, pattern: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let ty: mir::MirTypeId = ty.into();
        fn_unary(b, "f", ty, op);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(pattern), "expected `{}` in {ir}", pattern);
}

macro_rules! neg_int {
    ($name:ident, $ty:expr, $llvm:expr) => {
        #[test]
        fn $name() {
            unary_check(mir::MirUnaryOp::Neg, $ty, &format!("sub {}", $llvm));
        }
    };
}

macro_rules! not_int {
    ($name:ident, $ty:expr, $llvm:expr) => {
        #[test]
        fn $name() {
            unary_check(mir::MirUnaryOp::Not, $ty, &format!("xor {}", $llvm));
        }
    };
}

// ── Negation across signed widths ──
neg_int!(neg_i8, mir::MirType::I8, "i8");
neg_int!(neg_i16, mir::MirType::I16, "i16");
neg_int!(neg_i32, mir::MirType::I32, "i32");
neg_int!(neg_i64, mir::MirType::I64, "i64");
neg_int!(neg_i128, mir::MirType::I128, "i128");

// ── Negation across unsigned widths (wraps, but emits sub) ──
neg_int!(neg_u8, mir::MirType::U8, "i8");
neg_int!(neg_u16, mir::MirType::U16, "i16");
neg_int!(neg_u32, mir::MirType::U32, "i32");
neg_int!(neg_u64, mir::MirType::U64, "i64");
neg_int!(neg_u128, mir::MirType::U128, "i128");

// ── Bitwise NOT across signed widths ──
not_int!(not_i8, mir::MirType::I8, "i8");
not_int!(not_i16, mir::MirType::I16, "i16");
not_int!(not_i32, mir::MirType::I32, "i32");
not_int!(not_i64, mir::MirType::I64, "i64");
not_int!(not_i128, mir::MirType::I128, "i128");

// ── Bitwise NOT across unsigned widths ──
not_int!(not_u8, mir::MirType::U8, "i8");
not_int!(not_u16, mir::MirType::U16, "i16");
not_int!(not_u32, mir::MirType::U32, "i32");
not_int!(not_u64, mir::MirType::U64, "i64");
not_int!(not_u128, mir::MirType::U128, "i128");

// ── Float negation ──
#[test]
fn neg_f32() {
    unary_check(mir::MirUnaryOp::Neg, mir::MirType::F32, "fneg");
}
#[test]
fn neg_f64() {
    unary_check(mir::MirUnaryOp::Neg, mir::MirType::F64, "fneg");
}

// ── Nullary size/alignment helpers ──
fn nullary_check(op: mir::NullaryOp, make_ty: impl FnOnce() -> mir::MirType, expect: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let usize_id: mir::MirTypeId = mir::MirType::USize.into();
        let ty: mir::MirTypeId = make_ty().into();
        let mut f = b.start_function("f".into(), usize_id);
        let tmp = f.new_temp(usize_id, false);
        f.create_block();
        f.push_assign(mir::Place::Local(tmp.clone()), mir::Rvalue::NullaryOp(op, ty));
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(expect), "expected `{}` in {ir}", expect);
}

macro_rules! size_of_matrix {
    ($name:ident, $ty:expr, $expect:expr) => {
        #[test]
        fn $name() {
            nullary_check(mir::NullaryOp::SizeOf, || $ty, $expect);
        }
    };
}

macro_rules! align_of_matrix {
    ($name:ident, $ty:expr, $expect:expr) => {
        #[test]
        fn $name() {
            nullary_check(mir::NullaryOp::AlignOf, || $ty, $expect);
        }
    };
}

// ── SizeOf across every primitive ──
size_of_matrix!(sizeof_u8, mir::MirType::U8, "i64 1");
size_of_matrix!(sizeof_u16, mir::MirType::U16, "i64 2");
size_of_matrix!(sizeof_u32, mir::MirType::U32, "i64 4");
size_of_matrix!(sizeof_u64, mir::MirType::U64, "i64 8");
size_of_matrix!(sizeof_u128, mir::MirType::U128, "i64 16");
size_of_matrix!(sizeof_i8, mir::MirType::I8, "i64 1");
size_of_matrix!(sizeof_i16, mir::MirType::I16, "i64 2");
size_of_matrix!(sizeof_i32, mir::MirType::I32, "i64 4");
size_of_matrix!(sizeof_i64, mir::MirType::I64, "i64 8");
size_of_matrix!(sizeof_i128, mir::MirType::I128, "i64 16");
size_of_matrix!(sizeof_f32, mir::MirType::F32, "i64 4");
size_of_matrix!(sizeof_f64, mir::MirType::F64, "i64 8");
size_of_matrix!(sizeof_bool, mir::MirType::Bool, "i64 1");

// ── AlignOf across every primitive ──
align_of_matrix!(alignof_u8, mir::MirType::U8, "i64 1");
align_of_matrix!(alignof_u16, mir::MirType::U16, "i64 2");
align_of_matrix!(alignof_u32, mir::MirType::U32, "i64 4");
align_of_matrix!(alignof_u64, mir::MirType::U64, "i64 8");
align_of_matrix!(alignof_i8, mir::MirType::I8, "i64 1");
align_of_matrix!(alignof_i16, mir::MirType::I16, "i64 2");
align_of_matrix!(alignof_i32, mir::MirType::I32, "i64 4");
align_of_matrix!(alignof_i64, mir::MirType::I64, "i64 8");
align_of_matrix!(alignof_f32, mir::MirType::F32, "i64 4");
align_of_matrix!(alignof_f64, mir::MirType::F64, "i64 8");
align_of_matrix!(alignof_bool, mir::MirType::Bool, "i64 1");

// ── SizeOf for aggregate shapes ──
#[test]
fn sizeof_array_i32_3() {
    nullary_check(
        mir::NullaryOp::SizeOf,
        || mir::MirType::Array {
            element_type: mir::MirType::I32.into(),
            len: 3,
        },
        "i64 12",
    );
}
#[test]
fn sizeof_array_i8_5() {
    nullary_check(
        mir::NullaryOp::SizeOf,
        || mir::MirType::Array {
            element_type: mir::MirType::I8.into(),
            len: 5,
        },
        "i64 5",
    );
}
#[test]
fn sizeof_tuple_i8_i16() {
    nullary_check(
        mir::NullaryOp::SizeOf,
        || mir::MirType::Tuple {
            element_types: thin_vec::thin_vec![mir::MirType::I8.into(), mir::MirType::I16.into()],
        },
        "i64 4",
    );
}
#[test]
fn sizeof_slice_ref() {
    nullary_check(
        mir::NullaryOp::SizeOf,
        || mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::U8.into(),
        },
        "i64 16",
    );
}

// ── AlignOf for aggregate shapes ──
#[test]
fn alignof_array_i32() {
    nullary_check(
        mir::NullaryOp::AlignOf,
        || mir::MirType::Array {
            element_type: mir::MirType::I32.into(),
            len: 3,
        },
        "i64 4",
    );
}
#[test]
fn alignof_slice_ref() {
    nullary_check(
        mir::NullaryOp::AlignOf,
        || mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::U8.into(),
        },
        "i64 8",
    );
}

// ── Unary op across a temp chain (Neg then Not) ──
#[test]
fn neg_then_not_chain() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let i32: mir::MirTypeId = mir::MirType::I32.into();
        let mut f = b.start_function("f".into(), i32);
        let a = f.add_param("a".into(), i32, false);
        let tmp1 = f.new_temp(i32, false);
        let tmp2 = f.new_temp(i32, false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(tmp1.clone()),
            mir::Rvalue::UnaryOp {
                op: mir::MirUnaryOp::Neg,
                operand: mir::Operand::Copy(mir::Place::Local(a)),
            },
        );
        f.push_assign(
            mir::Place::Local(tmp2.clone()),
            mir::Rvalue::UnaryOp {
                op: mir::MirUnaryOp::Not,
                operand: mir::Operand::Copy(mir::Place::Local(tmp1)),
            },
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(tmp2))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("sub"), "expected neg: {ir}");
    assert!(ir.contains("xor"), "expected not: {ir}");
}
