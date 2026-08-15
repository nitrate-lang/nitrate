//! Cross-signedness cast coverage: every signed→unsigned and unsigned→signed
//! cast with differing widths. These verify the sign/zero-extension source
//! signedness and truncation behaviors.

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

// Signed → unsigned widening (sext) for differing widths.
widening!(i8_to_u16_sext, mir::MirType::I8, mir::MirType::U16, "sext");
widening!(i8_to_u32_sext, mir::MirType::I8, mir::MirType::U32, "sext");
widening!(i8_to_u64_sext, mir::MirType::I8, mir::MirType::U64, "sext");
widening!(i8_to_u128_sext, mir::MirType::I8, mir::MirType::U128, "sext");
widening!(i16_to_u32_sext, mir::MirType::I16, mir::MirType::U32, "sext");
widening!(i16_to_u64_sext, mir::MirType::I16, mir::MirType::U64, "sext");
widening!(i16_to_u128_sext, mir::MirType::I16, mir::MirType::U128, "sext");
widening!(i32_to_u64_sext, mir::MirType::I32, mir::MirType::U64, "sext");
widening!(i32_to_u128_sext, mir::MirType::I32, mir::MirType::U128, "sext");
widening!(i64_to_u128_sext, mir::MirType::I64, mir::MirType::U128, "sext");

// Unsigned → signed widening (zext) for differing widths.
widening!(u8_to_i16_zext, mir::MirType::U8, mir::MirType::I16, "zext");
widening!(u8_to_i32_zext, mir::MirType::U8, mir::MirType::I32, "zext");
widening!(u8_to_i64_zext, mir::MirType::U8, mir::MirType::I64, "zext");
widening!(u8_to_i128_zext, mir::MirType::U8, mir::MirType::I128, "zext");
widening!(u16_to_i32_zext, mir::MirType::U16, mir::MirType::I32, "zext");
widening!(u16_to_i64_zext, mir::MirType::U16, mir::MirType::I64, "zext");
widening!(u16_to_i128_zext, mir::MirType::U16, mir::MirType::I128, "zext");
widening!(u32_to_i64_zext, mir::MirType::U32, mir::MirType::I64, "zext");
widening!(u32_to_i128_zext, mir::MirType::U32, mir::MirType::I128, "zext");
widening!(u64_to_i128_zext, mir::MirType::U64, mir::MirType::I128, "zext");

// Signed → unsigned truncation (same signedness, differing width).
truncating!(i16_to_u8_trunc, mir::MirType::I16, mir::MirType::U8);
truncating!(i32_to_u8_trunc, mir::MirType::I32, mir::MirType::U8);
truncating!(i32_to_u16_trunc, mir::MirType::I32, mir::MirType::U16);
truncating!(i64_to_u8_trunc, mir::MirType::I64, mir::MirType::U8);
truncating!(i64_to_u16_trunc, mir::MirType::I64, mir::MirType::U16);
truncating!(i64_to_u32_trunc, mir::MirType::I64, mir::MirType::U32);
truncating!(i128_to_u8_trunc, mir::MirType::I128, mir::MirType::U8);
truncating!(i128_to_u16_trunc, mir::MirType::I128, mir::MirType::U16);
truncating!(i128_to_u32_trunc, mir::MirType::I128, mir::MirType::U32);
truncating!(i128_to_u64_trunc, mir::MirType::I128, mir::MirType::U64);

// Unsigned → signed truncation (same signedness, differing width).
truncating!(u16_to_i8_trunc, mir::MirType::U16, mir::MirType::I8);
truncating!(u32_to_i8_trunc, mir::MirType::U32, mir::MirType::I8);
truncating!(u32_to_i16_trunc, mir::MirType::U32, mir::MirType::I16);
truncating!(u64_to_i8_trunc, mir::MirType::U64, mir::MirType::I8);
truncating!(u64_to_i16_trunc, mir::MirType::U64, mir::MirType::I16);
truncating!(u64_to_i32_trunc, mir::MirType::U64, mir::MirType::I32);
truncating!(u128_to_i8_trunc, mir::MirType::U128, mir::MirType::I8);
truncating!(u128_to_i16_trunc, mir::MirType::U128, mir::MirType::I16);
truncating!(u128_to_i32_trunc, mir::MirType::U128, mir::MirType::I32);
truncating!(u128_to_i64_trunc, mir::MirType::U128, mir::MirType::I64);
