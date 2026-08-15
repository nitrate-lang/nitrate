//! Shift and logical-operator matrix across all integer widths.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

fn run(op: mir::MirBinaryOp, ty: mir::MirType, pattern: &str) {
    let h = Harness::new();
    let module = h.build_module(move |b| {
        let ty: mir::MirTypeId = ty.into();
        fn_binary(b, "bin", ty.clone(), ty.clone(), ty, op);
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "module invalid: {ir}");
    assert!(ir.contains(pattern), "expected `{}` in {ir}", pattern);
}

macro_rules! signed_shift {
    ($($name:ident : $ty:expr, $op:ident, $instr:expr;)*) => {
        $(
            #[test]
            fn $name() {
                run(mir::MirBinaryOp::$op, $ty, $instr);
            }
        )*
    };
}

macro_rules! unsigned_shift {
    ($($name:ident : $ty:expr, $op:ident, $instr:expr;)*) => {
        $(
            #[test]
            fn $name() {
                run(mir::MirBinaryOp::$op, $ty, $instr);
            }
        )*
    };
}

macro_rules! logic {
    ($($name:ident : $ty:expr, $op:ident, $instr:expr;)*) => {
        $(
            #[test]
            fn $name() {
                let h = Harness::new();
                let module = h.build_module(move |b| {
                    let ty: mir::MirTypeId = $ty.into();
                    let bool: mir::MirTypeId = mir::MirType::Bool.into();
                    fn_binary(b, "bin", ty.clone(), ty, bool, mir::MirBinaryOp::$op);
                });
                let ir = h.ir(&module);
                assert!(h.verify(&module), "module invalid: {ir}");
                assert!(ir.contains($instr), "expected `{}` in {ir}", $instr);
            }
        )*
    };
}

// Signed left/right shifts across every signed width.
signed_shift! {
    shl_i8:  mir::MirType::I8,  Shl, "shl i8";
    shl_i16: mir::MirType::I16, Shl, "shl i16";
    shl_i32: mir::MirType::I32, Shl, "shl i32";
    shl_i64: mir::MirType::I64, Shl, "shl i64";
    shl_i128: mir::MirType::I128, Shl, "shl i128";
    ashr_i8:  mir::MirType::I8,  Shr, "ashr i8";
    ashr_i16: mir::MirType::I16, Shr, "ashr i16";
    ashr_i32: mir::MirType::I32, Shr, "ashr i32";
    ashr_i64: mir::MirType::I64, Shr, "ashr i64";
    ashr_i128: mir::MirType::I128, Shr, "ashr i128";
}

// Unsigned left/right shifts across every unsigned width.
unsigned_shift! {
    shl_u8:  mir::MirType::U8,  Shl, "shl i8";
    shl_u16: mir::MirType::U16, Shl, "shl i16";
    shl_u32: mir::MirType::U32, Shl, "shl i32";
    shl_u64: mir::MirType::U64, Shl, "shl i64";
    shl_u128: mir::MirType::U128, Shl, "shl i128";
    lshr_u8:  mir::MirType::U8,  Shr, "lshr i8";
    lshr_u16: mir::MirType::U16, Shr, "lshr i16";
    lshr_u32: mir::MirType::U32, Shr, "lshr i32";
    lshr_u64: mir::MirType::U64, Shr, "lshr i64";
    lshr_u128: mir::MirType::U128, Shr, "lshr i128";
}

// Logical operators across all integer widths (returns bool).
logic! {
    land_i8:  mir::MirType::I8,  LogicAnd, "and i1";
    land_i16: mir::MirType::I16, LogicAnd, "and i1";
    land_i32: mir::MirType::I32, LogicAnd, "and i1";
    land_i64: mir::MirType::I64, LogicAnd, "and i1";
    land_i128: mir::MirType::I128, LogicAnd, "and i1";
    land_u8:  mir::MirType::U8,  LogicAnd, "and i1";
    land_u16: mir::MirType::U16, LogicAnd, "and i1";
    land_u32: mir::MirType::U32, LogicAnd, "and i1";
    land_u64: mir::MirType::U64, LogicAnd, "and i1";
    land_u128: mir::MirType::U128, LogicAnd, "and i1";
    lor_i8:  mir::MirType::I8,  LogicOr, "or i1";
    lor_i16: mir::MirType::I16, LogicOr, "or i1";
    lor_i32: mir::MirType::I32, LogicOr, "or i1";
    lor_i64: mir::MirType::I64, LogicOr, "or i1";
    lor_i128: mir::MirType::I128, LogicOr, "or i1";
    lor_u8:  mir::MirType::U8,  LogicOr, "or i1";
    lor_u16: mir::MirType::U16, LogicOr, "or i1";
    lor_u32: mir::MirType::U32, LogicOr, "or i1";
    lor_u64: mir::MirType::U64, LogicOr, "or i1";
    lor_u128: mir::MirType::U128, LogicOr, "or i1";
}
