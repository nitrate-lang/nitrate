use core::panic;

use inkwell::{
    basic_block::BasicBlock,
    values::{BasicValueEnum, PointerValue},
};
use nitrate_hir::{Store, SymbolTab, prelude as hir};
use nitrate_hir_get_type::{TypeInferenceCtx, get_type};
use nitrate_llvm::LLVMContext;

fn gen_rval_lit_unit<'ctx>(ctx: &'ctx LLVMContext) -> BasicValueEnum<'ctx> {
    /*
     * The Unit Type is an empty struct
     */

    ctx.const_struct(&[], false).into()
}

fn gen_rval_lit_bool<'ctx>(ctx: &'ctx LLVMContext, value: bool) -> BasicValueEnum<'ctx> {
    /*
     * The Bool Type is represented as an i1.
     * No sign extension is performed.
     */

    match value {
        true => ctx.bool_type().const_int(1, false).into(),
        false => ctx.bool_type().const_int(0, false).into(),
    }
}

fn gen_rval_lit_i8<'ctx>(ctx: &'ctx LLVMContext, value: i8) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i8 type.
     * Sign extension is performed.
     */

    ctx.i8_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i16<'ctx>(ctx: &'ctx LLVMContext, value: i16) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i16 type.
     * Sign extension is performed.
     */

    ctx.i16_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i32<'ctx>(ctx: &'ctx LLVMContext, value: i32) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i32 type.
     * Sign extension is performed.
     */

    ctx.i32_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i64<'ctx>(ctx: &'ctx LLVMContext, value: i64) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i64 type.
     * Sign extension is performed.
     */

    ctx.i64_type().const_int(value as u64, true).into()
}

fn gen_rval_lit_i128<'ctx>(ctx: &'ctx LLVMContext, value: i128) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
    let i128 = ctx.i128_type().const_int_arbitrary_precision(&[low, high]);
    i128.into()
}

fn gen_rval_lit_u8<'ctx>(ctx: &'ctx LLVMContext, value: u8) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i8 type.
     * No sign extension is performed.
     */

    ctx.i8_type().const_int(value as u64, false).into()
}

fn gen_rval_lit_u16<'ctx>(ctx: &'ctx LLVMContext, value: u16) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i16 type.
     * No sign extension is performed.
     */

    ctx.i16_type().const_int(value as u64, false).into()
}

fn gen_rval_lit_u32<'ctx>(ctx: &'ctx LLVMContext, value: u32) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i32 type.
     * No sign extension is performed.
     */

    ctx.i32_type().const_int(value as u64, false).into()
}

fn gen_rval_lit_u64<'ctx>(ctx: &'ctx LLVMContext, value: u64) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM i64 type.
     * No sign extension is performed.
     */

    ctx.i64_type().const_int(value, false).into()
}

fn gen_rval_lit_u128<'ctx>(ctx: &'ctx LLVMContext, value: u128) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let low = (value & 0xFFFFFFFFFFFFFFFF) as u64;
    let high = ((value >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
    let u128 = ctx.i128_type().const_int_arbitrary_precision(&[low, high]);
    u128.into()
}

fn gen_rval_lit_f32<'ctx>(ctx: &'ctx LLVMContext, value: f32) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM f32 type.
     */

    ctx.f32_type().const_float(value as f64).into()
}

fn gen_rval_lit_f64<'ctx>(ctx: &'ctx LLVMContext, value: f64) -> BasicValueEnum<'ctx> {
    /*
     * Direct correspondence to LLVM f64 type.
     */

    ctx.f64_type().const_float(value).into()
}

fn gen_rval_lit_string<'ctx>(ctx: &'ctx LLVMContext, value: &str) -> BasicValueEnum<'ctx> {
    /*
     * Intern the string literal in the LLVM module's global string table.
     * No null terminator is added.
     */

    ctx.const_string(value.as_bytes(), false).into()
}

fn gen_rval_lit_bstring<'ctx>(ctx: &'ctx LLVMContext, value: &[u8]) -> BasicValueEnum<'ctx> {
    /*
     * Intern the byte string literal in the LLVM module's global string table.
     * No null terminator is added. It is treated as a raw byte array.
     */

    ctx.const_string(value, false).into()
}

fn gen_rval_add<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fadd = bb.build_float_add(lhs.into_float_value(), rhs.into_float_value(), "");
        return fadd.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let iadd = bb.build_int_add(lhs.into_int_value(), rhs.into_int_value(), "");
        return iadd.unwrap().into();
    } else {
        panic!("Addition not implemented for this type");
    }
}

fn gen_rval_sub<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fsub = bb.build_float_sub(lhs.into_float_value(), rhs.into_float_value(), "");
        return fsub.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let isub = bb.build_int_sub(lhs.into_int_value(), rhs.into_int_value(), "");
        return isub.unwrap().into();
    } else {
        panic!("Subtraction not implemented for this type");
    }
}

fn gen_rval_mul<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fmul = bb.build_float_mul(lhs.into_float_value(), rhs.into_float_value(), "");
        return fmul.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let imul = bb.build_int_mul(lhs.into_int_value(), rhs.into_int_value(), "");
        return imul.unwrap().into();
    } else {
        panic!("Multiplication not implemented for this type");
    }
}

fn gen_rval_div<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let llvm_rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fdiv = bb.build_float_div(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "");
        return fdiv.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(lhs, &TypeInferenceCtx { store, tab })
            .expect("Failed to get type")
            .is_signed_primitive();

        let div = if is_signed {
            bb.build_int_signed_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        } else {
            bb.build_int_unsigned_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        };

        return div.unwrap().into();
    } else {
        panic!("Division not implemented for this type");
    }
}

fn gen_rval_rem<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let llvm_rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let frem = bb.build_float_rem(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "");
        return frem.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(lhs, &TypeInferenceCtx { store, tab })
            .expect("Failed to get type")
            .is_signed_primitive();

        let rem = if is_signed {
            bb.build_int_signed_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        } else {
            bb.build_int_unsigned_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "")
        };

        return rem.unwrap().into();
    } else {
        panic!("Remainder not implemented for this type");
    }
}

fn gen_rval_and<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);

    let and = bb.build_and(lhs.into_int_value(), rhs.into_int_value(), "");
    return and.unwrap().into();
}

fn gen_rval_or<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);

    let or = bb.build_or(lhs.into_int_value(), rhs.into_int_value(), "");
    return or.unwrap().into();
}

fn gen_rval_xor<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);

    let xor = bb.build_xor(lhs.into_int_value(), rhs.into_int_value(), "");
    return xor.unwrap().into();
}

fn gen_rval_shl<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);

    let shl = bb.build_left_shift(lhs.into_int_value(), rhs.into_int_value(), "");
    return shl.unwrap().into();
}

fn gen_rval_shr<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let sign_extend = get_type(lhs, &TypeInferenceCtx { store, tab })
        .expect("Failed to get type")
        .is_signed_primitive();

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);

    let shr = bb.build_right_shift(lhs.into_int_value(), rhs.into_int_value(), sign_extend, "");
    return shr.unwrap().into();
}

fn gen_rval_rol<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let _lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let _rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);

    // TODO: implement rotate left
    unimplemented!()
}

fn gen_rval_ror<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let _lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let _rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);

    // TODO: implement rotate right
    unimplemented!()
}

fn gen_rval_land<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let parent_function = bb.get_insert_block().unwrap().get_parent().unwrap();
    let bool = ctx.bool_type();

    let rhs_bb = ctx.append_basic_block(parent_function, "land_rhs");
    let end_bb = ctx.append_basic_block(parent_function, "land_join");

    /**************************************************************************/
    // 1. Allocate space for the result
    let land_result = bb.build_alloca(bool, "land_result").unwrap();

    /**************************************************************************/
    // 2. Evaluate LHS; if true, skip RHS
    let lhs_val = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    bb.build_store(land_result, lhs_val).unwrap();
    bb.build_conditional_branch(lhs_val.into_int_value(), rhs_bb, end_bb)
        .unwrap();

    /**************************************************************************/
    // 3. Evaluate RHS
    bb.position_at_end(rhs_bb);
    let rhs_val = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    bb.build_store(land_result, rhs_val).unwrap();
    bb.build_unconditional_branch(end_bb).unwrap();

    /**************************************************************************/
    // 4. Join block and load result
    bb.position_at_end(end_bb);
    let load = bb.build_load(bool, land_result, "land_load").unwrap();
    load.into()
}

fn gen_rval_lor<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let parent_function = bb.get_insert_block().unwrap().get_parent().unwrap();
    let bool = ctx.bool_type();

    let rhs_bb = ctx.append_basic_block(parent_function, "lor_rhs");
    let end_bb = ctx.append_basic_block(parent_function, "lor_join");

    /**************************************************************************/
    // 1. Allocate space for the result
    let lor_result = bb.build_alloca(bool, "lor_result").unwrap();

    /**************************************************************************/
    // 2. Evaluate LHS; if true, skip RHS
    let lhs_val = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    bb.build_store(lor_result, lhs_val).unwrap();
    bb.build_conditional_branch(lhs_val.into_int_value(), end_bb, rhs_bb)
        .unwrap();

    /**************************************************************************/
    // 3. Evaluate RHS
    bb.position_at_end(rhs_bb);
    let rhs_val = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    bb.build_store(lor_result, rhs_val).unwrap();
    bb.build_unconditional_branch(end_bb).unwrap();

    /**************************************************************************/
    // 4. Join block and load result
    bb.position_at_end(end_bb);
    let load = bb.build_load(bool, lor_result, "lor_load").unwrap();
    load.into()
}

fn gen_rval_lt<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let llvm_rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = bb.build_float_compare(
            inkwell::FloatPredicate::OLT,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(lhs, &TypeInferenceCtx { store, tab })
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            bb.build_int_compare(
                inkwell::IntPredicate::SLT,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            bb.build_int_compare(
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
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let llvm_rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = bb.build_float_compare(
            inkwell::FloatPredicate::OGT,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(lhs, &TypeInferenceCtx { store, tab })
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            bb.build_int_compare(
                inkwell::IntPredicate::SGT,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            bb.build_int_compare(
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
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let llvm_rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = bb.build_float_compare(
            inkwell::FloatPredicate::OLE,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(lhs, &TypeInferenceCtx { store, tab })
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            bb.build_int_compare(
                inkwell::IntPredicate::SLE,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            bb.build_int_compare(
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
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let llvm_lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let llvm_rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = bb.build_float_compare(
            inkwell::FloatPredicate::OGE,
            llvm_lhs.into_float_value(),
            llvm_rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let is_signed = get_type(lhs, &TypeInferenceCtx { store, tab })
            .expect("Failed to get type")
            .is_signed_primitive();

        let cmp = if is_signed {
            bb.build_int_compare(
                inkwell::IntPredicate::SGE,
                llvm_lhs.into_int_value(),
                llvm_rhs.into_int_value(),
                "",
            )
        } else {
            bb.build_int_compare(
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
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = bb.build_float_compare(
            inkwell::FloatPredicate::OEQ,
            lhs.into_float_value(),
            rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let icmp = bb.build_int_compare(
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
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    /*
     * // TODO: add documentation
     */

    let lhs = gen_rval(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = gen_rval(rhs, bb, ret_alloc, end_block, ctx, store, tab);
    let lhs_ty = lhs.get_type();
    let rhs_ty = rhs.get_type();

    if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
        let fcmp = bb.build_float_compare(
            inkwell::FloatPredicate::ONE,
            lhs.into_float_value(),
            rhs.into_float_value(),
            "",
        );
        return fcmp.unwrap().into();
    } else if lhs_ty.is_int_type() && rhs_ty.is_int_type() {
        let icmp = bb.build_int_compare(
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

pub(crate) fn gen_rval<'ctx>(
    hir_value: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret: Option<&PointerValue<'ctx>>,
    endb: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
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
            let lhs = &store[left].borrow();
            let rhs = &store[right].borrow();

            match op {
                hir::BinaryOp::Add => gen_rval_add(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Sub => gen_rval_sub(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Mul => gen_rval_mul(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Div => gen_rval_div(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Mod => gen_rval_rem(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::And => gen_rval_and(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Or => gen_rval_or(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Xor => gen_rval_xor(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Shl => gen_rval_shl(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Shr => gen_rval_shr(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Rol => gen_rval_rol(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Ror => gen_rval_ror(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::LogicAnd => gen_rval_land(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::LogicOr => gen_rval_lor(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Lt => gen_rval_lt(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Gt => gen_rval_gt(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Lte => gen_rval_lte(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Gte => gen_rval_gte(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Eq => gen_rval_eq(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Ne => gen_rval_ne(lhs, rhs, bb, ret, endb, ctx, store, tab),
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
            // TODO: implement if expression codegen
            unimplemented!()
        }

        hir::Value::While { condition, body } => {
            // TODO: implement while loop codegen
            unimplemented!()
        }

        hir::Value::Loop { body } => {
            // TODO: implement loop codegen
            unimplemented!()
        }

        hir::Value::Break { label } => {
            // TODO: implement break codegen
            unimplemented!()
        }

        hir::Value::Continue { label } => {
            // TODO: implement continue codegen
            unimplemented!()
        }

        hir::Value::Return { value } => {
            // TODO: implement return codegen
            unimplemented!()
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
