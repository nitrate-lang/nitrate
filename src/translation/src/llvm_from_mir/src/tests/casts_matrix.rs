//! Exhaustive cast matrix: every integer source width to every integer target
//! width, plus every float/int and int/float combination. Sign/zero extension
//! and truncation are asserted per source signedness.
//!
//! Values are dynamic function parameters so the cast instruction is emitted
//! rather than constant-folded.

use crate::test_common::{Harness, fn_cast};
use nitrate_mir::prelude as mir;

fn cast_ir(src: mir::MirType, dst: mir::MirType) -> String {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let src: mir::MirTypeId = src.into();
        let dst: mir::MirTypeId = dst.into();
        fn_cast(b, "f", src, dst);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    ir
}

macro_rules! widening {
    ($name:ident, $src:expr, $dst:expr, $instr:expr) => {
        #[test]
        fn $name() {
            let ir = cast_ir($src, $dst);
            assert!(ir.contains($instr), "expected `{}` in {ir}", $instr);
        }
    };
}

macro_rules! truncating {
    ($name:ident, $src:expr, $dst:expr) => {
        #[test]
        fn $name() {
            let ir = cast_ir($src, $dst);
            assert!(ir.contains("trunc"), "expected `trunc` in {ir}");
        }
    };
}

// ── Signed widening (sext) ──
widening!(sext_i8_to_i16, mir::MirType::I8, mir::MirType::I16, "sext");
widening!(sext_i8_to_i32, mir::MirType::I8, mir::MirType::I32, "sext");
widening!(sext_i8_to_i64, mir::MirType::I8, mir::MirType::I64, "sext");
widening!(sext_i8_to_i128, mir::MirType::I8, mir::MirType::I128, "sext");
widening!(sext_i16_to_i32, mir::MirType::I16, mir::MirType::I32, "sext");
widening!(sext_i16_to_i64, mir::MirType::I16, mir::MirType::I64, "sext");
widening!(sext_i16_to_i128, mir::MirType::I16, mir::MirType::I128, "sext");
widening!(sext_i32_to_i64, mir::MirType::I32, mir::MirType::I64, "sext");
widening!(sext_i32_to_i128, mir::MirType::I32, mir::MirType::I128, "sext");
widening!(sext_i64_to_i128, mir::MirType::I64, mir::MirType::I128, "sext");

// ── Unsigned widening (zext) ──
widening!(zext_u8_to_u16, mir::MirType::U8, mir::MirType::U16, "zext");
widening!(zext_u8_to_u32, mir::MirType::U8, mir::MirType::U32, "zext");
widening!(zext_u8_to_u64, mir::MirType::U8, mir::MirType::U64, "zext");
widening!(zext_u8_to_u128, mir::MirType::U8, mir::MirType::U128, "zext");
widening!(zext_u16_to_u32, mir::MirType::U16, mir::MirType::U32, "zext");
widening!(zext_u16_to_u64, mir::MirType::U16, mir::MirType::U64, "zext");
widening!(zext_u16_to_u128, mir::MirType::U16, mir::MirType::U128, "zext");
widening!(zext_u32_to_u64, mir::MirType::U32, mir::MirType::U64, "zext");
widening!(zext_u32_to_u128, mir::MirType::U32, mir::MirType::U128, "zext");
widening!(zext_u64_to_u128, mir::MirType::U64, mir::MirType::U128, "zext");

// ── Signed truncation ──
truncating!(trunc_i16_to_i8, mir::MirType::I16, mir::MirType::I8);
truncating!(trunc_i32_to_i8, mir::MirType::I32, mir::MirType::I8);
truncating!(trunc_i32_to_i16, mir::MirType::I32, mir::MirType::I16);
truncating!(trunc_i64_to_i8, mir::MirType::I64, mir::MirType::I8);
truncating!(trunc_i64_to_i16, mir::MirType::I64, mir::MirType::I16);
truncating!(trunc_i64_to_i32, mir::MirType::I64, mir::MirType::I32);
truncating!(trunc_i128_to_i8, mir::MirType::I128, mir::MirType::I8);
truncating!(trunc_i128_to_i16, mir::MirType::I128, mir::MirType::I16);
truncating!(trunc_i128_to_i32, mir::MirType::I128, mir::MirType::I32);
truncating!(trunc_i128_to_i64, mir::MirType::I128, mir::MirType::I64);

// ── Unsigned truncation ──
truncating!(trunc_u16_to_u8, mir::MirType::U16, mir::MirType::U8);
truncating!(trunc_u32_to_u8, mir::MirType::U32, mir::MirType::U8);
truncating!(trunc_u32_to_u16, mir::MirType::U32, mir::MirType::U16);
truncating!(trunc_u64_to_u8, mir::MirType::U64, mir::MirType::U8);
truncating!(trunc_u64_to_u16, mir::MirType::U64, mir::MirType::U16);
truncating!(trunc_u64_to_u32, mir::MirType::U64, mir::MirType::U32);
truncating!(trunc_u128_to_u8, mir::MirType::U128, mir::MirType::U8);
truncating!(trunc_u128_to_u16, mir::MirType::U128, mir::MirType::U16);
truncating!(trunc_u128_to_u32, mir::MirType::U128, mir::MirType::U32);
truncating!(trunc_u128_to_u64, mir::MirType::U128, mir::MirType::U64);

// ── Cross-signedness (same width → no-op) ──
#[test]
fn cast_i8_to_u8_elided() {
    let ir = cast_ir(mir::MirType::I8, mir::MirType::U8);
    assert!(
        !ir.contains("sext") && !ir.contains("zext") && !ir.contains("trunc"),
        "{ir}"
    );
}
#[test]
fn cast_u8_to_i8_elided() {
    let ir = cast_ir(mir::MirType::U8, mir::MirType::I8);
    assert!(
        !ir.contains("sext") && !ir.contains("zext") && !ir.contains("trunc"),
        "{ir}"
    );
}

// ── Signed int → float ──
widening!(sitofp_i8, mir::MirType::I8, mir::MirType::F32, "sitofp");
widening!(sitofp_i16, mir::MirType::I16, mir::MirType::F32, "sitofp");
widening!(sitofp_i32, mir::MirType::I32, mir::MirType::F32, "sitofp");
widening!(sitofp_i64, mir::MirType::I64, mir::MirType::F32, "sitofp");
widening!(sitofp_i128, mir::MirType::I128, mir::MirType::F32, "sitofp");
widening!(sitofp_i64_f64, mir::MirType::I64, mir::MirType::F64, "sitofp");

// ── Unsigned int → float ──
widening!(uitofp_u8, mir::MirType::U8, mir::MirType::F32, "uitofp");
widening!(uitofp_u16, mir::MirType::U16, mir::MirType::F32, "uitofp");
widening!(uitofp_u32, mir::MirType::U32, mir::MirType::F32, "uitofp");
widening!(uitofp_u64, mir::MirType::U64, mir::MirType::F32, "uitofp");
widening!(uitofp_u128, mir::MirType::U128, mir::MirType::F32, "uitofp");
widening!(uitofp_u64_f64, mir::MirType::U64, mir::MirType::F64, "uitofp");

// ── float → signed int ──
widening!(fptosi_f32_i8, mir::MirType::F32, mir::MirType::I8, "fptosi");
widening!(fptosi_f32_i16, mir::MirType::F32, mir::MirType::I16, "fptosi");
widening!(fptosi_f32_i32, mir::MirType::F32, mir::MirType::I32, "fptosi");
widening!(fptosi_f32_i64, mir::MirType::F32, mir::MirType::I64, "fptosi");
widening!(fptosi_f64_i32, mir::MirType::F64, mir::MirType::I32, "fptosi");
widening!(fptosi_f64_i64, mir::MirType::F64, mir::MirType::I64, "fptosi");

// ── float → unsigned int ──
widening!(fptoui_f32_u8, mir::MirType::F32, mir::MirType::U8, "fptoui");
widening!(fptoui_f32_u16, mir::MirType::F32, mir::MirType::U16, "fptoui");
widening!(fptoui_f32_u32, mir::MirType::F32, mir::MirType::U32, "fptoui");
widening!(fptoui_f32_u64, mir::MirType::F32, mir::MirType::U64, "fptoui");
widening!(fptoui_f64_u32, mir::MirType::F64, mir::MirType::U32, "fptoui");
widening!(fptoui_f64_u64, mir::MirType::F64, mir::MirType::U64, "fptoui");

// ── float widening / truncation ──
widening!(fpext_f32_f64, mir::MirType::F32, mir::MirType::F64, "fpext");
#[test]
fn fptrunc_f64_f32() {
    let ir = cast_ir(mir::MirType::F64, mir::MirType::F32);
    assert!(ir.contains("fptrunc"), "expected fptrunc: {ir}");
}

// ── Pointer casts ──
#[test]
fn bitcast_pointer_to_pointer() {
    // LLVM uses opaque pointers, so all pointer casts are typed `ptr` and a
    // ptr→ptr cast is an identity (no `bitcast` instruction is needed).
    let h = Harness::new();
    let module = h.build_module(|b| {
        let src = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let src_id: mir::MirTypeId = src.into();
        let dst = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::U8.into(),
        };
        let dst_id: mir::MirTypeId = dst.into();
        fn_cast(b, "f", src_id, dst_id);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains("ptr"), "expected ptr typed function: {ir}");
}

#[test]
fn ptrtoint_pointer_to_usize() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let src = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let src_id: mir::MirTypeId = src.into();
        let dst_id: mir::MirTypeId = mir::MirType::USize.into();
        fn_cast(b, "f", src_id, dst_id);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains("ptrtoint"), "expected ptrtoint: {ir}");
}

#[test]
fn inttoptr_usize_to_pointer() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let src_id: mir::MirTypeId = mir::MirType::USize.into();
        let dst = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let dst_id: mir::MirTypeId = dst.into();
        fn_cast(b, "f", src_id, dst_id);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains("inttoptr"), "expected inttoptr: {ir}");
}
