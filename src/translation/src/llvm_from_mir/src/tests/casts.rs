//! Tests for type-cast lowering, emphasizing sign/zero extension correctness.
//!
//! Values are function parameters so LLVM does not constant-fold the cast,
//! leaving the emitted cast instruction visible for assertion.

use crate::test_common::{Harness, fn_cast};
use nitrate_mir::prelude as mir;

fn cast_ir(src_ty: mir::MirType, target_ty: mir::MirType) -> (Harness, String) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let src: mir::MirTypeId = src_ty.into();
        let tgt: mir::MirTypeId = target_ty.into();
        fn_cast(b, "f", src, tgt);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    (h, ir)
}

#[test]
fn cast_u8_to_u32_zero_extends() {
    let (_, ir) = cast_ir(mir::MirType::U8, mir::MirType::U32);
    assert!(ir.contains("zext"), "expected zero extension: {ir}");
}

#[test]
fn cast_i8_to_i32_sign_extends() {
    let (_, ir) = cast_ir(mir::MirType::I8, mir::MirType::I32);
    assert!(ir.contains("sext"), "expected sign extension: {ir}");
}

#[test]
fn cast_u16_to_u64_zero_extends() {
    let (_, ir) = cast_ir(mir::MirType::U16, mir::MirType::U64);
    assert!(ir.contains("zext"), "expected zero extension: {ir}");
}

#[test]
fn cast_i16_to_i64_sign_extends() {
    let (_, ir) = cast_ir(mir::MirType::I16, mir::MirType::I64);
    assert!(ir.contains("sext"), "expected sign extension: {ir}");
}

#[test]
fn cast_u32_to_u8_truncates() {
    let (_, ir) = cast_ir(mir::MirType::U32, mir::MirType::U8);
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_i32_to_i8_truncates() {
    let (_, ir) = cast_ir(mir::MirType::I32, mir::MirType::I8);
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_u64_to_u32_truncates() {
    let (_, ir) = cast_ir(mir::MirType::U64, mir::MirType::U32);
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_i64_to_i32_truncates() {
    let (_, ir) = cast_ir(mir::MirType::I64, mir::MirType::I32);
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_u128_to_u64_truncates() {
    let (_, ir) = cast_ir(mir::MirType::U128, mir::MirType::U64);
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_i128_to_i64_truncates() {
    let (_, ir) = cast_ir(mir::MirType::I128, mir::MirType::I64);
    assert!(ir.contains("trunc"), "expected truncation: {ir}");
}

#[test]
fn cast_f32_to_f64_extends() {
    let (_, ir) = cast_ir(mir::MirType::F32, mir::MirType::F64);
    assert!(ir.contains("fpext"), "expected float extension: {ir}");
}

#[test]
fn cast_f64_to_f32_truncates() {
    let (_, ir) = cast_ir(mir::MirType::F64, mir::MirType::F32);
    assert!(ir.contains("fptrunc"), "expected float truncation: {ir}");
}

#[test]
fn cast_unsigned_int_to_float() {
    let (_, ir) = cast_ir(mir::MirType::U32, mir::MirType::F32);
    assert!(ir.contains("uitofp"), "expected unsigned int to float: {ir}");
}

#[test]
fn cast_signed_int_to_float() {
    let (_, ir) = cast_ir(mir::MirType::I32, mir::MirType::F64);
    assert!(ir.contains("sitofp"), "expected signed int to float: {ir}");
}

#[test]
fn cast_float_to_unsigned_int() {
    let (_, ir) = cast_ir(mir::MirType::F32, mir::MirType::U32);
    assert!(ir.contains("fptoui"), "expected float to unsigned int: {ir}");
}

#[test]
fn cast_float_to_signed_int() {
    let (_, ir) = cast_ir(mir::MirType::F64, mir::MirType::I32);
    assert!(ir.contains("fptosi"), "expected float to signed int: {ir}");
}

#[test]
fn cast_identity_elided() {
    let (_, ir) = cast_ir(mir::MirType::U32, mir::MirType::U32);
    assert!(
        !ir.contains("zext") && !ir.contains("sext") && !ir.contains("trunc"),
        "no cast needed: {ir}"
    );
}

#[test]
fn cast_int_to_pointer() {
    // Pointers contain nested types (`to: MirType::I32`), so they must be
    // interned inside the TLS store scope rather than at the call site.
    let h = Harness::new();
    let module = h.build_module(|b| {
        let src: mir::MirTypeId = mir::MirType::USize.into();
        let target = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let tgt: mir::MirTypeId = target.into();
        fn_cast(b, "f", src, tgt);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains("inttoptr"), "expected inttoptr cast: {ir}");
}

#[test]
fn cast_pointer_to_int() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let src_ty = mir::MirType::Reference {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let src: mir::MirTypeId = src_ty.into();
        let tgt: mir::MirTypeId = mir::MirType::USize.into();
        fn_cast(b, "f", src, tgt);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains("ptrtoint"), "expected ptrtoint cast: {ir}");
}
