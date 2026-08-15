//! Tests for constant literal lowering.

use crate::test_common::{Harness, fn_return};
use nitrate_mir::prelude as mir;
use ordered_float::OrderedFloat;

#[test]
fn bool_true_is_i1_one() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "t",
            mir::MirType::Bool.into(),
            mir::Operand::Constant(mir::MirLiteral::Bool(true)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("true"), "expected `true` constant in IR: {ir}");
}

#[test]
fn bool_false_is_i1_zero() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "f",
            mir::MirType::Bool.into(),
            mir::Operand::Constant(mir::MirLiteral::Bool(false)),
        );
    });
    let ir = h.ir(&module);
    assert!(ir.contains("false"), "expected `false` constant in IR: {ir}");
}

#[test]
fn i8_literal_sign_extended() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::I8.into(),
            mir::Operand::Constant(mir::MirLiteral::I8(-1)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i8 -1"), "expected `i8 -1` in IR: {ir}");
}

#[test]
fn u8_literal_unsigned() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::U8.into(),
            mir::Operand::Constant(mir::MirLiteral::U8(255)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i8 255"), "expected `i8 255` in IR: {ir}");
}

#[test]
fn i16_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::I16.into(),
            mir::Operand::Constant(mir::MirLiteral::I16(-1234)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i16 -1234"), "expected i16 constant: {ir}");
}

#[test]
fn u16_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::U16.into(),
            mir::Operand::Constant(mir::MirLiteral::U16(65535)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i16 65535"), "expected u16 constant: {ir}");
}

#[test]
fn i32_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::I32.into(),
            mir::Operand::Constant(mir::MirLiteral::I32(-42)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i32 -42"), "expected `i32 -42` in IR: {ir}");
}

#[test]
fn u32_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::U32.into(),
            mir::Operand::Constant(mir::MirLiteral::U32(4_000_000_000)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i32 4000000000"), "expected u32 constant: {ir}");
}

#[test]
fn i64_literal_negative() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::I64.into(),
            mir::Operand::Constant(mir::MirLiteral::I64(-9_000_000_000)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i64 -9000000000"), "expected i64 negative: {ir}");
}

#[test]
fn u64_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::U64.into(),
            mir::Operand::Constant(mir::MirLiteral::U64(u64::MAX)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i64 18446744073709551615"), "expected max u64 in IR: {ir}");
}

#[test]
fn u128_literal_arbitrary_precision() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::U128.into(),
            mir::Operand::Constant(mir::MirLiteral::U128(u128::MAX)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(
        ir.contains("i128 340282366920938463463374607431768211455"),
        "unexpected i128 constant: {ir}"
    );
}

#[test]
fn i128_negative_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::I128.into(),
            mir::Operand::Constant(mir::MirLiteral::I128(-1)),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("i128 -1"), "expected `i128 -1` in IR: {ir}");
}

#[test]
fn f32_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::F32.into(),
            mir::Operand::Constant(mir::MirLiteral::F32(OrderedFloat(1.5))),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("float"), "expected float type in IR: {ir}");
    assert!(ir.contains("1.5"), "expected 1.5 constant in IR: {ir}");
}

#[test]
fn f64_literal() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::F64.into(),
            mir::Operand::Constant(mir::MirLiteral::F64(OrderedFloat(2.25))),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("double"), "expected double type in IR: {ir}");
    assert!(ir.contains("2.25"), "expected 2.25 constant in IR: {ir}");
}

#[test]
fn usize_literal_uses_pointer_width() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::USize.into(),
            mir::Operand::Constant(mir::MirLiteral::USize { bits: 64, value: 7 }),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    let width = h.llvm.ptr_size() * 8;
    assert!(ir.contains(&format!("i{width} 7")), "expected usize 7 in IR: {ir}");
}

#[test]
fn string_literal_becomes_private_global() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        fn_return(
            b,
            "v",
            mir::MirType::Str.into(),
            mir::Operand::Constant(mir::MirLiteral::Str("hello".into())),
        );
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("hello"), "expected string data in IR: {ir}");
    assert!(
        ir.contains("internal constant"),
        "expected const string global in IR: {ir}"
    );
}

#[test]
fn duplicate_string_literals_share_global() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let s: mir::MirLiteral = mir::MirLiteral::Str("dup".into());
        fn_return(b, "f1", mir::MirType::Str.into(), mir::Operand::Constant(s.clone()));
        fn_return(b, "f2", mir::MirType::Str.into(), mir::Operand::Constant(s));
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    let count = ir.matches("dup").count();
    assert_eq!(count, 1, "duplicate string literal created multiple globals: {ir}");
}

#[test]
fn byte_string_literal_becomes_fat_pointer() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let bytes: thin_vec::ThinVec<u8> = thin_vec::thin_vec![1u8, 2, 3];
        let slice_ty = mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::U8.into(),
        }
        .into();
        fn_return(b, "v", slice_ty, mir::Operand::Constant(mir::MirLiteral::BStr(bytes)));
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    let width = h.llvm.ptr_size() * 8;
    assert!(
        ir.contains(&format!("i{width} 3")),
        "byte string length not in IR: {ir}"
    );
}

#[test]
fn all_integer_widths_codegen() {
    let h = Harness::new();
    let cases = [
        (mir::MirLiteral::I8(1), mir::MirType::I8),
        (mir::MirLiteral::I16(1), mir::MirType::I16),
        (mir::MirLiteral::I32(1), mir::MirType::I32),
        (mir::MirLiteral::I64(1), mir::MirType::I64),
        (mir::MirLiteral::U8(1), mir::MirType::U8),
        (mir::MirLiteral::U16(1), mir::MirType::U16),
        (mir::MirLiteral::U32(1), mir::MirType::U32),
        (mir::MirLiteral::U64(1), mir::MirType::U64),
    ];
    for (lit, ty) in cases {
        let lit_clone = lit.clone();
        let ty_clone = ty.clone();
        let module = h.build_module(move |b| {
            let ty_id: mir::MirTypeId = ty_clone.into();
            fn_return(b, "v", ty_id, mir::Operand::Constant(lit_clone));
        });
        assert!(h.verify(&module), "failed to verify literal {lit:?}");
    }
}
