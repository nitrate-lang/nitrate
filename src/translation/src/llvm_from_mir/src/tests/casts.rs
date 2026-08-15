//! Tests for type-cast lowering, emphasizing sign/zero extension correctness.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;
use ordered_float::OrderedFloat;

/// Build a function `name(value as target)` and return its IR text.
fn cast_fn(h: &Harness, name: &str, value: mir::Operand, target: mir::MirType) -> String {
    h.build_ir(|b| {
        let target_id: mir::MirTypeId = target.into();
        let mut f = b.start_function(name.into(), target_id);
        let tmp = f.new_temp(target_id, false);
        f.create_block();
        let place = mir::Place::Local(tmp.clone());
        f.push_assign(
            place.clone(),
            mir::Rvalue::Cast {
                value,
                target_ty: target_id,
            },
        );
        f.ret(Some(mir::Operand::Copy(place)));
        f.finish_function();
    })
}

#[test]
fn cast_u8_to_u32_zero_extends() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::U8(255)),
        mir::MirType::U32,
    );
    assert!(ir.contains("zext"), "expected zero extension: {ir}");
}

#[test]
fn cast_i8_to_i32_sign_extends() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::I8(-1)),
        mir::MirType::I32,
    );
    assert!(ir.contains("sext"), "expected sign extension: {ir}");
}

#[test]
fn cast_u16_to_u64_zero_extends() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::U16(1)),
        mir::MirType::U64,
    );
    assert!(ir.contains("zext"), "expected zero extension: {ir}");
}

#[test]
fn cast_i16_to_i64_sign_extends() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::I16(-1)),
        mir::MirType::I64,
    );
    assert!(ir.contains("sext"), "expected sign extension: {ir}");
}

#[test]
fn cast_u32_to_u8_truncates() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::U32(300)),
        mir::MirType::U8,
    );
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_i32_to_i8_truncates() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::I32(-300)),
        mir::MirType::I8,
    );
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_u64_to_u32_truncates() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::U64(u64::MAX)),
        mir::MirType::U32,
    );
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_i64_to_i32_truncates() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::I64(-1)),
        mir::MirType::I32,
    );
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_u128_to_u64_truncates() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::U128(u128::MAX)),
        mir::MirType::U64,
    );
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_f32_to_f64_extends() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::F32(OrderedFloat(1.0))),
        mir::MirType::F64,
    );
    assert!(ir.contains("fpext"), "expected float extension: {ir}");
}

#[test]
fn cast_f64_to_f32_truncates() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::F64(OrderedFloat(1.0))),
        mir::MirType::F32,
    );
    assert!(ir.contains("fptrunc"), "expected float truncation: {ir}");
}

#[test]
fn cast_unsigned_int_to_float() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::U32(3)),
        mir::MirType::F32,
    );
    assert!(ir.contains("uitofp"), "expected unsigned int to float: {ir}");
}

#[test]
fn cast_signed_int_to_float() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::I32(3)),
        mir::MirType::F64,
    );
    assert!(ir.contains("sitofp"), "expected signed int to float: {ir}");
}

#[test]
fn cast_float_to_unsigned_int() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::F32(OrderedFloat(3.0))),
        mir::MirType::U32,
    );
    assert!(ir.contains("fptoui"), "expected float to unsigned int: {ir}");
}

#[test]
fn cast_float_to_signed_int() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::F64(OrderedFloat(3.0))),
        mir::MirType::I32,
    );
    assert!(ir.contains("fptosi"), "expected float to signed int: {ir}");
}

#[test]
fn cast_identity_elided() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::U32(1)),
        mir::MirType::U32,
    );
    assert!(
        !ir.contains("zext") && !ir.contains("sext") && !ir.contains("trunc"),
        "no cast needed: {ir}"
    );
}

#[test]
fn cast_int_to_pointer() {
    let h = Harness::new();
    let ir = cast_fn(
        &h,
        "f",
        mir::Operand::Constant(mir::MirLiteral::USize { bits: 64, value: 0 }),
        mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        },
    );
    assert!(ir.contains("inttoptr"), "expected inttoptr cast: {ir}");
}
