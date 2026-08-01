use inkwell::{
    basic_block::BasicBlock,
    types::{BasicType, BasicTypeEnum},
    values::{BasicValueEnum, PointerValue},
};

use crate::{
    place::gen_place,
    ty::{TypegenCtx, gen_function_ty, gen_ty},
};
use core::panic;
use nitrate_hir::{ValueId, prelude as hir};
use nitrate_hir_get_type::HirGetType;
use nitrate_llvm::LLVMContext;
use nitrate_nstring::NString;
use std::ops::Deref;
use std::{collections::HashMap, unimplemented};

pub struct CodegenCtx<'ctx, 'module, 'tab, 'builder, 'global> {
    pub llvm: &'ctx LLVMContext,
    pub module: &'module inkwell::module::Module<'ctx>,
    pub tab: &'tab hir::SymbolTab,
    pub bb: &'builder inkwell::builder::Builder<'ctx>,
    pub globals: &'global HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,

    pub locals: HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
    pub parameters: HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
    pub default_continue_target: Vec<(Option<NString>, BasicBlock<'ctx>)>,
    pub default_break_target: Vec<(Option<NString>, BasicBlock<'ctx>)>,
}

impl<'ctx, 'module, 'tab, 'builder, 'global> From<&mut CodegenCtx<'ctx, 'module, 'tab, 'builder, 'global>>
    for TypegenCtx<'ctx, 'tab, 'module>
{
    fn from(codegen_ctx: &mut CodegenCtx<'ctx, 'module, 'tab, 'builder, 'global>) -> Self {
        TypegenCtx {
            llvm: codegen_ctx.llvm,
            tab: codegen_ctx.tab,
            module: codegen_ctx.module,
        }
    }
}

impl<'ctx, 'module, 'tab, 'builder, 'global> CodegenCtx<'ctx, 'module, 'tab, 'builder, 'global> {
    pub fn new(
        llvm: &'ctx LLVMContext,
        module: &'module inkwell::module::Module<'ctx>,
        tab: &'tab hir::SymbolTab,
        bb: &'builder inkwell::builder::Builder<'ctx>,
        globals: &'global HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
    ) -> CodegenCtx<'ctx, 'module, 'tab, 'builder, 'global> {
        CodegenCtx {
            tab,
            llvm,
            module,
            bb,
            globals,

            locals: HashMap::new(),
            parameters: HashMap::new(),
            default_continue_target: Vec::new(),
            default_break_target: Vec::new(),
        }
    }
}

#[derive(Debug)]
#[allow(dead_code)]
pub enum CodegenError {
    OperandTypeCombinationError { operation_name: &'static str },
    InvalidPlaceValue,
    SymbolNotFound { symbol_name: NString },
}

/**
 * The Unit Value is an empty struct
 */
fn gen_rval_lit_unit<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>) -> BasicValueEnum<'ctx> {
    ctx.llvm.const_struct(&[], false).into()
}

/**
 * Direct correspondence to LLVM i1 type.
 * No sign extension is performed.
 */
fn gen_rval_lit_bool<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: bool) -> BasicValueEnum<'ctx> {
    match value {
        true => ctx.llvm.bool_type().const_int(1, false).into(),
        false => ctx.llvm.bool_type().const_int(0, false).into(),
    }
}

/**
 * Direct correspondence to LLVM i8 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i8<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: i8) -> BasicValueEnum<'ctx> {
    ctx.llvm.i8_type().const_int(value as u64, true).into()
}

/**
 * Direct correspondence to LLVM i16 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i16<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: i16) -> BasicValueEnum<'ctx> {
    ctx.llvm.i16_type().const_int(value as u64, true).into()
}

/**
 * Direct correspondence to LLVM i32 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i32<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: i32) -> BasicValueEnum<'ctx> {
    ctx.llvm.i32_type().const_int(value as u64, true).into()
}

/**
 * Direct correspondence to LLVM i64 type.
 * Sign extension is performed.
 */
fn gen_rval_lit_i64<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: i64) -> BasicValueEnum<'ctx> {
    ctx.llvm.i64_type().const_int(value as u64, true).into()
}

/**
 * Direct correspondence to LLVM i128 type.
 * Sign extension is not performed because the value is constructed
 * from its low and high parts directly.
 */
fn gen_rval_lit_i128<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: i128) -> BasicValueEnum<'ctx> {
    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;

    ctx.llvm.i128_type().const_int_arbitrary_precision(&[low, high]).into()
}

/**
 * Direct correspondence to LLVM i8 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u8<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: u8) -> BasicValueEnum<'ctx> {
    ctx.llvm.i8_type().const_int(value as u64, false).into()
}

/**
 * Direct correspondence to LLVM i16 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u16<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: u16) -> BasicValueEnum<'ctx> {
    ctx.llvm.i16_type().const_int(value as u64, false).into()
}

/**
 * Direct correspondence to LLVM i32 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u32<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: u32) -> BasicValueEnum<'ctx> {
    ctx.llvm.i32_type().const_int(value as u64, false).into()
}

/**
 * Direct correspondence to LLVM i64 type (2's complement).
 * No sign extension is performed.
 */
fn gen_rval_lit_u64<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: u64) -> BasicValueEnum<'ctx> {
    ctx.llvm.i64_type().const_int(value, false).into()
}

/**
 * Direct correspondence to LLVM i128 type (2's complement).
 * Sign extension is not performed because the value is constructed
 * from its low and high parts directly.
 */
fn gen_rval_lit_u128<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: u128) -> BasicValueEnum<'ctx> {
    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;

    ctx.llvm.i128_type().const_int_arbitrary_precision(&[low, high]).into()
}

/**
 * Direct correspondence to LLVM f32 type.
 */
fn gen_rval_lit_f32<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: f32) -> BasicValueEnum<'ctx> {
    ctx.llvm.f32_type().const_float(value as f64).into()
}

/**
 * Direct correspondence to LLVM f64 type.
 */
fn gen_rval_lit_f64<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: f64) -> BasicValueEnum<'ctx> {
    ctx.llvm.f64_type().const_float(value).into()
}

/**
 * Create a LLVM string constant byte-array.
 * No null terminator is added.
 */
fn gen_rval_lit_string<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: &str) -> BasicValueEnum<'ctx> {
    ctx.llvm.const_string(value.as_bytes(), false).into()
}

/**
 * Create a LLVM string constant byte-array.
 * No null terminator is added.
 */
fn gen_rval_lit_bstring<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: &[u8]) -> BasicValueEnum<'ctx> {
    ctx.llvm.const_string(value, false).into()
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fadd = ctx
            .bb
            .build_float_add(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        fadd.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let iadd = ctx
            .bb
            .build_int_add(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        iadd.into()
    } else {
        panic!("Unsupported operand types for addition");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fsub = ctx
            .bb
            .build_float_sub(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        fsub.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let isub = ctx
            .bb
            .build_int_sub(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        isub.into()
    } else {
        panic!("Unsupported operand types for subtraction");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fmul = ctx
            .bb
            .build_float_mul(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        fmul.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let imul = ctx
            .bb
            .build_int_mul(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        imul.into()
    } else {
        panic!("Unsupported operand types for multiplication");
    }
}

/**
 * Division:
 *
 * Signed Integers:
 * - https://llvm.org/docs/LangRef.html#sdiv-instruction
 *
 * Unsigned Integers:
 * - https://llvm.org/docs/LangRef.html#udiv-instruction
 *
 * Floating-point:
 * - Follows the IEEE 754 standard for floating-point arithmetic.
 * - https://llvm.org/docs/LangRef.html#fdiv-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_div<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fdiv = ctx
            .bb
            .build_float_div(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "")
            .unwrap();

        fdiv.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .determine_type(ctx.tab)
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

        div.into()
    } else {
        panic!("Unsupported operand types for division");
    }
}

/**
 * Remainder:
 *
 * Signed Integers:
 * - https://llvm.org/docs/LangRef.html#srem-instruction
 *
 * Unsigned Integers:
 * - https://llvm.org/docs/LangRef.html#urem-instruction
 *
 * Floating-point:
 * - Follows the IEEE 754 standard for floating-point arithmetic.
 * - https://llvm.org/docs/LangRef.html#frem-instruction
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_rem<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let frem = ctx
            .bb
            .build_float_rem(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "")
            .unwrap();

        frem.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .determine_type(ctx.tab)
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

        rem.into()
    } else {
        panic!("Unsupported operand types for remainder");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let and = ctx
            .bb
            .build_and(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        and.into()
    } else {
        panic!("Unsupported operand types for bitwise and");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let or = ctx.bb.build_or(lhs.into_int_value(), rhs.into_int_value(), "").unwrap();

        or.into()
    } else {
        panic!("Unsupported operand types for bitwise or");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let xor = ctx
            .bb
            .build_xor(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        xor.into()
    } else {
        panic!("Unsupported operand types for bitwise xor");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let shl = ctx
            .bb
            .build_left_shift(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        shl.into()
    } else {
        panic!("Unsupported operand types for bitwise left-shift");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let sign_extend = lhs
            .determine_type(ctx.tab)
            .expect("Failed to get type")
            .is_signed_primitive();

        let shr = ctx
            .bb
            .build_right_shift(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), sign_extend, "")
            .unwrap();

        shr.into()
    } else {
        panic!("Unsupported operand types for bitwise right-shift");
    }
}

/**
 * Bitwise rotate left operation formula:
 * rol(x, n) = (x << (n % bit_width)) | (x >> ((bit_width - (n % bit_width)) % bit_width))
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_rol<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if !lhs_ty.is_int_type() || !rhs_ty.is_int_type() {
        panic!("Unsupported operand types for bitwise rotate left");
    }

    let bit_width = ctx.llvm.target_data().get_store_size(&lhs.get_type()) * 8;
    let bit_width_i32 = ctx.llvm.i32_type().const_int(bit_width as u64, false);

    let reduced_n = ctx
        .bb
        .build_int_unsigned_rem(rhs.into_int_value(), bit_width_i32, "")
        .unwrap();

    let shl = ctx.bb.build_left_shift(lhs.into_int_value(), reduced_n, "").unwrap();

    let sub = ctx.bb.build_int_sub(bit_width_i32, reduced_n, "").unwrap();

    let sub_reduced = ctx.bb.build_int_unsigned_rem(sub, bit_width_i32, "").unwrap();

    let shr = ctx
        .bb
        .build_right_shift(lhs.into_int_value(), sub_reduced, false, "")
        .unwrap();

    let or = ctx.bb.build_or(shl, shr, "").unwrap();

    or.into()
}

/**
 * Bitwise rotate right operation formula:
 * ror(x, n) = (x >> (n % bit_width)) | (x << ((bit_width - (n % bit_width)) % bit_width))
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_ror<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if !lhs_ty.is_int_type() || !rhs_ty.is_int_type() {
        panic!("Unsupported operand types for bitwise rotate right");
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

    let sub_reduced = ctx.bb.build_int_unsigned_rem(sub, bit_width_i32, "").unwrap();

    let shl = ctx.bb.build_left_shift(lhs.into_int_value(), sub_reduced, "").unwrap();

    let or = ctx.bb.build_or(shr, shl, "").unwrap();

    or.into()
}

/**
 * Logical AND operation:
 *
 * Evaluates the LHS; if true, evaluates the RHS; otherwise, returns false.
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_land<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let parent_function = ctx.bb.get_insert_block().unwrap().get_parent().unwrap();
    let bool = ctx.llvm.bool_type();

    let rhs_bb = ctx.llvm.append_basic_block(parent_function, "land_rhs");
    let end_bb = ctx.llvm.append_basic_block(parent_function, "land_join");

    /**************************************************************************/
    // 1. Allocate space for the result
    let land_result = ctx.bb.build_alloca(bool, "land_result").unwrap();

    /**************************************************************************/
    // 2. Evaluate LHS; if true, skip RHS
    let lhs_val = gen_rval(ctx, lhs);
    let lhs_val_ty = lhs_val.get_type();
    if !lhs_val_ty.is_int_type() {
        panic!("Unsupported operand types for logical AND");
    }

    ctx.bb.build_store(land_result, lhs_val).unwrap();
    ctx.bb
        .build_conditional_branch(lhs_val.into_int_value(), rhs_bb, end_bb)
        .unwrap();

    /**************************************************************************/
    // 3. Evaluate RHS
    ctx.bb.position_at_end(rhs_bb);
    let rhs_val = gen_rval(ctx, rhs);
    ctx.bb.build_store(land_result, rhs_val).unwrap();
    ctx.bb.build_unconditional_branch(end_bb).unwrap();

    /**************************************************************************/
    // 4. Join block and load result
    ctx.bb.position_at_end(end_bb);

    ctx.bb.build_load(bool, land_result, "land_load").unwrap()
}

/**
 * Logical OR operation:
 *
 * Evaluates the LHS; if false, evaluates the RHS; otherwise, returns true.
 *
 * This operation has left-to-right evaluation order.
 */
fn gen_rval_lor<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let parent_function = ctx.bb.get_insert_block().unwrap().get_parent().unwrap();
    let bool = ctx.llvm.bool_type();

    let rhs_bb = ctx.llvm.append_basic_block(parent_function, "lor_rhs");
    let end_bb = ctx.llvm.append_basic_block(parent_function, "lor_join");

    /**************************************************************************/
    // 1. Allocate space for the result
    let lor_result = ctx.bb.build_alloca(bool, "lor_result").unwrap();

    /**************************************************************************/
    // 2. Evaluate LHS; if true, skip RHS
    let lhs_val = gen_rval(ctx, lhs);
    let lhs_val_ty = lhs_val.get_type();
    if !lhs_val_ty.is_int_type() {
        panic!("Unsupported operand types for logical OR");
    }

    ctx.bb.build_store(lor_result, lhs_val).unwrap();
    ctx.bb
        .build_conditional_branch(lhs_val.into_int_value(), end_bb, rhs_bb)
        .unwrap();

    /**************************************************************************/
    // 3. Evaluate RHS
    ctx.bb.position_at_end(rhs_bb);
    let rhs_val = gen_rval(ctx, rhs);
    ctx.bb.build_store(lor_result, rhs_val).unwrap();
    ctx.bb.build_unconditional_branch(end_bb).unwrap();

    /**************************************************************************/
    // 4. Join block and load result
    ctx.bb.position_at_end(end_bb);

    ctx.bb.build_load(bool, lor_result, "lor_load").unwrap()
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
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

        fcmp.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .determine_type(ctx.tab)
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

        cmp.into()
    } else {
        panic!("Unsupported operand types for less than comparison");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
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

        fcmp.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .determine_type(ctx.tab)
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

        cmp.into()
    } else {
        panic!("Unsupported operand types for greater than comparison");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
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

        fcmp.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .determine_type(ctx.tab)
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

        cmp.into()
    } else {
        panic!("Unsupported operand types for less than or equal comparison");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
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

        fcmp.into()
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = lhs
            .determine_type(ctx.tab)
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

        cmp.into()
    } else {
        panic!("Unsupported operand types for greater than or equal comparison");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
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

        fcmp.into()
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

        icmp.into()
    } else {
        panic!("Unsupported operand types for equality comparison");
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
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
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

        fcmp.into()
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

        icmp.into()
    } else {
        panic!("Unsupported operand types for inequality comparison");
    }
}

/**
 * Unary addition operation.
 *
 * This operation is effectively a no-op and simply returns the operand as is.
 */
fn gen_rval_unary_add<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, operand: &hir::Value) -> BasicValueEnum<'ctx> {
    gen_rval(ctx, operand)
}

/**
 * Unary subtraction operation.
 *
 * This operation negates the operand.
 * - For floating-point types, it uses the LLVM `fneg` instruction.
 * - For integer types, it subtracts the operand from zero.
 */
fn gen_rval_unary_sub<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, operand: &hir::Value) -> BasicValueEnum<'ctx> {
    let llvm_operand = gen_rval(ctx, operand);
    let operand_ty = llvm_operand.get_type();

    if operand_ty.is_float_type() {
        let fneg = ctx.bb.build_float_neg(llvm_operand.into_float_value(), "").unwrap();

        fneg.into()
    } else if operand_ty.is_int_type() {
        let zero = operand_ty.into_int_type().const_int(0, false);

        let neg = ctx.bb.build_int_sub(zero, llvm_operand.into_int_value(), "").unwrap();

        neg.into()
    } else {
        panic!("Unsupported operand type for unary subtraction");
    }
}

/**
 * Unary logical NOT operation.
 *
 * This operation inverts the boolean value of the operand.
 * - For integer types, it uses the LLVM `not` instruction.
 */
fn gen_rval_unary_not<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, operand: &hir::Value) -> BasicValueEnum<'ctx> {
    let llvm_operand = gen_rval(ctx, operand);
    let operand_ty = llvm_operand.get_type();

    if operand_ty.is_int_type() {
        let not = ctx.bb.build_not(llvm_operand.into_int_value(), "").unwrap();

        not.into()
    } else {
        panic!("Unsupported operand type for unary logical NOT");
    }
}

fn gen_rval_struct_object<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    struct_def_id: &hir::StructDefId,
    fields: &[(NString, ValueId)],
) -> BasicValueEnum<'ctx> {
    let struct_ty = hir::Type::Struct {
        span: nitrate_tree::ByteSpan::default(),
        def: struct_def_id.clone(),
    };

    let struct_def = struct_def_id.borrow();
    let mut field_map = HashMap::new();
    for (i, (field_name, _)) in struct_def.fields.iter().enumerate() {
        field_map.insert(field_name.clone(), i);
    }

    let llvm_ty = gen_ty(&struct_ty, &mut ctx.into());
    let struct_alloca = ctx.bb.build_alloca(llvm_ty, "struct_alloca").unwrap();

    for (field_name, field_value_id) in fields {
        let field_index = *field_map.get(field_name).expect("field not found in struct");

        let field_value = field_value_id.borrow();
        let llvm_field_value = gen_rval(ctx, &field_value);

        let index = ctx.llvm.i32_type().const_int(field_index as u64, false);
        let gep = unsafe {
            // SAFETY: ** I don't know if this is safe or not
            ctx.bb.build_in_bounds_gep(
                llvm_ty,
                struct_alloca,
                &[ctx.llvm.i32_type().const_int(0, false), index],
                "struct_field_gep",
            )
        }
        .unwrap();

        ctx.bb.build_store(gep, llvm_field_value).unwrap();
    }

    ctx.bb.build_load(llvm_ty, struct_alloca, "struct_load").unwrap()
}

fn gen_rval_enum_variant<'ctx>(
    _ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    _enum_def_id: &hir::EnumDefId,
    _variant_name: &NString,
    _value: &hir::Value,
) -> BasicValueEnum<'ctx> {
    // TODO: implement enum variant codegen
    unimplemented!()
}

fn gen_rval_field_access<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    struct_value: &hir::Value,
    field_name: &NString,
) -> BasicValueEnum<'ctx> {
    let value_type = struct_value.determine_type(ctx.tab).expect("Failed to get type");
    // Resolve through references to find the actual struct type
    let actual_type = match &value_type {
        hir::Type::Reference { to, .. } | hir::Type::Pointer { to, .. } => to.deref().clone(),
        _ => value_type.clone(),
    };
    let hir_struct_def = actual_type.as_struct().expect("expected struct type").borrow();

    let field_ty = &hir_struct_def
        .fields
        .get(field_name)
        .expect("expected field to exist in struct")
        .ty;

    // `gen_place` on a FieldAccess handles both plain places and
    // auto-deref through references/pointers, producing the address of the
    // actual field in memory (no copies).
    let field_ptr = gen_place(
        ctx,
        &hir::Value::FieldAccess {
            span: nitrate_tree::ByteSpan::default(),
            expr: struct_value.clone().into(),
            field_name: field_name.clone(),
        },
    );

    ctx.bb
        .build_load(gen_ty(field_ty, &mut ctx.into()), field_ptr, "field_access_load")
        .unwrap()
}

fn gen_rval_assign<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    place: &hir::Value,
    value: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_place = gen_place(ctx, place);
    let llvm_value = gen_rval(ctx, value);

    ctx.bb.build_store(llvm_place, llvm_value).unwrap();
    llvm_value
}

fn gen_rval_deref<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, place: &hir::Value) -> BasicValueEnum<'ctx> {
    let llvm_value = gen_rval(ctx, place);
    let ptr_ty = llvm_value.get_type();
    if !ptr_ty.is_pointer_type() {
        panic!("Unsupported operand type for dereference");
    }

    let pointee_ty = match place.determine_type(ctx.tab).unwrap() {
        hir::Type::Pointer { to, .. } => to.deref().clone(),
        hir::Type::Reference { to, .. } => to.deref().clone(),
        _ => unreachable!(),
    };

    ctx.bb
        .build_load(
            gen_ty(&pointee_ty, &mut ctx.into()),
            llvm_value.into_pointer_value(),
            "deref_load",
        )
        .unwrap()
}

fn gen_rval_cast<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    value: &hir::Value,
    target_type: &hir::Type,
) -> BasicValueEnum<'ctx> {
    let llvm_value = gen_rval(ctx, value);
    let llvm_target_ty = gen_ty(target_type, &mut ctx.into());

    // In LLVM, both references and pointers map to the same ptr type.
    // Casting between pointer-like types (reference → pointer, pointer → pointer, etc.)
    // is a no-op at the LLVM level — just bitcast.
    if llvm_value.is_pointer_value() || llvm_target_ty.is_pointer_type() {
        return ctx.bb.build_bit_cast(llvm_value, llvm_target_ty, "cast_ptr").unwrap();
    }

    // Int-to-int cast
    if llvm_value.is_int_value() && llvm_target_ty.is_int_type() {
        let v = llvm_value.into_int_value();
        let dst_ty = llvm_target_ty.into_int_type();
        let src_bits = v.get_type().get_bit_width();
        let dst_bits = dst_ty.get_bit_width();

        if src_bits == dst_bits {
            return ctx.bb.build_bit_cast(llvm_value, llvm_target_ty, "cast_int").unwrap();
        } else if src_bits < dst_bits {
            return ctx
                .bb
                .build_int_z_extend_or_bit_cast(v, dst_ty, "cast_zext")
                .unwrap()
                .into();
        } else {
            return ctx
                .bb
                .build_int_truncate_or_bit_cast(v, dst_ty, "cast_trunc")
                .unwrap()
                .into();
        }
    }

    // Float-to-float cast
    if llvm_value.is_float_value() && llvm_target_ty.is_float_type() {
        return ctx
            .bb
            .build_float_cast(
                llvm_value.into_float_value(),
                llvm_target_ty.into_float_type(),
                "cast_fp",
            )
            .unwrap()
            .into();
    }

    // Int-to-float cast
    if llvm_value.is_int_value() && llvm_target_ty.is_float_type() {
        return ctx
            .bb
            .build_signed_int_to_float(
                llvm_value.into_int_value(),
                llvm_target_ty.into_float_type(),
                "cast_sitofp",
            )
            .unwrap()
            .into();
    }

    // Float-to-int cast
    if llvm_value.is_float_value() && llvm_target_ty.is_int_type() {
        return ctx
            .bb
            .build_float_to_signed_int(
                llvm_value.into_float_value(),
                llvm_target_ty.into_int_type(),
                "cast_fptosi",
            )
            .unwrap()
            .into();
    }

    // Fallback: try bitcast
    ctx.bb
        .build_bit_cast(llvm_value, llvm_target_ty, "cast_fallback")
        .unwrap()
}

fn gen_rval_borrow<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    _exclusive: bool,
    _mutable: bool,
    place: &hir::Value,
) -> BasicValueEnum<'ctx> {
    let llvm_place = gen_place(ctx, place);
    // Return the address of the place as the borrowed value
    llvm_place.into()
}

fn gen_rval_list<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, elements: &[hir::ValueId]) -> BasicValueEnum<'ctx> {
    if elements.is_empty() {
        // If the list is empty it will have size zero. That mean any dereference is invalid anyways.
        // Therefore, any type will be okay. For simplicity we will use the unit type.
        let element_type = gen_ty(
            &hir::Type::Unit {
                span: nitrate_tree::ByteSpan::default(),
            },
            &mut ctx.into(),
        );
        let list_ty = element_type.array_type(0);

        let list_alloca = ctx.bb.build_alloca(list_ty, "list_alloca").unwrap();
        let load = ctx.bb.build_load(list_ty, list_alloca, "list_load").unwrap();
        return load;
    }

    let mut llvm_elements = Vec::new();
    for element in elements {
        let llvm_element = gen_rval(ctx, &element.borrow());
        llvm_elements.push(llvm_element);
    }

    let list_ty = llvm_elements[0].get_type().array_type(llvm_elements.len() as u32);

    let list_alloca = ctx.bb.build_alloca(list_ty, "list_alloca").unwrap();
    for (i, llvm_element) in llvm_elements.iter().enumerate() {
        let index = ctx.llvm.i32_type().const_int(i as u64, false);
        let gep = unsafe {
            // SAFETY: ** I don't know if this is safe or not
            ctx.bb.build_in_bounds_gep(
                list_ty,
                list_alloca,
                &[ctx.llvm.i32_type().const_int(0, false), index],
                "list_elem_gep",
            )
        }
        .unwrap();
        ctx.bb.build_store(gep, *llvm_element).unwrap();
    }

    ctx.bb.build_load(list_ty, list_alloca, "list_load").unwrap()
}

fn gen_rval_tuple<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, elements: &[hir::ValueId]) -> BasicValueEnum<'ctx> {
    if elements.is_empty() {
        return gen_rval_lit_unit(ctx);
    }

    let mut llvm_elements = Vec::new();
    let mut llvm_elements_types = Vec::new();
    for element in elements {
        let llvm_element = gen_rval(ctx, &element.borrow());
        llvm_elements_types.push(llvm_element.get_type());
        llvm_elements.push(llvm_element);
    }

    let tuple_ty = ctx.llvm.struct_type(&llvm_elements_types, false);
    let tuple_alloca = ctx.bb.build_alloca(tuple_ty, "tuple_alloca").unwrap();
    for (i, llvm_element) in llvm_elements.iter().enumerate() {
        let index = ctx.llvm.i32_type().const_int(i as u64, false);
        let gep = unsafe {
            // SAFETY: ** I don't know if this is safe or not
            ctx.bb.build_in_bounds_gep(
                tuple_ty,
                tuple_alloca,
                &[ctx.llvm.i32_type().const_int(0, false), index],
                "tuple_elem_gep",
            )
        }
        .unwrap();
        ctx.bb.build_store(gep, *llvm_element).unwrap();
    }

    ctx.bb.build_load(tuple_ty, tuple_alloca, "tuple_load").unwrap()
}

fn gen_rval_if<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    condition: &hir::Value,
    true_branch: &hir::Block,
    false_branch: Option<&hir::Block>,
) -> BasicValueEnum<'ctx> {
    let top_block = ctx.bb.get_insert_block().unwrap();
    let current_function = top_block.get_parent().unwrap();

    if let Some(false_branch) = false_branch {
        let then_bb = ctx.llvm.append_basic_block(current_function, "if_then");
        let else_bb = ctx.llvm.append_basic_block(current_function, "if_else");
        let join_bb = ctx.llvm.append_basic_block(current_function, "if_join");

        let true_branch_ty = true_branch.determine_type(ctx.tab).unwrap();
        let false_branch_ty = false_branch.determine_type(ctx.tab).unwrap();

        let if_result_ty = if true_branch_ty.is_diverging() {
            gen_ty(&false_branch_ty, &mut ctx.into())
        } else {
            gen_ty(&true_branch_ty, &mut ctx.into())
        };

        let result = ctx.bb.build_alloca(if_result_ty, "if_result").unwrap();
        let cond_val = gen_rval(ctx, condition);

        ctx.bb
            .build_conditional_branch(cond_val.into_int_value(), then_bb, else_bb)
            .unwrap();

        /************************************************************************/
        // True branch
        ctx.bb.position_at_end(then_bb);
        if true_branch_ty.is_diverging() {
            gen_block(ctx, true_branch);
        } else {
            let result_val = gen_rval_block(ctx, true_branch);
            ctx.bb.build_store(result, result_val).unwrap();
            ctx.bb.build_unconditional_branch(join_bb).unwrap();
        }

        /************************************************************************/
        // False branch
        ctx.bb.position_at_end(else_bb);
        if false_branch_ty.is_diverging() {
            gen_block(ctx, false_branch);
        } else {
            let result_val = gen_rval_block(ctx, false_branch);
            ctx.bb.build_store(result, result_val).unwrap();
            ctx.bb.build_unconditional_branch(join_bb).unwrap();
        }

        /************************************************************************/
        // Join block
        ctx.bb.position_at_end(join_bb);
        let load = ctx.bb.build_load(if_result_ty, result, "if_load").unwrap();
        return load;
    }

    let then_bb = ctx.llvm.append_basic_block(current_function, "if_then");
    let join_bb = ctx.llvm.append_basic_block(current_function, "if_join");

    let true_branch_ty = true_branch.determine_type(ctx.tab).unwrap();

    let cond_val = gen_rval(ctx, condition);
    ctx.bb
        .build_conditional_branch(cond_val.into_int_value(), then_bb, join_bb)
        .unwrap();

    ctx.bb.position_at_end(then_bb);
    gen_block(ctx, true_branch);
    if !true_branch_ty.is_diverging() {
        ctx.bb.build_unconditional_branch(join_bb).unwrap();
    }

    ctx.bb.position_at_end(join_bb);

    gen_rval_lit_unit(ctx)
}

fn gen_rval_while<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, condition: &hir::Value, body: &hir::Block) {
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
    let cond_val = gen_rval(ctx, condition);
    ctx.bb
        .build_conditional_branch(cond_val.into_int_value(), body_bb, join_bb)
        .unwrap();

    /************************************************************************/
    // 2. While loop body
    ctx.bb.position_at_end(body_bb);
    gen_block(ctx, body);
    if !body.determine_type(ctx.tab).unwrap().is_diverging() {
        ctx.bb.build_unconditional_branch(cond_bb).unwrap();
    }

    ctx.bb.position_at_end(join_bb);

    ctx.default_continue_target.pop();
    ctx.default_break_target.pop();
}

fn gen_rval_loop<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, body: &hir::Block) {
    let top_block = ctx.bb.get_insert_block().unwrap();
    let current_function = top_block.get_parent().unwrap();

    let body_bb = ctx.llvm.append_basic_block(current_function, "loop_body");
    let join_bb = ctx.llvm.append_basic_block(current_function, "loop_join");

    ctx.default_continue_target.push((None, body_bb));
    ctx.default_break_target.push((None, join_bb));

    /************************************************************************/
    // 1. Loop body
    ctx.bb.position_at_end(body_bb);
    gen_block(ctx, body);
    if !body.determine_type(ctx.tab).unwrap().is_diverging() {
        ctx.bb.build_unconditional_branch(body_bb).unwrap();
    }

    /************************************************************************/
    // 2. Join block
    ctx.bb.position_at_end(join_bb);

    ctx.default_continue_target.pop();
    ctx.default_break_target.pop();
}

/**
 * Generates a break statement.
 */
fn gen_rval_break<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, label: Option<&str>) {
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
    } else {
        let target_bb = ctx.default_break_target.last().expect("No loop to break from").1;
        ctx.bb.build_unconditional_branch(target_bb).unwrap();
    }
}

fn gen_rval_continue<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, label: Option<&str>) {
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
    } else {
        let target_bb = ctx.default_continue_target.last().expect("No loop to continue from").1;
        ctx.bb.build_unconditional_branch(target_bb).unwrap();
    }
}

fn gen_rval_return<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, value: &hir::Value) {
    let llvm_value = gen_rval(ctx, value);
    ctx.bb.build_return(Some(&llvm_value)).unwrap();
}

fn gen_rval_block<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, hir_block: &hir::Block) -> BasicValueEnum<'ctx> {
    for (i, element) in hir_block.elements.iter().enumerate() {
        let element_val = match element {
            hir::BlockElement::Expr(expr) => gen_rval(ctx, &expr.borrow()),

            hir::BlockElement::Local(local) => {
                let hir_local = local.borrow();
                let local_name = hir_local.name.to_owned();
                let hir_local_ty = &hir_local.ty;
                let hir_local_init = &hir_local.initializer.borrow();

                let llvm_local_ty = gen_ty(hir_local_ty, &mut ctx.into());
                let llvm_local = ctx.bb.build_alloca(llvm_local_ty, &local_name).unwrap();
                let llvm_init_value = gen_rval(ctx, hir_local_init);
                ctx.bb.build_store(llvm_local, llvm_init_value).unwrap();

                ctx.locals.insert(local_name, (llvm_local, llvm_local_ty));
                gen_rval_lit_unit(ctx)
            }
        };

        if i == hir_block.elements.len() - 1 {
            return element_val;
        }
    }

    gen_rval_lit_unit(ctx)
}

fn gen_rval_call<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    callee: &hir::Value,
    arguments: &hir::Arguments<ValueId>,
) -> BasicValueEnum<'ctx> {
    let callee_ty_hir = callee.determine_type(ctx.tab).unwrap();
    let hir::Type::Function { function_type, .. } = callee_ty_hir else {
        panic!("Callee is not a function type");
    };

    let llvm_function_ty = gen_function_ty(&function_type, &mut ctx.into());

    let mut llvm_arguments = Vec::new();
    for arg_id in arguments.to_owned().into_iter() {
        let llvm_arg = gen_rval(ctx, &arg_id.borrow());
        llvm_arguments.push(llvm_arg.into());
    }

    let callee = gen_rval(ctx, callee);
    if !callee.get_type().is_pointer_type() {
        panic!("Callee is not a function pointer");
    }

    ctx.bb
        .build_indirect_call(llvm_function_ty, callee.into_pointer_value(), &llvm_arguments, "")
        .unwrap()
        .try_as_basic_value()
        .expect_left("missing value")
}

fn gen_rval_method_call<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    object: &hir::Value,
    method_name: &NString,
    arguments: &hir::Arguments<ValueId>,
) -> BasicValueEnum<'ctx> {
    let object_ty_hir = object.determine_type(ctx.tab).unwrap().into();
    let function_id = ctx
        .tab
        .get_method(&object_ty_hir, method_name)
        .expect("Method not found")
        .to_owned();

    let llvm_function_ty = gen_function_ty(&function_id.borrow().get_type(), &mut ctx.into());

    let mut llvm_arguments = Vec::new();

    // The method function expects self as the first parameter.
    // Check what the function's first param expects (reference vs by-value)
    {
        let func_def = function_id.borrow();
        let self_param_ty = &func_def.params[0].borrow().ty;
        let is_self_ref = matches!(&**self_param_ty, hir::Type::Reference { .. });

        // If the method takes &self (reference), pass the address of the object.
        // If the object receiver is *already* a reference/pointer (auto-deref
        // on method calls), pass the pointer value directly rather than the
        // address of the reference slot.
        if is_self_ref {
            let obj_type = object.determine_type(ctx.tab).expect("Failed to get type");
            let llvm_object_ptr = if matches!(&obj_type, hir::Type::Reference { .. } | hir::Type::Pointer { .. }) {
                let ref_val = gen_rval(ctx, object);
                ref_val.into_pointer_value()
            } else {
                gen_place(ctx, object)
            };
            llvm_arguments.push(llvm_object_ptr.into());
        } else {
            let llvm_object_arg = gen_rval(ctx, object);
            llvm_arguments.push(llvm_object_arg.into());
        }
    } // func_def borrow ends here

    for arg_id in arguments.to_owned().into_iter() {
        let llvm_arg = gen_rval(ctx, &arg_id.borrow());
        llvm_arguments.push(llvm_arg.into());
    }

    let callee = gen_rval(
        ctx,
        &hir::Value::FunctionSymbol {
            span: nitrate_tree::ByteSpan::default(),
            id: function_id,
        },
    );
    if !callee.get_type().is_pointer_type() {
        panic!("Callee is not a function pointer");
    }

    ctx.bb
        .build_indirect_call(llvm_function_ty, callee.into_pointer_value(), &llvm_arguments, "")
        .unwrap()
        .try_as_basic_value()
        .expect_left("missing value")
}

fn gen_rval_symbol<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, symbol_name: &NString) -> BasicValueEnum<'ctx> {
    if let Some((local, llvm_local_ty)) = ctx.locals.get(symbol_name) {
        let load = ctx.bb.build_load(*llvm_local_ty, *local, "symbol_load").unwrap();
        return load;
    } else if let Some((parameter, llvm_param_ty)) = ctx.parameters.get(symbol_name) {
        let load = ctx
            .bb
            .build_load(*llvm_param_ty, *parameter, "parameter_symbol_load")
            .unwrap();
        return load;
    } else if let Some((global, llvm_global_ty)) = ctx.globals.get(symbol_name) {
        let load = ctx
            .bb
            .build_load(*llvm_global_ty, *global, "global_symbol_load")
            .unwrap();
        return load;
    } else if let Some(function) = ctx.module.get_function(symbol_name) {
        let function_ptr = function.as_global_value().as_pointer_value();
        return function_ptr.into();
    }

    // Function not found in LLVM module - it may be a monomorphized function
    // that was created by Hindley-Milner but not registered in SymbolTab.
    // Look it up by searching through the store for matching name.
    // The monomorphized function is stored in the global Store via its FunctionId.
    panic!(
        "Undefined symbol: {}. Monomorphized function may not have been properly registered.",
        symbol_name
    );
}

pub(crate) fn gen_rval<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>,
    hir_value: &hir::Value,
) -> BasicValueEnum<'ctx> {
    match hir_value {
        hir::Value::Range { .. } => {
            panic!("Range expressions should have been desugared into StructObject before LLVM codegen by hir_solve.")
        }
        hir::Value::Unit { .. } => gen_rval_lit_unit(ctx),
        hir::Value::Bool { value: x, .. } => gen_rval_lit_bool(ctx, *x),
        hir::Value::I8 { value: x, .. } => gen_rval_lit_i8(ctx, *x),
        hir::Value::I16 { value: x, .. } => gen_rval_lit_i16(ctx, *x),
        hir::Value::I32 { value: x, .. } => gen_rval_lit_i32(ctx, *x),
        hir::Value::I64 { value: x, .. } => gen_rval_lit_i64(ctx, *x),
        hir::Value::I128 { value: x, .. } => gen_rval_lit_i128(ctx, **x),
        hir::Value::U8 { value: x, .. } => gen_rval_lit_u8(ctx, *x),
        hir::Value::U16 { value: x, .. } => gen_rval_lit_u16(ctx, *x),
        hir::Value::U32 { value: x, .. } => gen_rval_lit_u32(ctx, *x),
        hir::Value::U64 { value: x, .. } => gen_rval_lit_u64(ctx, *x),
        hir::Value::U128 { value: x, .. } => gen_rval_lit_u128(ctx, **x),
        hir::Value::F32 { value: x, .. } => gen_rval_lit_f32(ctx, x.into_inner()),
        hir::Value::F64 { value: x, .. } => gen_rval_lit_f64(ctx, x.into_inner()),
        hir::Value::USize { bits: 32, value: x, .. } => gen_rval_lit_u32(ctx, *x as u32),
        hir::Value::USize { bits: 64, value: x, .. } => gen_rval_lit_u64(ctx, *x),
        hir::Value::USize { value: x, .. } => panic!("Unsupported usize size: {}", x),
        hir::Value::StringLit { value: x, .. } => gen_rval_lit_string(ctx, x),
        hir::Value::BStringLit { value: x, .. } => gen_rval_lit_bstring(ctx, x.as_slice()),

        hir::Value::InferredInteger { .. } | hir::Value::InferredFloat { .. } => {
            panic!("Inferred values should have been resolved before code generation")
        }

        hir::Value::Binary { left, op, right, .. } => {
            let lhs = &left.borrow();
            let rhs = &right.borrow();

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

        hir::Value::Unary { op, operand, .. } => {
            let operand = &operand.borrow();
            match op {
                hir::UnaryOp::Add => gen_rval_unary_add(ctx, operand),
                hir::UnaryOp::Sub => gen_rval_unary_sub(ctx, operand),
                hir::UnaryOp::Not => gen_rval_unary_not(ctx, operand),
            }
        }

        hir::Value::StructObject { struct_def, fields, .. } => gen_rval_struct_object(ctx, struct_def, fields),

        hir::Value::EnumVariant {
            enum_def,
            variant,
            value,
            ..
        } => gen_rval_enum_variant(ctx, enum_def, variant, &value.borrow()),

        hir::Value::IndexAccess { collection, index, .. } => {
            // Generate the place (pointer) to the indexed element, then load it
            let index_place = gen_place(
                ctx,
                &hir::Value::IndexAccess {
                    span: nitrate_tree::ByteSpan::default(),
                    collection: collection.clone(),
                    index: index.clone(),
                },
            );

            // Resolve through references/pointers to find the underlying
            // collection, then extract the element type for the load.
            let collection_ty = collection.borrow().determine_type(ctx.tab).unwrap();
            let actual_ty = match &collection_ty {
                hir::Type::Reference { to, .. } | hir::Type::Pointer { to, .. } => to.deref().clone(),
                _ => collection_ty.clone(),
            };
            let element_type = match &actual_ty {
                hir::Type::Array { element_type, .. }
                | hir::Type::SliceRef { element_type, .. }
                | hir::Type::SlicePtr { element_type, .. } => element_type.deref().clone(),
                _ => actual_ty,
            };
            let llvm_element_ty = gen_ty(&element_type, &mut ctx.into());
            ctx.bb.build_load(llvm_element_ty, index_place, "index_load").unwrap()
        }

        hir::Value::FieldAccess { expr, field_name, .. } => gen_rval_field_access(ctx, &expr.borrow(), field_name),

        hir::Value::Assign { place, value, .. } => gen_rval_assign(ctx, &place.borrow(), &value.borrow()),

        hir::Value::Deref { place, .. } => gen_rval_deref(ctx, &place.borrow()),

        hir::Value::Cast { value, target_type, .. } => gen_rval_cast(ctx, &value.borrow(), target_type),

        hir::Value::Borrow {
            exclusive,
            mutable,
            place,
            ..
        } => gen_rval_borrow(ctx, *exclusive, *mutable, &place.borrow()),

        hir::Value::List { elements, .. } => gen_rval_list(ctx, elements),
        hir::Value::Tuple { elements, .. } => gen_rval_tuple(ctx, elements),

        hir::Value::If {
            condition,
            true_branch,
            false_branch,
            ..
        } => {
            let condition = &condition.borrow();
            let true_branch = &true_branch.borrow();
            match false_branch {
                None => gen_rval_if(ctx, condition, true_branch, None),
                Some(false_branch) => {
                    let false_branch = false_branch.borrow();
                    gen_rval_if(ctx, condition, true_branch, Some(&false_branch))
                }
            }
        }

        hir::Value::While { condition, body, .. } => {
            let condition = &condition.borrow();
            let body = &body.borrow();
            gen_rval_while(ctx, condition, body);
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Loop { body, .. } => {
            gen_rval_loop(ctx, &body.borrow());
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Break { label, .. } => {
            gen_rval_break(ctx, label.as_deref());
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Continue { label, .. } => {
            gen_rval_continue(ctx, label.as_deref());
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Return { value, .. } => {
            gen_rval_return(ctx, &value.borrow());
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Block { block, .. } => gen_rval_block(ctx, &block.borrow()),

        hir::Value::Call { callee, args, .. } => gen_rval_call(ctx, &callee.borrow(), args),

        hir::Value::MethodCall {
            object,
            method_name,
            args,
            ..
        } => gen_rval_method_call(ctx, &object.borrow(), method_name, args),

        hir::Value::FunctionSymbol { id, .. } => {
            let function_def = id.borrow();
            gen_rval_symbol(ctx, function_def.mangled_name.as_ref().unwrap())
        }

        hir::Value::GlobalVariableSymbol { id, .. } => {
            let global_def = id.borrow();
            gen_rval_symbol(ctx, global_def.mangled_name.as_ref().unwrap())
        }

        hir::Value::LocalVariableSymbol { id, .. } => {
            let local_def = id.borrow();
            gen_rval_symbol(ctx, &local_def.name)
        }

        hir::Value::ParameterSymbol { id, .. } => {
            let param_def = id.borrow();
            gen_rval_symbol(ctx, &param_def.name)
        }
    }
}

pub(crate) fn gen_block<'ctx>(ctx: &mut CodegenCtx<'ctx, '_, '_, '_, '_>, hir_block: &hir::Block) {
    for element in &hir_block.elements {
        match element {
            hir::BlockElement::Expr(expr) => {
                gen_rval(ctx, &expr.borrow());
            }

            hir::BlockElement::Local(local) => {
                let hir_local = local.borrow();
                let local_name = hir_local.name.to_owned();
                let hir_local_ty = &hir_local.ty;
                let hir_local_init = &hir_local.initializer.borrow();

                let llvm_local_ty = gen_ty(hir_local_ty, &mut ctx.into());
                let llvm_local = ctx.bb.build_alloca(llvm_local_ty, &local_name).unwrap();
                let llvm_init_value = gen_rval(ctx, hir_local_init);
                ctx.bb.build_store(llvm_local, llvm_init_value).unwrap();

                ctx.locals.insert(local_name, (llvm_local, llvm_local_ty));
            }
        };
    }
}
