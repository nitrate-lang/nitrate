use std::collections::HashMap;

use inkwell::{
    basic_block::BasicBlock,
    types::BasicType,
    values::{BasicValueEnum, PointerValue},
};

use interned_string::IString;
use nitrate_hir::prelude as hir;
use nitrate_hir_get_type::{TypeInferenceCtx, get_type};
use nitrate_llvm::LLVMContext;

pub struct RvalGenCtx<'ctx, 'store, 'tab, 'builder, 'ret, 'endb> {
    pub store: &'store hir::Store,
    pub tab: &'tab hir::SymbolTab,
    pub llvm: &'ctx LLVMContext,

    pub bb: &'builder inkwell::builder::Builder<'ctx>,
    pub ret: &'ret PointerValue<'ctx>,
    pub endb: &'endb BasicBlock<'ctx>,
    pub locals: HashMap<IString, PointerValue<'ctx>>,
}

pub(crate) fn gen_rval_lit_unit<'ctx>(ctx: &'ctx RvalGenCtx) -> BasicValueEnum<'ctx> {
    /*
     * The Unit Type is an empty struct
     */

    ctx.llvm.const_struct(&[], false).into()
}

fn gen_rval_lit_bool<'ctx>(ctx: &'ctx RvalGenCtx, value: bool) -> BasicValueEnum<'ctx> {
    /*
     * The Bool Type is represented as an i1.
     * No sign extension is performed.
     */

    match value {
        true => ctx.llvm.bool_type().const_int(1, false).into(),
        false => ctx.llvm.bool_type().const_int(0, false).into(),
    }
}

fn gen_rval_lit_i8<'ctx>(ctx: &'ctx RvalGenCtx, value: i8) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i8 type.
     * Sign extension is performed.
     */

    ctx.llvm.i8_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i16<'ctx>(ctx: &'ctx RvalGenCtx, value: i16) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i16 type.
     * Sign extension is performed.
     */

    ctx.llvm.i16_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i32<'ctx>(ctx: &'ctx RvalGenCtx, value: i32) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i32 type.
     * Sign extension is performed.
     */

    ctx.llvm.i32_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i64<'ctx>(ctx: &'ctx RvalGenCtx, value: i64) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i64 type.
     * Sign extension is performed.
     */

    ctx.llvm.i64_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i128<'ctx>(ctx: &'ctx RvalGenCtx, value: i128) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i128 type.
     * Sign extension is not performed because the value is constructed
     * from its low and high parts directly.
     */

    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;

    ctx.llvm
        .i128_type()
        .const_int_arbitrary_precision(&[low, high])
        .into()
}

fn gen_rval_lit_u8<'ctx>(ctx: &'ctx RvalGenCtx, value: u8) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i8 type.
     * No sign extension is performed.
     */

    ctx.llvm.i8_type().const_int(value as u64, false).into()
}

fn gen_rval_lit_u16<'ctx>(ctx: &'ctx RvalGenCtx, value: u16) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i16 type.
     * No sign extension is performed.
     */

    ctx.llvm.i16_type().const_int(value as u64, false).into()
}

fn gen_rval_lit_u32<'ctx>(ctx: &'ctx RvalGenCtx, value: u32) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i32 type.
     * No sign extension is performed.
     */

    ctx.llvm.i32_type().const_int(value as u64, false).into()
}

fn gen_rval_lit_u64<'ctx>(ctx: &'ctx RvalGenCtx, value: u64) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i64 type.
     * No sign extension is performed.
     */

    ctx.llvm.i64_type().const_int(value, false).into()
}

fn gen_rval_lit_u128<'ctx>(ctx: &'ctx RvalGenCtx, value: u128) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i128 type.
     * Sign extension is not performed because the value is constructed
     * from its low and high parts directly.
     */

    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;

    ctx.llvm
        .i128_type()
        .const_int_arbitrary_precision(&[low, high])
        .into()
}

fn gen_rval_lit_f32<'ctx>(ctx: &'ctx RvalGenCtx, value: f32) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM f32 type.
     */

    ctx.llvm.f32_type().const_float(value as f64).into()
}

fn gen_rval_lit_f64<'ctx>(ctx: &'ctx RvalGenCtx, value: f64) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM f64 type.
     */

    ctx.llvm.f64_type().const_float(value).into()
}

fn gen_rval_lit_string<'ctx>(ctx: &'ctx RvalGenCtx, value: &str) -> BasicValueEnum<'ctx> {
    /*
     * Intern the string literal in the LLVM module's global string table.
     * No null terminator is added.
     */

    ctx.llvm.const_string(value.as_bytes(), false).into()
}

fn gen_rval_lit_bstring<'ctx>(ctx: &'ctx RvalGenCtx, value: &[u8]) -> BasicValueEnum<'ctx> {
    /*
     * Intern the byte string literal in the LLVM module's global string table.
     * No null terminator is added. It is treated as a raw byte array.
     */

    ctx.llvm.const_string(value, false).into()
}

fn gen_rval_add<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * Unsigned integer addition:
     * - The result is modulo 2^n, where n is the bit width of the type.
     *
     * Floating-point addition:
     * - Follows the IEEE 754 standard for floating-point arithmetic.
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fadd = ctx
            .bb
            .build_float_add(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        return fadd.into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let iadd = ctx
            .bb
            .build_int_add(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        return iadd.into();
    } else {
        panic!("Addition not implemented for this type");
    }
}

fn gen_rval_sub<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * Signed and unsigned integer subtraction:
     * - The result is modulo 2^n, where n is the bit width of the type.
     *
     * Floating-point subtraction:
     * - Follows the IEEE 754 standard for floating-point arithmetic.
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fsub = ctx
            .bb
            .build_float_sub(lhs.into_float_value(), rhs.into_float_value(), "")
            .unwrap();

        return fsub.into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let isub = ctx
            .bb
            .build_int_sub(lhs.into_int_value(), rhs.into_int_value(), "")
            .unwrap();

        return isub.into();
    } else {
        panic!("Subtraction not implemented for this type");
    }
}

fn gen_rval_mul<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fmul = ctx
            .bb
            .build_float_mul(lhs.into_float_value(), rhs.into_float_value(), "");
        return fmul.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let imul = ctx
            .bb
            .build_int_mul(lhs.into_int_value(), rhs.into_int_value(), "");
        return imul.unwrap().into();
    } else {
        panic!("Multiplication not implemented for this type");
    }
}

fn gen_rval_div<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fdiv =
            ctx.bb
                .build_float_div(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "");
        return fdiv.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(
            lhs,
            &TypeInferenceCtx {
                store: ctx.store,
                tab: ctx.tab,
            },
        )
        .expect("Failed to get type")
        .is_signed_primitive();

        let div = if is_signed {
            ctx.bb
                .build_int_signed_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        } else {
            ctx.bb
                .build_int_unsigned_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        };

        return div.unwrap().into();
    } else {
        panic!("Division not implemented for this type");
    }
}

fn gen_rval_rem<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let frem =
            ctx.bb
                .build_float_rem(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "");
        return frem.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(
            lhs,
            &TypeInferenceCtx {
                store: ctx.store,
                tab: ctx.tab,
            },
        )
        .expect("Failed to get type")
        .is_signed_primitive();

        let rem = if is_signed {
            ctx.bb
                .build_int_signed_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        } else {
            ctx.bb
                .build_int_unsigned_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        };

        return rem.unwrap().into();
    } else {
        panic!("Remainder not implemented for this type");
    }
}

fn gen_rval_and<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);

    let and = ctx
        .bb
        .build_and(lhs.into_int_value(), rhs.into_int_value(), "");
    return and.unwrap().into();
}

fn gen_rval_or<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);

    let or = ctx
        .bb
        .build_or(lhs.into_int_value(), rhs.into_int_value(), "");
    return or.unwrap().into();
}

fn gen_rval_xor<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);

    let xor = ctx
        .bb
        .build_xor(lhs.into_int_value(), rhs.into_int_value(), "");
    return xor.unwrap().into();
}

fn gen_rval_shl<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);

    let shl = ctx
        .bb
        .build_left_shift(lhs.into_int_value(), rhs.into_int_value(), "");
    return shl.unwrap().into();
}

fn gen_rval_shr<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let sign_extend = get_type(
        lhs,
        &TypeInferenceCtx {
            store: ctx.store,
            tab: ctx.tab,
        },
    )
    .expect("Failed to get type")
    .is_signed_primitive();

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);

    let shr = ctx
        .bb
        .build_right_shift(lhs.into_int_value(), rhs.into_int_value(), sign_extend, "");
    return shr.unwrap().into();
}

fn gen_rval_rol<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * Bitwise rotate left operation formula:
     * rol(x, n) = (x << (n % bit_width)) | (x >> ((bit_width - (n % bit_width)) % bit_width))
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
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

    or.into()
}

fn gen_rval_ror<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * Bitwise rotate right operation formula:
     * ror(x, n) = (x >> (n % bit_width)) | (x << ((bit_width - (n % bit_width)) % bit_width))
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
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

    or.into()
}

fn gen_rval_land<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

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
    let load = ctx.bb.build_load(bool, land_result, "land_load").unwrap();
    load.into()
}

fn gen_rval_lor<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

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
    let load = ctx.bb.build_load(bool, lor_result, "lor_load").unwrap();
    load.into()
}

fn gen_rval_lt<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx.bb.build_float_compare(
            inkwell::FloatPredicate::OLT,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(
            lhs,
            &TypeInferenceCtx {
                store: ctx.store,
                tab: ctx.tab,
            },
        )
        .expect("Failed to get type")
        .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::SLT,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::ULT,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        };

        return cmp.unwrap().into();
    } else {
        panic!("Comparison not implemented for this type");
    }
}

fn gen_rval_gt<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx.bb.build_float_compare(
            inkwell::FloatPredicate::OGT,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(
            lhs,
            &TypeInferenceCtx {
                store: ctx.store,
                tab: ctx.tab,
            },
        )
        .expect("Failed to get type")
        .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::SGT,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::UGT,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        };

        return cmp.unwrap().into();
    } else {
        panic!("Comparison not implemented for this type");
    }
}

fn gen_rval_lte<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx.bb.build_float_compare(
            inkwell::FloatPredicate::OLE,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(
            lhs,
            &TypeInferenceCtx {
                store: ctx.store,
                tab: ctx.tab,
            },
        )
        .expect("Failed to get type")
        .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::SLE,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::ULE,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        };

        return cmp.unwrap().into();
    } else {
        panic!("Comparison not implemented for this type");
    }
}

fn gen_rval_gte<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(ctx, lhs);
    let llvm_rhs = gen_rval(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx.bb.build_float_compare(
            inkwell::FloatPredicate::OGE,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(
            lhs,
            &TypeInferenceCtx {
                store: ctx.store,
                tab: ctx.tab,
            },
        )
        .expect("Failed to get type")
        .is_signed_primitive();

        let cmp = if is_signed {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::SGE,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            ctx.bb.build_int_compare(
                inkwell::IntPredicate::UGE,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        };

        return cmp.unwrap().into();
    } else {
        panic!("Comparison not implemented for this type");
    }
}

fn gen_rval_eq<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx.bb.build_float_compare(
            inkwell::FloatPredicate::OEQ,
            lhs.into_float_value(),
            rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let icmp = ctx.bb.build_int_compare(
            inkwell::IntPredicate::EQ,
            lhs.into_int_value(),
            rhs.into_int_value(),
            "",
        );
        return icmp.unwrap().into();
    } else {
        panic!("Comparison not implemented for this type");
    }
}

fn gen_rval_ne<'ctx>(
    ctx: &'ctx RvalGenCtx,
    lhs: &hir::Value,
    rhs: &hir::Value,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(ctx, lhs);
    let rhs = gen_rval(ctx, rhs);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = ctx.bb.build_float_compare(
            inkwell::FloatPredicate::ONE,
            lhs.into_float_value(),
            rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let icmp = ctx.bb.build_int_compare(
            inkwell::IntPredicate::NE,
            lhs.into_int_value(),
            rhs.into_int_value(),
            "",
        );
        return icmp.unwrap().into();
    } else {
        panic!("Comparison not implemented for this type");
    }
}

fn gen_if<'ctx>(
    ctx: &'ctx RvalGenCtx,
    condition: &hir::Value,
    true_branch: &hir::Block,
    false_branch: Option<&hir::Block>,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     * // TODO: implement if-else codegen
     */

    unimplemented!()
}

fn gen_while<'ctx>(
    ctx: &'ctx RvalGenCtx,
    condition: &hir::Value,
    body: &hir::Block,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     * // TODO: implement while loop codegen
     */

    unimplemented!()
}

fn gen_loop<'ctx>(ctx: &'ctx RvalGenCtx, body: &hir::Block) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     * // TODO: implement loop codegen
     */

    unimplemented!()
}

fn gen_break<'ctx>(ctx: &'ctx RvalGenCtx, label: Option<&str>) {
    /*
     * // TODO: add documentation
     * // TODO: implement break codegen
     */

    unimplemented!()
}

fn gen_continue<'ctx>(ctx: &'ctx RvalGenCtx, label: Option<&str>) {
    /*
     * // TODO: add documentation
     * // TODO: implement continue codegen
     */

    unimplemented!()
}

fn gen_return<'ctx>(ctx: &'ctx RvalGenCtx, value: &hir::Value) {
    /*
     * // TODO: add documentation
     */

    let llvm_value = gen_rval(ctx, value);

    ctx.bb.build_store(*ctx.ret, llvm_value).unwrap();
    ctx.bb.build_unconditional_branch(*ctx.endb).unwrap();
}

pub(crate) fn gen_rval<'ctx>(
    ctx: &'ctx RvalGenCtx,
    hir_value: &hir::Value,
) -> BasicValueEnum<'ctx> {
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

        hir::Value::StructObject {
            struct_path,
            fields,
        } => {
            // TODO: implement struct object codegen
            unimplemented!()
        }

        hir::Value::EnumVariant {
            enum_path,
            variant,
            value,
        } => {
            // TODO: implement enum variant codegen
            unimplemented!()
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
            // TODO: implement unary operation codegen
            unimplemented!()
        }

        hir::Value::FieldAccess { expr, field } => {
            // TODO: implement field access codegen
            unimplemented!()
        }

        hir::Value::IndexAccess { collection, index } => {
            // TODO: implement index access codegen
            unimplemented!()
        }

        hir::Value::Assign { place, value } => {
            // TODO: implement assignment codegen
            unimplemented!()
        }

        hir::Value::Deref { place } => {
            // TODO: implement dereference codegen
            unimplemented!()
        }

        hir::Value::Cast { expr, to } => {
            // TODO: implement cast codegen
            unimplemented!()
        }

        hir::Value::Borrow {
            exclusive,
            mutable,
            place,
        } => {
            // TODO: implement borrow codegen
            unimplemented!()
        }

        hir::Value::List { elements } => {
            // TODO: implement list codegen
            unimplemented!()
        }

        hir::Value::Tuple { elements } => {
            // TODO: implement tuple codegen
            unimplemented!()
        }

        hir::Value::If {
            condition,
            true_branch,
            false_branch,
        } => {
            let condition = &ctx.store[condition].borrow();
            let true_branch = &ctx.store[true_branch].borrow();

            match false_branch {
                None => gen_if(ctx, condition, true_branch, None),
                Some(fb) => {
                    let fb = &ctx.store[fb].borrow();
                    gen_if(ctx, condition, true_branch, Some(fb))
                }
            }
        }

        hir::Value::While { condition, body } => {
            let condition = &ctx.store[condition].borrow();
            let body = &ctx.store[body].borrow();
            gen_while(ctx, condition, body)
        }

        hir::Value::Loop { body } => {
            let body = &ctx.store[body].borrow();
            gen_loop(ctx, body)
        }

        hir::Value::Break { label } => {
            gen_break(ctx, label.as_deref());
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Continue { label } => {
            gen_continue(ctx, label.as_deref());
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Return { value } => {
            let value = &ctx.store[value].borrow();
            gen_return(ctx, value);
            gen_rval_lit_unit(ctx)
        }

        hir::Value::Block { block } => {
            // TODO: implement block codegen
            unimplemented!()
        }

        hir::Value::Closure { captures, callee } => {
            // TODO: implement closure codegen
            unimplemented!()
        }

        hir::Value::Call {
            callee,
            positional,
            named,
        } => {
            // TODO: implement function call codegen
            unimplemented!()
        }

        hir::Value::MethodCall {
            object,
            method_name,
            positional,
            named,
        } => {
            // TODO: implement method call codegen
            unimplemented!()
        }

        hir::Value::Symbol { path } => {
            // TODO: implement symbol reference codegen
            unimplemented!()
        }
    }
}
