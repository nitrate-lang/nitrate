//! Comprehensive width-coverage for binary operations not already covered by
//! `binops.rs`, specifically i128/u128 widths, narrow-width comparisons, and
//! rotations across more widths. Operands are dynamic to avoid constant folding.

use crate::test_common::{Harness, fn_binary};
use nitrate_mir::prelude as mir;

macro_rules! int_arith {
    ($($name:ident : $ity:expr, $rty:expr, $op:expr, $pattern:expr;)*) => {
        $(
            #[test]
            fn $name() {
                let h = Harness::new();
                let module = h.build_module(move |b| {
                    let ity: mir::MirTypeId = $ity.into();
                    let rty: mir::MirTypeId = $rty.into();
                    fn_binary(b, "bin", ity.clone(), ity, rty, $op);
                });
                let ir = h.ir(&module);
                assert!(h.verify(&module), "module invalid: {ir}");
                assert!(ir.contains($pattern), "expected `{}` in {ir}", $pattern);
            }
        )*
    };
}

// ── i128 full signed operator coverage ──
int_arith! {
    i128_add:   mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Add, "add i128";
    i128_sub:   mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Sub, "sub i128";
    i128_mul:   mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Mul, "mul i128";
    i128_and:   mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::And, "and i128";
    i128_or:    mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Or, "or i128";
    i128_xor:   mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Xor, "xor i128";
    i128_shl:   mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Shl, "shl i128";
    i128_sdiv:  mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Div, "sdiv i128";
    i128_srem:  mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Mod, "srem i128";
    i128_ashr:  mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Shr, "ashr i128";
    i128_slt:   mir::MirType::I128, mir::MirType::Bool, mir::MirBinaryOp::Lt, "icmp slt";
    i128_sgt:   mir::MirType::I128, mir::MirType::Bool, mir::MirBinaryOp::Gt, "icmp sgt";
    i128_sle:   mir::MirType::I128, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp sle";
    i128_sge:   mir::MirType::I128, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp sge";
    i128_eq:    mir::MirType::I128, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    i128_ne:    mir::MirType::I128, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
}

// ── u128 full unsigned operator coverage ──
int_arith! {
    u128_add:   mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Add, "add i128";
    u128_sub:   mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Sub, "sub i128";
    u128_mul:   mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Mul, "mul i128";
    u128_and:   mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::And, "and i128";
    u128_or:    mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Or, "or i128";
    u128_xor:   mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Xor, "xor i128";
    u128_shl:   mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Shl, "shl i128";
    u128_udiv:  mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Div, "udiv i128";
    u128_urem:  mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Mod, "urem i128";
    u128_lshr:  mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Shr, "lshr i128";
    u128_ult:   mir::MirType::U128, mir::MirType::Bool, mir::MirBinaryOp::Lt, "icmp ult";
    u128_ugt:   mir::MirType::U128, mir::MirType::Bool, mir::MirBinaryOp::Gt, "icmp ugt";
    u128_ule:   mir::MirType::U128, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp ule";
    u128_uge:   mir::MirType::U128, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp uge";
    u128_eq:    mir::MirType::U128, mir::MirType::Bool, mir::MirBinaryOp::Eq, "icmp eq";
    u128_ne:    mir::MirType::U128, mir::MirType::Bool, mir::MirBinaryOp::Ne, "icmp ne";
}

// ── Narrow signed comparisons (i8/i16) ──
int_arith! {
    i8_slt:  mir::MirType::I8,  mir::MirType::Bool, mir::MirBinaryOp::Lt,  "icmp slt";
    i8_sgt:  mir::MirType::I8,  mir::MirType::Bool, mir::MirBinaryOp::Gt,  "icmp sgt";
    i8_sle:  mir::MirType::I8,  mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp sle";
    i8_sge:  mir::MirType::I8,  mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp sge";
    i16_slt: mir::MirType::I16, mir::MirType::Bool, mir::MirBinaryOp::Lt,  "icmp slt";
    i16_sgt: mir::MirType::I16, mir::MirType::Bool, mir::MirBinaryOp::Gt,  "icmp sgt";
    i16_sle: mir::MirType::I16, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp sle";
    i16_sge: mir::MirType::I16, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp sge";
}

// ── Narrow unsigned comparisons (u8/u16) ──
int_arith! {
    u8_ult:  mir::MirType::U8,  mir::MirType::Bool, mir::MirBinaryOp::Lt,  "icmp ult";
    u8_ugt:  mir::MirType::U8,  mir::MirType::Bool, mir::MirBinaryOp::Gt,  "icmp ugt";
    u8_ule:  mir::MirType::U8,  mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp ule";
    u8_uge:  mir::MirType::U8,  mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp uge";
    u16_ult: mir::MirType::U16, mir::MirType::Bool, mir::MirBinaryOp::Lt,  "icmp ult";
    u16_ugt: mir::MirType::U16, mir::MirType::Bool, mir::MirBinaryOp::Gt,  "icmp ugt";
    u16_ule: mir::MirType::U16, mir::MirType::Bool, mir::MirBinaryOp::Lte, "icmp ule";
    u16_uge: mir::MirType::U16, mir::MirType::Bool, mir::MirBinaryOp::Gte, "icmp uge";
}

// ── Rotations across all remaining widths ──
int_arith! {
    rol_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Rol, "rol";
    ror_i8:  mir::MirType::I8,  mir::MirType::I8,  mir::MirBinaryOp::Ror, "ror";
    rol_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Rol, "rol";
    ror_i16: mir::MirType::I16, mir::MirType::I16, mir::MirBinaryOp::Ror, "ror";
    rol_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Rol, "rol";
    ror_i64: mir::MirType::I64, mir::MirType::I64, mir::MirBinaryOp::Ror, "ror";
    rol_i128: mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Rol, "rol";
    ror_i128: mir::MirType::I128, mir::MirType::I128, mir::MirBinaryOp::Ror, "ror";
    rol_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Rol, "rol";
    ror_u8:  mir::MirType::U8,  mir::MirType::U8,  mir::MirBinaryOp::Ror, "ror";
    rol_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Rol, "rol";
    ror_u16: mir::MirType::U16, mir::MirType::U16, mir::MirBinaryOp::Ror, "ror";
    rol_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Rol, "rol";
    ror_u64: mir::MirType::U64, mir::MirType::U64, mir::MirBinaryOp::Ror, "ror";
    rol_u128: mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Rol, "rol";
    ror_u128: mir::MirType::U128, mir::MirType::U128, mir::MirBinaryOp::Ror, "ror";
}

// ── Float comparison full coverage ──
macro_rules! float_cmp {
    ($($name:ident : $fty:expr, $op:expr, $pattern:expr;)*) => {
        $(
            #[test]
            fn $name() {
                let h = Harness::new();
                let module = h.build_module(move |b| {
                    let fty: mir::MirTypeId = $fty.into();
                    let bool: mir::MirTypeId = mir::MirType::Bool.into();
                    fn_binary(b, "fcmp", fty.clone(), fty, bool, $op);
                });
                let ir = h.ir(&module);
                assert!(h.verify(&module), "module invalid: {ir}");
                assert!(ir.contains($pattern), "expected `{}` in {ir}", $pattern);
            }
        )*
    };
}

float_cmp! {
    f64_olt: mir::MirType::F64, mir::MirBinaryOp::Lt,  "fcmp olt";
    f64_ogt: mir::MirType::F64, mir::MirBinaryOp::Gt,  "fcmp ogt";
    f64_ole: mir::MirType::F64, mir::MirBinaryOp::Lte, "fcmp ole";
    f64_oge: mir::MirType::F64, mir::MirBinaryOp::Gte, "fcmp oge";
    f64_oeq: mir::MirType::F64, mir::MirBinaryOp::Eq,  "fcmp oeq";
    f64_one: mir::MirType::F64, mir::MirBinaryOp::Ne,  "fcmp one";
    f32_oeq: mir::MirType::F32, mir::MirBinaryOp::Eq,  "fcmp oeq";
    f32_one: mir::MirType::F32, mir::MirBinaryOp::Ne,  "fcmp one";
    f32_ole: mir::MirType::F32, mir::MirBinaryOp::Lte, "fcmp ole";
    f32_oge: mir::MirType::F32, mir::MirBinaryOp::Gte, "fcmp oge";
}
