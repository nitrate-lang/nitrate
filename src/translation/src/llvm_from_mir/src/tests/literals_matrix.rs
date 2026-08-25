//! Matrix of literal constant tests: every literal kind, boundary values, and
//! their emitted LLVM IR text.

use crate::test_common::{Harness, fn_return};
use nitrate_mir::prelude as mir;
use ordered_float::OrderedFloat;

macro_rules! lit_test {
    ($name:ident, $ty:expr, $llvm_ty:expr, $lit:expr, $pat:expr) => {
        #[test]
        fn $name() {
            let h = Harness::new();
            let module = h.build_module(|b| {
                let ty: mir::MirTypeId = $ty.into();
                fn_return(b, "v", ty, mir::Operand::Constant($lit));
            });
            let ir = h.ir(&module);
            assert!(h.verify(&module), "module invalid: {ir}");
            assert!(ir.contains($pat), "expected `{}` in {ir}", $pat);
        }
    };
}

// ── Bool literals ──
lit_test!(bool_true, mir::MirType::Bool, "i1", mir::MirLiteral::Bool(true), "true");
lit_test!(
    bool_false,
    mir::MirType::Bool,
    "i1",
    mir::MirLiteral::Bool(false),
    "false"
);

// ── Signed integer boundary values ──
lit_test!(i8_min, mir::MirType::I8, "i8", mir::MirLiteral::I8(i8::MIN), "i8 -128");
lit_test!(i8_max, mir::MirType::I8, "i8", mir::MirLiteral::I8(i8::MAX), "i8 127");
lit_test!(
    i16_min,
    mir::MirType::I16,
    "i16",
    mir::MirLiteral::I16(i16::MIN),
    "i16 -32768"
);
lit_test!(
    i16_max,
    mir::MirType::I16,
    "i16",
    mir::MirLiteral::I16(i16::MAX),
    "i16 32767"
);
lit_test!(
    i32_min,
    mir::MirType::I32,
    "i32",
    mir::MirLiteral::I32(i32::MIN),
    "i32 -2147483648"
);
lit_test!(
    i32_max,
    mir::MirType::I32,
    "i32",
    mir::MirLiteral::I32(i32::MAX),
    "i32 2147483647"
);
lit_test!(
    i64_min,
    mir::MirType::I64,
    "i64",
    mir::MirLiteral::I64(i64::MIN),
    "i64 -9223372036854775808"
);
lit_test!(
    i64_max,
    mir::MirType::I64,
    "i64",
    mir::MirLiteral::I64(i64::MAX),
    "i64 9223372036854775807"
);
lit_test!(
    i128_min,
    mir::MirType::I128,
    "i128",
    mir::MirLiteral::I128(i128::MIN),
    "i128 -170141183460469231731687303715884105728"
);
lit_test!(
    i128_max,
    mir::MirType::I128,
    "i128",
    mir::MirLiteral::I128(i128::MAX),
    "i128 170141183460469231731687303715884105727"
);

// ── Unsigned integer boundary values (LLVM prints as signed decimal) ──
lit_test!(u8_max, mir::MirType::U8, "i8", mir::MirLiteral::U8(u8::MAX), "i8 -1");
lit_test!(
    u16_max,
    mir::MirType::U16,
    "i16",
    mir::MirLiteral::U16(u16::MAX),
    "i16 -1"
);
lit_test!(
    u32_max,
    mir::MirType::U32,
    "i32",
    mir::MirLiteral::U32(u32::MAX),
    "i32 -1"
);
lit_test!(
    u64_max,
    mir::MirType::U64,
    "i64",
    mir::MirLiteral::U64(u64::MAX),
    "i64 -1"
);
lit_test!(
    u128_max,
    mir::MirType::U128,
    "i128",
    mir::MirLiteral::U128(u128::MAX),
    "i128 -1"
);
lit_test!(u8_zero, mir::MirType::U8, "i8", mir::MirLiteral::U8(0), "i8 0");
lit_test!(u16_zero, mir::MirType::U16, "i16", mir::MirLiteral::U16(0), "i16 0");
lit_test!(u32_zero, mir::MirType::U32, "i32", mir::MirLiteral::U32(0), "i32 0");
lit_test!(u64_zero, mir::MirType::U64, "i64", mir::MirLiteral::U64(0), "i64 0");

// ── USize literals ──
lit_test!(
    usize_0,
    mir::MirType::USize,
    "i64",
    mir::MirLiteral::USize { bits: 64, value: 0 },
    "i64 0"
);
#[test]
fn usize_max_value() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::USize.into(),
            mir::Operand::Constant(mir::MirLiteral::USize {
                bits: 64,
                value: u64::MAX,
            }),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i64 -1"), "expected usize max as i64 -1: {ir}");
}

// ── Float literals (negative, zero, integral) ──
macro_rules! float_test {
    ($name:ident, $ty:expr, $llvm:expr, $lit:expr, $pat:expr) => {
        #[test]
        fn $name() {
            let h = Harness::new();
            let module = h.build_module(|b| {
                let ty: mir::MirTypeId = $ty.into();
                fn_return(b, "v", ty, mir::Operand::Constant($lit));
            });
            let ir = h.ir(&module);
            assert!(h.verify(&module), "module invalid: {ir}");
            assert!(ir.contains($pat), "expected `{}` in {ir}", $pat);
        }
    };
}

float_test!(
    f32_zero,
    mir::MirType::F32,
    "float",
    mir::MirLiteral::F32(OrderedFloat(0.0)),
    "0.0"
);
float_test!(
    f32_neg,
    mir::MirType::F32,
    "float",
    mir::MirLiteral::F32(OrderedFloat(-3.5)),
    "-3.5"
);
float_test!(
    f64_zero,
    mir::MirType::F64,
    "double",
    mir::MirLiteral::F64(OrderedFloat(0.0)),
    "0.0"
);
float_test!(
    f64_neg,
    mir::MirType::F64,
    "double",
    mir::MirLiteral::F64(OrderedFloat(-3.5)),
    "-3.5"
);

// ── Unit literal ──
#[test]
fn unit_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::Unit.into(),
            mir::Operand::Constant(mir::MirLiteral::Unit),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    // Unit maps to LLVM `void`, so the constant return is `ret void`.
    assert!(ir.contains("ret void"), "expected unit ret void: {ir}");
}

// ── String / byte-string literals (dedup, embedding) ──
#[test]
fn str_literal_empty() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::Str.into(),
            mir::Operand::Constant(mir::MirLiteral::Str("".into())),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("constant [1 x i8]"), "expected empty string: {ir}");
}

#[test]
fn str_literal_multiple_distinct() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "f1",
            mir::MirType::Str.into(),
            mir::Operand::Constant(mir::MirLiteral::Str("a".into())),
        );
        fn_return(
            b,
            "f2",
            mir::MirType::Str.into(),
            mir::Operand::Constant(mir::MirLiteral::Str("b".into())),
        );
        fn_return(
            b,
            "f3",
            mir::MirType::Str.into(),
            mir::Operand::Constant(mir::MirLiteral::Str("c".into())),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("a"), "missing string a: {ir}");
    assert!(ir.contains("b"), "missing string b: {ir}");
    assert!(ir.contains("c"), "missing string c: {ir}");
}

#[test]
fn bstr_literal_empty() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let slice_ty = mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::U8.into(),
        };
        let slice_id: mir::MirTypeId = slice_ty.into();
        fn_return(
            b,
            "v",
            slice_id,
            mir::Operand::Constant(mir::MirLiteral::BStr(thin_vec::ThinVec::new())),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    let width = h.llvm.ptr_size() * 8;
    assert!(ir.contains(&format!("i{width} 0")), "expected len 0: {ir}");
}
