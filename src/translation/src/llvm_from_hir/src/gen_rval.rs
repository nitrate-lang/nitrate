use inkwell::{
    basic_block::BasicBlock,
    values::{BasicValueEnum, PointerValue},
};

use crate::ty::gen_ty;
use interned_string::IString;
use nitrate_hir::{ValueId, prelude as hir};
use nitrate_hir_get_type::HirGetType;
use nitrate_llvm::LLVMContext;
use std::collections::HashMap;
use std::ops::Deref;

pub struct RvalGenCtx<'ctx, 'store, 'tab, 'builder> {
    pub store: &'store hir::Store,
    pub tab: &'tab hir::SymbolTab,
    pub llvm: &'ctx LLVMContext,

    pub bb: &'builder inkwell::builder::Builder<'ctx>,
    pub locals: HashMap<IString, PointerValue<'ctx>>,
    pub default_continue_target: Vec<(Option<IString>, BasicBlock<'ctx>)>,
    pub default_break_target: Vec<(Option<IString>, BasicBlock<'ctx>)>,
}

#[derive(Debug)]
pub enum RvalError {
    OperandTypeCombinationError {
        #[allow(dead_code)]
        operation_name: &'static str,
    },
}

/**
 * The Unit Value is an empty struct
 */
fn gen_rval_lit_unit<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.const_struct(&[], false).into())
}

/**
 * Direct correspondence to LLVM i1 type.
 * No sign extension is performed.
 */
fn gen_rval_lit_bool<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: bool,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    match value {
        true => Ok(ctx.llvm.bool_type().const_int(1, false).into()),
        false => Ok(ctx.llvm.bool_type().const_int(0, false).into()),
    }
}

/**
 * Direct correspondence to LLVM i8 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i8<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: i8,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i8_type().const_int(value as u64, true).into())
}

/**
 * Direct correspondence to LLVM i16 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i16<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: i16,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i16_type().const_int(value as u64, true).into())
}

/**
 * Direct correspondence to LLVM i32 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i32<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: i32,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i32_type().const_int(value as u64, true).into())
}

/**
 * Direct correspondence to LLVM i64 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i64<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: i64,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i64_type().const_int(value as u64, true).into())
}

/**
 * Direct correspondence to LLVM i128 type.
 * Sign extension is not performed because the value is constructed
 * from its low and high parts directly.
 */
fn gen_rval_lit_i128<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: i128,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;

    let value = ctx
        .llvm
        .i128_type()
        .const_int_arbitrary_precision(&[low, high])
        .into();

    Ok(value)
}

/**
 * Direct correspondence to LLVM i8 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u8<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: u8,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i8_type().const_int(value as u64, false).into())
}

/**
 * Direct correspondence to LLVM i16 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u16<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: u16,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i16_type().const_int(value as u64, false).into())
}

/**
 * Direct correspondence to LLVM i32 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u32<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: u32,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i32_type().const_int(value as u64, false).into())
}

/**
 * Direct correspondence to LLVM i64 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u64<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: u64,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.i64_type().const_int(value, false).into())
}

/**
 * Direct correspondence to LLVM i128 type (2's complement).
 * Sign extension is not performed because the value is constructed
 * from its low and high parts directly.
 */
fn gen_rval_lit_u128<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: u128,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;

    let value = ctx
        .llvm
        .i128_type()
        .const_int_arbitrary_precision(&[low, high])
        .into();

    Ok(value)
}

/**
 * Direct correspondence to LLVM f32 type.
 */
fn gen_rval_lit_f32<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: f32,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.f32_type().const_float(value as f64).into())
}

/**
 * Direct correspondence to LLVM f64 type.
 */
fn gen_rval_lit_f64<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: f64,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.f64_type().const_float(value).into())
}

/**
 * Create a LLVM string constant byte-array.
 * No null terminator is added.
 */
fn gen_rval_lit_string<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: &str,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.const_string(value.as_bytes(), false).into())
}

/**
 * Create a LLVM string constant byte-array.
 * No null terminator is added.
 */
fn gen_rval_lit_bstring<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: &[u8],
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    Ok(ctx.llvm.const_string(value, false).into())
}

/**
 * Addition:
 *
 * Integers:
 * - The result is modulo 2^n, where n is the bit width of the type.
 * - https://llvm.org/docs/LangRef.html#add-instruction
 *
 * Floating-point:
 * - Follows the IEEE 754 standard for floating-point arithmetic.
 * - https://llvm.org/docs/LangRef.html#fadd-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_add<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fadd = ctx
            .bb
            .build_float_add(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        Ok(fadd.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let iadd = ctx
            .bb
            .build_int_add(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        Ok(iadd.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "addition",
        })
    }
}

/**
 * Subtraction:
 *
 * Integers:
 * - The result is modulo 2^n, where n is the bit width of the type.
 * - https://llvm.org/docs/LangRef.html#sub-instruction
 *
 * Floating-point:
 * - Follows the IEEE 754 standard for floating-point arithmetic.
 * - https://llvm.org/docs/LangRef.html#fsub-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_sub<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fsub = ctx
            .bb
            .build_float_sub(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        Ok(fsub.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let isub = ctx
            .bb
            .build_int_sub(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        Ok(isub.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "subtraction",
        })
    }
}

/**
 * Multiplication:
 *
 * Integers:
 * - The result is modulo 2^n, where n is the bit width of the type.
 * - https://llvm.org/docs/LangRef.html#mul-instruction
 *
 * Floating-point:
 * - Follows the IEEE 754 standard for floating-point arithmetic.
 * - https://llvm.org/docs/LangRef.html#fmul-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_mul<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fmul = ctx
            .bb
            .build_float_mul(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        Ok(fmul.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let imul = ctx
            .bb
            .build_int_mul(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        Ok(imul.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "multiplication",
        })
    }
}

/**
 * Division:
 *
 * Signed Integers:
 * - // TODO: define behavior for division by zero
 * - // TODO: define behavior for overflow
 * - https://llvm.org/docs/LangRef.html#sdiv-instruction
 *
 * Unsigned Integers:
 * - // TODO: define behavior for division by zero
 * - https://llvm.org/docs/LangRef.html#udiv-instruction
 *
 * Floating-point:
 * - Follows the IEEE 754 standard for floating-point arithmetic.
 * - https://llvm.org/docs/LangRef.html#fdiv-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_div<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_lhs = gen_rval(ctx, lhs)?;
    let llvm_rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fdiv = ctx
            .bb
            .build_float_div(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "")
            .unwrap();

        Ok(fdiv.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .get_type(&ctx.store, &ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let div = if is_signed {
            ctx.bb
                .build_int_signed_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
                .unwrap()
        } else {
            ctx.bb
                .build_int_unsigned_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
                .unwrap()
        };

        Ok(div.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "division",
        })
    }
}

/**
 * Remainder:
 *
 * Signed Integers:
 * - // TODO: define behavior for remainder by zero
 * - // TODO: define behavior for overflow
 * - https://llvm.org/docs/LangRef.html#srem-instruction
 *
 * Unsigned Integers:
 * - // TODO: define behavior for remainder by zero
 * - https://llvm.org/docs/LangRef.html#urem-instruction
 *
 * Floating-point:
 * - Follows the IEEE 754 standard for floating-point arithmetic.
 * - https://llvm.org/docs/LangRef.html#frem-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_rem<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_lhs = gen_rval(ctx, lhs)?;
    let llvm_rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let frem = ctx
            .bb
            .build_float_rem(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "")
            .unwrap();

        Ok(frem.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .get_type(&ctx.store, &ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let rem = if is_signed {
            ctx.bb
                .build_int_signed_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
                .unwrap()
        } else {
            ctx.bb
                .build_int_unsigned_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
                .unwrap()
        };

        Ok(rem.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "remainder",
        })
    }
}

/**
 * Bitwise AND operation.
 *
 * Integers:
 * - Performs a bitwise AND operation on each corresponding bit of the operands.
 * - https://llvm.org/docs/LangRef.html#and-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_and<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let and = ctx
            .bb
            .build_and(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        Ok(and.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "bitwise and",
        })
    }
}

/**
 * Bitwise OR operation.
 *
 * Integers:
 * - Performs a bitwise OR operation on each corresponding bit of the operands.
 * - https://llvm.org/docs/LangRef.html#or-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_or<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let or = ctx
            .bb
            .build_or(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        Ok(or.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "bitwise or",
        })
    }
}

/**
 * Bitwise XOR operation.
 *
 * Integers:
 * - Performs a bitwise XOR operation on each corresponding bit of the operands.
 * - https://llvm.org/docs/LangRef.html#xor-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_xor<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let xor = ctx
            .bb
            .build_xor(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        Ok(xor.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "bitwise xor",
        })
    }
}

/**
 * Bitwise left-shift operation.
 *
 * Integers:
 * - Performs a bitwise left-shift operation on each corresponding bit of the operands.
 * - https://llvm.org/docs/LangRef.html#shl-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_shl<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let shl = ctx
            .bb
            .build_left_shift(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        Ok(shl.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "bitwise left-shift",
        })
    }
}

/**
 * Bitwise right-shift operation.
 *
 * Integers:
 * - Performs a bitwise right-shift operation on each corresponding bit of the operands.
 * - https://llvm.org/docs/LangRef.html#shr-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_shr<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_lhs = gen_rval(ctx, lhs)?;
    let llvm_rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let sign_extend = lhs
            .get_type(&ctx.store, &ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let shr = ctx
            .bb
            .build_right_shift(
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                sign_extend,
                "",
            )
            .unwrap();

        Ok(shr.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "bitwise right-shift",
        })
    }
}

/**
 * Bitwise rotate left operation formula:
 * rol(x, n) = (x << (n % bit_width)) | (x >> ((bit_width - (n % bit_width)) % bit_width))
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_rol<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if !lhs_ty.is_int_type() || !rhs_ty.is_int_type() {
        return Err(RvalError::OperandTypeCombinationError {
            operation_name: "bitwise rotate left",
        });
    }

    let bit_width = ctx.llvm.target_data().get_store_size(&lhs.get_type()) * 8;
    let bit_width_i32 = ctx.llvm.i32_type().const_int(bit_width as u64, false);

    let reduced_n = ctx
        .bb
        .build_int_unsigned_rem(rhs.into_int_value(), bit_width_i32, "")
        .unwrap();

    let shl = ctx
        .bb
        .build_left_shift(lhs.into_int_value(), reduced_n, "")
        .unwrap();

    let sub = ctx.bb.build_int_sub(bit_width_i32, reduced_n, "").unwrap();

    let sub_reduced = ctx
        .bb
        .build_int_unsigned_rem(sub, bit_width_i32, "")
        .unwrap();

    let shr = ctx
        .bb
        .build_right_shift(lhs.into_int_value(), sub_reduced, false, "")
        .unwrap();

    let or = ctx.bb.build_or(shl, shr, "").unwrap();

    Ok(or.into())
}

/**
 * Bitwise rotate right operation formula:
 * ror(x, n) = (x >> (n % bit_width)) | (x << ((bit_width - (n % bit_width)) % bit_width))
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_ror<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if !lhs_ty.is_int_type() || !rhs_ty.is_int_type() {
        return Err(RvalError::OperandTypeCombinationError {
            operation_name: "bitwise rotate right",
        });
    }

    let bit_width = ctx.llvm.target_data().get_store_size(&lhs.get_type()) * 8;
    let bit_width_i32 = ctx.llvm.i32_type().const_int(bit_width as u64, false);

    let reduced_n = ctx
        .bb
        .build_int_unsigned_rem(rhs.into_int_value(), bit_width_i32, "")
        .unwrap();

    let shr = ctx
        .bb
        .build_right_shift(lhs.into_int_value(), reduced_n, false, "")
        .unwrap();

    let sub = ctx.bb.build_int_sub(bit_width_i32, reduced_n, "").unwrap();

    let sub_reduced = ctx
        .bb
        .build_int_unsigned_rem(sub, bit_width_i32, "")
        .unwrap();

    let shl = ctx
        .bb
        .build_left_shift(lhs.into_int_value(), sub_reduced, "")
        .unwrap();

    let or = ctx.bb.build_or(shr, shl, "").unwrap();

    Ok(or.into())
}

/**
 * Logical AND operation:
 *
 * Evaluates the LHS; if true, evaluates the RHS; otherwise, returns false.
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_land<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let parent_function = ctx.bb.get_insert_block().unwrap().get_parent().unwrap();
    let bool = ctx.llvm.bool_type();

    let rhs_bb = ctx.llvm.append_basic_block(parent_function, "land_rhs");
    let end_bb = ctx.llvm.append_basic_block(parent_function, "land_join");

    /**************************************************************************/
    // 1. Allocate space for the result
    let land_result = ctx.bb.build_alloca(bool, "land_result").unwrap();

    /**************************************************************************/
    // 2. Evaluate LHS; if true, skip RHS
    let lhs_val = gen_rval(ctx, lhs)?;
    let lhs_val_ty = lhs_val.get_type();
    if !lhs_val_ty.is_int_type() {
        return Err(RvalError::OperandTypeCombinationError {
            operation_name: "logical AND",
        });
    }

    ctx.bb.build_store(land_result, lhs_val).unwrap();
    ctx.bb
        .build_conditional_branch(lhs_val.into_int_value(), rhs_bb, end_bb)
        .unwrap();

    /**************************************************************************/
    // 3. Evaluate RHS
    ctx.bb.position_at_end(rhs_bb);
    let rhs_val = gen_rval(ctx, rhs)?;
    ctx.bb.build_store(land_result, rhs_val).unwrap();
    ctx.bb.build_unconditional_branch(end_bb).unwrap();

    /**************************************************************************/
    // 4. Join block and load result
    ctx.bb.position_at_end(end_bb);
    let load = ctx.bb.build_load(bool, land_result, "land_load").unwrap();
    Ok(load.into())
}

/**
 * Logical OR operation:
 *
 * Evaluates the LHS; if false, evaluates the RHS; otherwise, returns true.
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_lor<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let parent_function = ctx.bb.get_insert_block().unwrap().get_parent().unwrap();
    let bool = ctx.llvm.bool_type();

    let rhs_bb = ctx.llvm.append_basic_block(parent_function, "lor_rhs");
    let end_bb = ctx.llvm.append_basic_block(parent_function, "lor_join");

    /**************************************************************************/
    // 1. Allocate space for the result
    let lor_result = ctx.bb.build_alloca(bool, "lor_result").unwrap();

    /**************************************************************************/
    // 2. Evaluate LHS; if true, skip RHS
    let lhs_val = gen_rval(ctx, lhs)?;
    let lhs_val_ty = lhs_val.get_type();
    if !lhs_val_ty.is_int_type() {
        return Err(RvalError::OperandTypeCombinationError {
            operation_name: "logical OR",
        });
    }

    ctx.bb.build_store(lor_result, lhs_val).unwrap();
    ctx.bb
        .build_conditional_branch(lhs_val.into_int_value(), end_bb, rhs_bb)
        .unwrap();

    /**************************************************************************/
    // 3. Evaluate RHS
    ctx.bb.position_at_end(rhs_bb);
    let rhs_val = gen_rval(ctx, rhs)?;
    ctx.bb.build_store(lor_result, rhs_val).unwrap();
    ctx.bb.build_unconditional_branch(end_bb).unwrap();

    /**************************************************************************/
    // 4. Join block and load result
    ctx.bb.position_at_end(end_bb);
    let load = ctx.bb.build_load(bool, lor_result, "lor_load").unwrap();
    Ok(load.into())
}

/**
 * Less than operation:
 *
 * Signed integers:
 * - Signed integers use signed less-than comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'slt'
 *
 * Unsigned integers:
 * - Unsigned integers use unsigned less-than comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'ult'
 *
 * Floating-point:
 * - Floating-point numbers use ordered less-than comparison.
 * - https://llvm.org/docs/LangRef.html#fcmp-instruction with predicate 'olt'
 */
fn gen_rval_lt<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_lhs = gen_rval(ctx, lhs)?;
    let llvm_rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx
            .bb
            .build_float_compare(
                inkwell::FloatPredicate::OLT,
                llvm_lhs.into_float_value(),
                llvm_rhs.into_float_value(),
                "",
            )
            .unwrap();

        Ok(fcmp.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .get_type(&ctx.store, &ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::SLT,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        } else {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::ULT,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        };

        Ok(cmp.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "less than comparison",
        })
    }
}

/**
 * Greater than operation:
 *
 * Signed integers:
 * - Signed integers use signed greater-than comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'sgt'
 *
 * Unsigned integers:
 * - Unsigned integers use unsigned greater-than comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'ugt'
 *
 * Floating-point:
 * - Floating-point numbers use ordered greater-than comparison.
 * - https://llvm.org/docs/LangRef.html#fcmp-instruction with predicate 'ogt'
 */
fn gen_rval_gt<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_lhs = gen_rval(ctx, lhs)?;
    let llvm_rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx
            .bb
            .build_float_compare(
                inkwell::FloatPredicate::OGT,
                llvm_lhs.into_float_value(),
                llvm_rhs.into_float_value(),
                "",
            )
            .unwrap();

        Ok(fcmp.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .get_type(&ctx.store, &ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::SGT,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        } else {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::UGT,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        };

        Ok(cmp.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "greater than comparison",
        })
    }
}

/**
 * Less than or equal operation:
 *
 * Signed integers:
 * - Signed integers use signed less-than-or-equal comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'sle'
 *
 * Unsigned integers:
 * - Unsigned integers use unsigned less-than-or-equal comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'ule'
 *
 * Floating-point:
 * - Floating-point numbers use ordered less-than-or-equal comparison.
 * - https://llvm.org/docs/LangRef.html#fcmp-instruction with predicate 'ole'
 */
fn gen_rval_lte<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_lhs = gen_rval(ctx, lhs)?;
    let llvm_rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx
            .bb
            .build_float_compare(
                inkwell::FloatPredicate::OLE,
                llvm_lhs.into_float_value(),
                llvm_rhs.into_float_value(),
                "",
            )
            .unwrap();

        Ok(fcmp.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .get_type(&ctx.store, &ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::SLE,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        } else {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::ULE,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        };

        Ok(cmp.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "less than or equal comparison",
        })
    }
}

/**
 * Greater than or equal operation:
 *
 * Signed integers:
 * - Signed integers use signed greater-than-or-equal comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'sge'
 *
 * Unsigned integers:
 * - Unsigned integers use unsigned greater-than-or-equal comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'uge'
 *
 * Floating-point:
 * - Floating-point numbers use ordered greater-than-or-equal comparison.
 * - https://llvm.org/docs/LangRef.html#fcmp-instruction with predicate 'oge'
 */
fn gen_rval_gte<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_lhs = gen_rval(ctx, lhs)?;
    let llvm_rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx
            .bb
            .build_float_compare(
                inkwell::FloatPredicate::OGE,
                llvm_lhs.into_float_value(),
                llvm_rhs.into_float_value(),
                "",
            )
            .unwrap();

        Ok(fcmp.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .get_type(&ctx.store, &ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::SGE,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        } else {
            ctx.bb
                .build_int_compare(
                    inkwell::IntPredicate::UGE,
                    llvm_lhs.into_int_value(),
                    llvm_rhs.into_int_value(),
                    "",
                )
                .unwrap()
        };

        Ok(cmp.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "greater than or equal comparison",
        })
    }
}

/**
 * Equality operation:
 *
 * Signed and unsigned integers:
 * - Signed integers use signed equality comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'eq'
 *
 * Floating-point:
 * - Floating-point numbers use ordered equality comparison.
 * - https://llvm.org/docs/LangRef.html#fcmp-instruction with predicate 'oeq'
 */
fn gen_rval_eq<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx
            .bb
            .build_float_compare(
                inkwell::FloatPredicate::OEQ,
                lhs.into_float_value(),
                rhs.into_float_value(),
                "",
            )
            .unwrap();

        Ok(fcmp.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let icmp = ctx
            .bb
            .build_int_compare(
                inkwell::IntPredicate::EQ,
                lhs.into_int_value(),
                rhs.into_int_value(),
                "",
            )
            .unwrap();

        Ok(icmp.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "equality comparison",
        })
    }
}

/**
 * Inequality operation:
 *
 * Signed and unsigned integers:
 * - Signed integers use signed inequality comparison.
 * - https://llvm.org/docs/LangRef.html#icmp-instruction with predicate 'ne'
 *
 * Floating-point:
 * - Floating-point numbers use ordered inequality comparison.
 * - https://llvm.org/docs/LangRef.html#fcmp-instruction with predicate 'one'
 */
fn gen_rval_ne<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let lhs = gen_rval(ctx, lhs)?;
    let rhs = gen_rval(ctx, rhs)?;
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx
            .bb
            .build_float_compare(
                inkwell::FloatPredicate::ONE,
                lhs.into_float_value(),
                rhs.into_float_value(),
                "",
            )
            .unwrap();

        Ok(fcmp.into())
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let icmp = ctx
            .bb
            .build_int_compare(
                inkwell::IntPredicate::NE,
                lhs.into_int_value(),
                rhs.into_int_value(),
                "",
            )
            .unwrap();

        Ok(icmp.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "inequality comparison",
        })
    }
}

/**
 * Unary addition operation.
 *
 * This operation is effectively a no-op and simply returns the operand as is.
 */
fn gen_rval_unary_add<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    operand: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_operand = gen_rval(ctx, operand)?;
    Ok(llvm_operand)
}

/**
 * Unary subtraction operation.
 *
 * This operation negates the operand.
 * - For floating-point types, it uses the LLVM `fneg` instruction.
 * - For integer types, it subtracts the operand from zero.
 */
fn gen_rval_unary_sub<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    operand: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_operand = gen_rval(ctx, operand)?;
    let operand_ty = llvm_operand.get_type();

    if operand_ty.is_float_type() {
        let fneg = ctx
            .bb
            .build_float_neg(llvm_operand.into_float_value(), "")
            .unwrap();

        Ok(fneg.into())
    } else if operand_ty.is_int_type() {
        let zero = operand_ty.into_int_type().const_int(0, false);

        let neg = ctx
            .bb
            .build_int_sub(zero, llvm_operand.into_int_value(), "")
            .unwrap();

        Ok(neg.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "unary subtraction",
        })
    }
}

/**
 * Unary logical NOT operation.
 *
 * This operation inverts the boolean value of the operand.
 * - For integer types, it uses the LLVM `not` instruction.
 */
fn gen_rval_unary_not<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    operand: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let llvm_operand = gen_rval(ctx, operand)?;
    let operand_ty = llvm_operand.get_type();

    if operand_ty.is_int_type() {
        let not = ctx.bb.build_not(llvm_operand.into_int_value(), "").unwrap();

        Ok(not.into())
    } else {
        Err(RvalError::OperandTypeCombinationError {
            operation_name: "unary not",
        })
    }
}

/**
 * // TODO: add documentation
 */
fn gen_rval_struct_object<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _struct_path: &IString,
    _fields: &[(IString, ValueId)],
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement struct object codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_enum_variant<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _enum_path: &IString,
    _variant_name: &IString,
    _value: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement enum variant codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_field_access<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _struct_value: &hir::Value,
    _field_name: &IString,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement field access codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_index_access<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _collection: &hir::Value,
    _index: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement index access codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_assign<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _place: &hir::Value,
    _value: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement assignment codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_deref<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _place: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement dereference codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_cast<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _value: &hir::Value,
    _target_type: &hir::Type,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_borrow<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _exclusive: bool,
    _mutable: bool,
    _place: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement borrow codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_list<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _elements: &[hir::Value],
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement list codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_tuple<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _elements: &[hir::Value],
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement tuple codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_if<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    condition: &hir::Value,
    true_branch: &hir::Block,
    false_branch: Option<&hir::Block>,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    let top_block = ctx.bb.get_insert_block().unwrap();
    let current_function = top_block.get_parent().unwrap();

    if let Some(false_branch) = false_branch {
        let then_bb = ctx.llvm.append_basic_block(current_function, "if_then");
        let else_bb = ctx.llvm.append_basic_block(current_function, "if_else");
        let join_bb = ctx.llvm.append_basic_block(current_function, "if_join");

        let true_branch_ty = true_branch.get_type(ctx.store, ctx.tab).unwrap();
        let false_branch_ty = false_branch.get_type(ctx.store, ctx.tab).unwrap();

        let if_result_ty = if true_branch_ty.is_diverging() {
            gen_ty(&false_branch_ty, ctx.llvm, ctx.store, ctx.tab)
        } else {
            gen_ty(&true_branch_ty, ctx.llvm, ctx.store, ctx.tab)
        };

        let result = ctx.bb.build_alloca(if_result_ty, "if_result").unwrap();
        let cond_val = gen_rval(ctx, condition)?;

        ctx.bb
            .build_conditional_branch(cond_val.into_int_value(), then_bb, else_bb)
            .unwrap();

        /************************************************************************/
        // True branch
        ctx.bb.position_at_end(then_bb);
        if true_branch_ty.is_diverging() {
            gen_block(ctx, true_branch)?;
        } else {
            let result_val = gen_block_rval(ctx, true_branch)?;
            ctx.bb.build_store(result, result_val).unwrap();
            ctx.bb.build_unconditional_branch(join_bb).unwrap();
        }

        /************************************************************************/
        // False branch
        ctx.bb.position_at_end(else_bb);
        if false_branch_ty.is_diverging() {
            gen_block(ctx, false_branch)?;
        } else {
            let result_val = gen_block_rval(ctx, false_branch)?;
            ctx.bb.build_store(result, result_val).unwrap();
            ctx.bb.build_unconditional_branch(join_bb).unwrap();
        }

        /************************************************************************/
        // Join block
        ctx.bb.position_at_end(join_bb);
        let load = ctx.bb.build_load(if_result_ty, result, "if_load").unwrap();
        return Ok(load.into());
    }

    let then_bb = ctx.llvm.append_basic_block(current_function, "if_then");
    let join_bb = ctx.llvm.append_basic_block(current_function, "if_join");

    let true_branch_ty = true_branch.get_type(&ctx.store, &ctx.tab).unwrap();

    let cond_val = gen_rval(ctx, condition)?;
    ctx.bb
        .build_conditional_branch(cond_val.into_int_value(), then_bb, join_bb)
        .unwrap();

    ctx.bb.position_at_end(then_bb);
    gen_block(ctx, true_branch)?;
    if !true_branch_ty.is_diverging() {
        ctx.bb.build_unconditional_branch(join_bb).unwrap();
    }

    ctx.bb.position_at_end(join_bb);

    gen_rval_lit_unit(ctx)
}

/**
 * // TODO: add documentation
 */
fn gen_while<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    condition: &hir::Value,
    body: &hir::Block,
) -> Result<(), RvalError> {
    let top_block = ctx.bb.get_insert_block().unwrap();
    let current_function = top_block.get_parent().unwrap();

    let cond_bb = ctx.llvm.append_basic_block(current_function, "while_cond");
    let body_bb = ctx.llvm.append_basic_block(current_function, "while_body");
    let join_bb = ctx.llvm.append_basic_block(current_function, "while_join");

    ctx.default_continue_target.push((None, cond_bb));
    ctx.default_break_target.push((None, join_bb));

    ctx.bb.build_unconditional_branch(cond_bb).unwrap();

    /************************************************************************/
    // 1. While loop condition check
    ctx.bb.position_at_end(cond_bb);
    let cond_val = gen_rval(ctx, condition)?;
    ctx.bb
        .build_conditional_branch(cond_val.into_int_value(), body_bb, join_bb)
        .unwrap();

    /************************************************************************/
    // 2. While loop body
    ctx.bb.position_at_end(body_bb);
    gen_block(ctx, body)?;
    if !body.get_type(&ctx.store, &ctx.tab).unwrap().is_diverging() {
        ctx.bb.build_unconditional_branch(cond_bb).unwrap();
    }

    ctx.bb.position_at_end(join_bb);

    ctx.default_continue_target.pop();
    ctx.default_break_target.pop();

    Ok(())
}

/**
 * // TODO: add documentation
 */
fn gen_loop<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    body: &hir::Block,
) -> Result<(), RvalError> {
    let top_block = ctx.bb.get_insert_block().unwrap();
    let current_function = top_block.get_parent().unwrap();

    let body_bb = ctx.llvm.append_basic_block(current_function, "loop_body");
    let join_bb = ctx.llvm.append_basic_block(current_function, "loop_join");

    ctx.default_continue_target.push((None, body_bb));
    ctx.default_break_target.push((None, join_bb));

    /************************************************************************/
    // 1. Loop body
    ctx.bb.position_at_end(body_bb);
    gen_block(ctx, body)?;
    if !body.get_type(&ctx.store, &ctx.tab).unwrap().is_diverging() {
        ctx.bb.build_unconditional_branch(body_bb).unwrap();
    }

    /************************************************************************/
    // 2. Join block
    ctx.bb.position_at_end(join_bb);

    ctx.default_continue_target.pop();
    ctx.default_break_target.pop();

    Ok(())
}

/**
 * // TODO: add documentation
 */
fn gen_break<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    label: Option<&str>,
) -> Result<(), RvalError> {
    if let Some(label) = label {
        let target_bb = ctx
            .default_break_target
            .iter()
            .find(|x| match &x.0 {
                Some(l) => l.deref() == label,
                None => false,
            })
            .expect("Failed to find loop label for break")
            .1;

        ctx.bb.build_unconditional_branch(target_bb).unwrap();

        Ok(())
    } else {
        let target_bb = ctx
            .default_break_target
            .last()
            .expect("No loop to break from")
            .1;
        ctx.bb.build_unconditional_branch(target_bb).unwrap();

        Ok(())
    }
}

/**
 * // TODO: add documentation
 */
fn gen_continue<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    label: Option<&str>,
) -> Result<(), RvalError> {
    if let Some(label) = label {
        let target_bb = ctx
            .default_continue_target
            .iter()
            .find(|x| match &x.0 {
                Some(l) => l.deref() == label,
                None => false,
            })
            .expect("Failed to find loop label for continue")
            .1;

        ctx.bb.build_unconditional_branch(target_bb).unwrap();

        Ok(())
    } else {
        let target_bb = ctx
            .default_continue_target
            .last()
            .expect("No loop to continue from")
            .1;
        ctx.bb.build_unconditional_branch(target_bb).unwrap();

        Ok(())
    }
}

/**
 * // TODO: add documentation
 */
fn gen_return<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    value: &hir::Value,
) -> Result<(), RvalError> {
    let llvm_value = gen_rval(ctx, value)?;
    ctx.bb.build_return(Some(&llvm_value)).unwrap();

    Ok(())
}

/**
 * // TODO: add documentation
 */
fn gen_block_rval<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    hir_block: &hir::Block,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    for (i, element) in hir_block.elements.iter().enumerate() {
        let element_val = match element {
            hir::BlockElement::Stmt(expr) => {
                let expr = &ctx.store[expr].borrow();
                gen_rval(ctx, expr)?;
                gen_rval_lit_unit(ctx)?
            }

            hir::BlockElement::Expr(expr) => {
                let expr = &ctx.store[expr].borrow();
                gen_rval(ctx, expr)?
            }

            hir::BlockElement::Local(local) => {
                let hir_local = &ctx.store[local].borrow();
                let local_name = hir_local.name.to_owned();
                let hir_local_ty = &ctx.store[&hir_local.ty];
                let hir_local_init = &ctx.store[hir_local.init.as_ref().unwrap()].borrow();

                let llvm_local_ty = gen_ty(hir_local_ty, ctx.llvm, ctx.store, ctx.tab);
                let llvm_local = ctx.bb.build_alloca(llvm_local_ty, &local_name).unwrap();
                let llvm_init_value = gen_rval(ctx, hir_local_init)?;
                ctx.bb.build_store(llvm_local, llvm_init_value).unwrap();

                ctx.locals.insert(local_name, llvm_local);
                gen_rval_lit_unit(ctx)?
            }
        };

        if i == hir_block.elements.len() - 1 {
            return Ok(element_val);
        }
    }

    gen_rval_lit_unit(ctx)
}

/**
 * // TODO: add documentation
 */
fn gen_rval_closure<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _captures: &[IString],
    _callee: &hir::FunctionId,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement closure codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_call<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _callee: &hir::Value,
    _arguments: &[hir::ValueId],
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement function call codegen
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_method_call<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _callee: &hir::Value,
    _method_name: &IString,
    _arguments: &[hir::ValueId],
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
fn gen_rval_symbol<'ctx>(
    _ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    _symbol_name: &IString,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    // TODO: implement
    unimplemented!()
}

/**
 * // TODO: add documentation
 */
pub(crate) fn gen_rval<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    hir_value: &hir::Value,
) -> Result<BasicValueEnum<'ctx>, RvalError> {
    match hir_value {
        hir::Value::Unit => gen_rval_lit_unit(ctx),
        hir::Value::Bool(x) => gen_rval_lit_bool(ctx, *x),
        hir::Value::I8(x) => gen_rval_lit_i8(ctx, *x),
        hir::Value::I16(x) => gen_rval_lit_i16(ctx, *x),
        hir::Value::I32(x) => gen_rval_lit_i32(ctx, *x),
        hir::Value::I64(x) => gen_rval_lit_i64(ctx, *x),
        hir::Value::I128(x) => gen_rval_lit_i128(ctx, **x),
        hir::Value::U8(x) => gen_rval_lit_u8(ctx, *x),
        hir::Value::U16(x) => gen_rval_lit_u16(ctx, *x),
        hir::Value::U32(x) => gen_rval_lit_u32(ctx, *x),
        hir::Value::U64(x) => gen_rval_lit_u64(ctx, *x),
        hir::Value::U128(x) => gen_rval_lit_u128(ctx, **x),
        hir::Value::F32(x) => gen_rval_lit_f32(ctx, x.into_inner()),
        hir::Value::F64(x) => gen_rval_lit_f64(ctx, x.into_inner()),
        hir::Value::USize32(x) => gen_rval_lit_u32(ctx, *x),
        hir::Value::USize64(x) => gen_rval_lit_u64(ctx, *x),
        hir::Value::StringLit(x) => gen_rval_lit_string(ctx, x),
        hir::Value::BStringLit(x) => gen_rval_lit_bstring(ctx, x.as_slice()),

        hir::Value::InferredInteger(_) | hir::Value::InferredFloat(_) => {
            panic!("Inferred values should have been resolved before code generation")
        }

        hir::Value::Binary { left, op, right } => {
            let lhs = &ctx.store[left].borrow();
            let rhs = &ctx.store[right].borrow();

            match op {
                hir::BinaryOp::Add => gen_rval_add(ctx, lhs, rhs),
                hir::BinaryOp::Sub => gen_rval_sub(ctx, lhs, rhs),
                hir::BinaryOp::Mul => gen_rval_mul(ctx, lhs, rhs),
                hir::BinaryOp::Div => gen_rval_div(ctx, lhs, rhs),
                hir::BinaryOp::Mod => gen_rval_rem(ctx, lhs, rhs),
                hir::BinaryOp::And => gen_rval_and(ctx, lhs, rhs),
                hir::BinaryOp::Or => gen_rval_or(ctx, lhs, rhs),
                hir::BinaryOp::Xor => gen_rval_xor(ctx, lhs, rhs),
                hir::BinaryOp::Shl => gen_rval_shl(ctx, lhs, rhs),
                hir::BinaryOp::Shr => gen_rval_shr(ctx, lhs, rhs),
                hir::BinaryOp::Rol => gen_rval_rol(ctx, lhs, rhs),
                hir::BinaryOp::Ror => gen_rval_ror(ctx, lhs, rhs),
                hir::BinaryOp::LogicAnd => gen_rval_land(ctx, lhs, rhs),
                hir::BinaryOp::LogicOr => gen_rval_lor(ctx, lhs, rhs),
                hir::BinaryOp::Lt => gen_rval_lt(ctx, lhs, rhs),
                hir::BinaryOp::Gt => gen_rval_gt(ctx, lhs, rhs),
                hir::BinaryOp::Lte => gen_rval_lte(ctx, lhs, rhs),
                hir::BinaryOp::Gte => gen_rval_gte(ctx, lhs, rhs),
                hir::BinaryOp::Eq => gen_rval_eq(ctx, lhs, rhs),
                hir::BinaryOp::Ne => gen_rval_ne(ctx, lhs, rhs),
            }
        }

        hir::Value::Unary { op, operand } => {
            let operand = &ctx.store[operand].borrow();
            match op {
                hir::UnaryOp::Add => gen_rval_unary_add(ctx, operand),
                hir::UnaryOp::Sub => gen_rval_unary_sub(ctx, operand),
                hir::UnaryOp::Not => gen_rval_unary_not(ctx, operand),
            }
        }

        hir::Value::StructObject {
            struct_path,
            fields,
        } => gen_rval_struct_object(ctx, struct_path, fields),

        hir::Value::EnumVariant {
            enum_path,
            variant,
            value,
        } => {
            let value = &ctx.store[value].borrow();
            gen_rval_enum_variant(ctx, enum_path, variant, value)
        }

        hir::Value::FieldAccess { expr, field_name } => {
            let expr = &ctx.store[expr].borrow();
            gen_rval_field_access(ctx, expr, field_name)
        }

        hir::Value::IndexAccess { collection, index } => {
            let collection = &ctx.store[collection].borrow();
            let index = &ctx.store[index].borrow();
            gen_rval_index_access(ctx, collection, index)
        }

        hir::Value::Assign { place, value } => {
            let place = &ctx.store[place].borrow();
            let value = &ctx.store[value].borrow();
            gen_rval_assign(ctx, place, value)
        }

        hir::Value::Deref { place } => {
            let place = &ctx.store[place].borrow();
            gen_rval_deref(ctx, place)
        }

        hir::Value::Cast { value, target_type } => {
            let value = &ctx.store[value].borrow();
            let target_type = &ctx.store[target_type];
            gen_rval_cast(ctx, value, target_type)
        }

        hir::Value::Borrow {
            exclusive,
            mutable,
            place,
        } => {
            let place = &ctx.store[place].borrow();
            gen_rval_borrow(ctx, *exclusive, *mutable, place)
        }

        hir::Value::List { elements } => gen_rval_list(ctx, elements),
        hir::Value::Tuple { elements } => gen_rval_tuple(ctx, elements),

        hir::Value::If {
            condition,
            true_branch,
            false_branch,
        } => {
            let condition = &ctx.store[condition].borrow();
            let true_branch = &ctx.store[true_branch].borrow();
            match false_branch {
                None => gen_if(ctx, condition, true_branch, None),
                Some(false_branch) => {
                    let false_branch = &ctx.store[false_branch].borrow();
                    gen_if(ctx, condition, true_branch, Some(&false_branch))
                }
            }
        }

        hir::Value::While { condition, body } => {
            let condition = &ctx.store[condition].borrow();
            let body = &ctx.store[body].borrow();
            gen_while(ctx, condition, body)?;
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Loop { body } => {
            let body = &ctx.store[body].borrow();
            gen_loop(ctx, body)?;
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Break { label } => {
            gen_break(ctx, label.as_deref())?;
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Continue { label } => {
            gen_continue(ctx, label.as_deref())?;
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Return { value } => {
            let value = &ctx.store[value].borrow();
            gen_return(ctx, value)?;
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Block { block } => {
            let block = &ctx.store[block].borrow();
            gen_block_rval(ctx, block)
        }

        hir::Value::Closure { captures, callee } => gen_rval_closure(ctx, captures, callee),

        hir::Value::Call {
            callee,
            positional,
            named: _,
        } => {
            let callee = &ctx.store[callee].borrow();
            gen_rval_call(ctx, callee, positional)
        }

        hir::Value::MethodCall {
            object,
            method_name,
            positional,
            named: _,
        } => {
            let object = &ctx.store[object].borrow();
            gen_rval_method_call(ctx, object, method_name, positional)
        }

        hir::Value::Symbol { path } => gen_rval_symbol(ctx, path),
    }
}

/**
 * // TODO: add documentation
 */
pub(crate) fn gen_block<'ctx>(
    ctx: &mut RvalGenCtx<'ctx, '_, '_, '_>,
    hir_block: &hir::Block,
) -> Result<(), RvalError> {
    for element in &hir_block.elements {
        match element {
            hir::BlockElement::Stmt(expr) => {
                let expr = &ctx.store[expr].borrow();
                gen_rval(ctx, expr)?;
            }

            hir::BlockElement::Expr(expr) => {
                let expr = &ctx.store[expr].borrow();
                gen_rval(ctx, expr)?;
            }

            hir::BlockElement::Local(local) => {
                let hir_local = &ctx.store[local].borrow();
                let local_name = hir_local.name.to_owned();
                let hir_local_ty = &ctx.store[&hir_local.ty];
                let hir_local_init = &ctx.store[hir_local.init.as_ref().unwrap()].borrow();

                let llvm_local_ty = gen_ty(hir_local_ty, ctx.llvm, ctx.store, ctx.tab);
                let llvm_local = ctx.bb.build_alloca(llvm_local_ty, &local_name).unwrap();
                let llvm_init_value = gen_rval(ctx, hir_local_init)?;
                ctx.bb.build_store(llvm_local, llvm_init_value).unwrap();

                ctx.locals.insert(local_name, llvm_local);
            }
        };
    }

    Ok(())
}
