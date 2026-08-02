use crate::context::CodegenCtx;
use crate::place::gen_place;
use crate::place::get_place_type_for_load;
use crate::ty::gen_ty;
use inkwell::values::BasicValueEnum;
use nitrate_mir::prelude as mir;

/// Evaluate a MIR Operand into an LLVM BasicValueEnum.
pub fn gen_operand<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, operand: &mir::Operand) -> BasicValueEnum<'ctx> {
    match operand {
        mir::Operand::Copy(place) | mir::Operand::Move(place) => {
            let ptr = gen_place(ctx, place);
            let place_ty = get_place_type_for_load(ctx, place);
            let llvm_ty = gen_ty(&place_ty, &mut ctx.ty_ctx());
            ctx.builder.build_load(llvm_ty, ptr, "load").unwrap()
        }
        mir::Operand::Constant(lit) => gen_literal(ctx, lit),
    }
}

/// Generate a literal constant.
pub fn gen_literal<'ctx>(ctx: &CodegenCtx<'ctx, '_>, lit: &mir::MirLiteral) -> BasicValueEnum<'ctx> {
    match lit {
        mir::MirLiteral::Unit => ctx.llvm.const_struct(&[], false).into(),
        mir::MirLiteral::Bool(b) => {
            let val = if *b { 1 } else { 0 };
            ctx.llvm.bool_type().const_int(val, false).into()
        }
        mir::MirLiteral::I8(v) => ctx.llvm.i8_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I16(v) => ctx.llvm.i16_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I32(v) => ctx.llvm.i32_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I64(v) => ctx.llvm.i64_type().const_int(*v as u64, true).into(),
        mir::MirLiteral::I128(v) => {
            let low = (*v & 0xFFFFFFFFFFFFFFFF) as u64;
            let high = ((*v >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
            ctx.llvm.i128_type().const_int_arbitrary_precision(&[low, high]).into()
        }
        mir::MirLiteral::U8(v) => ctx.llvm.i8_type().const_int(*v as u64, false).into(),
        mir::MirLiteral::U16(v) => ctx.llvm.i16_type().const_int(*v as u64, false).into(),
        mir::MirLiteral::U32(v) => ctx.llvm.i32_type().const_int(*v as u64, false).into(),
        mir::MirLiteral::U64(v) => ctx.llvm.i64_type().const_int(*v, false).into(),
        mir::MirLiteral::U128(v) => {
            let low = (*v & 0xFFFFFFFFFFFFFFFF) as u64;
            let high = ((*v >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
            ctx.llvm.i128_type().const_int_arbitrary_precision(&[low, high]).into()
        }
        mir::MirLiteral::F32(v) => ctx.llvm.f32_type().const_float(v.0 as f64).into(),
        mir::MirLiteral::F64(v) => ctx.llvm.f64_type().const_float(v.0).into(),
        mir::MirLiteral::USize { value, .. } => {
            let ptr_ty = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            ptr_ty.const_int(*value, false).into()
        }
        mir::MirLiteral::Str(s) => {
            let bytes = s.as_bytes();
            let global_ptr = ctx.llvm.const_string(bytes, false);
            let len_val = ctx
                .llvm
                .ptr_sized_int_type(ctx.llvm.target_data(), None)
                .const_int(bytes.len() as u64, false);
            let ptr_ty = ctx.llvm.ptr_type(inkwell::AddressSpace::default());
            let slice_struct = ctx.llvm.struct_type(&[ptr_ty.into(), len_val.get_type().into()], false);
            slice_struct
                .const_named_struct(&[global_ptr.into(), len_val.into()])
                .into()
        }
        mir::MirLiteral::BStr(b) => {
            let global_ptr = ctx.llvm.const_string(b, false);
            let len_val = ctx
                .llvm
                .ptr_sized_int_type(ctx.llvm.target_data(), None)
                .const_int(b.len() as u64, false);
            let ptr_ty = ctx.llvm.ptr_type(inkwell::AddressSpace::default());
            let slice_struct = ctx.llvm.struct_type(&[ptr_ty.into(), len_val.get_type().into()], false);
            slice_struct
                .const_named_struct(&[global_ptr.into(), len_val.into()])
                .into()
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Binary operations
// ─────────────────────────────────────────────────────────────

pub fn gen_binary_op<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_>,
    op: mir::MirBinaryOp,
    lhs: &mir::Operand,
    rhs: &mir::Operand,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = gen_operand(ctx, lhs);
    let llvm_rhs = gen_operand(ctx, rhs);
    let lhs_ty = llvm_lhs.get_type();
    let rhs_ty = llvm_rhs.get_type();

    match op {
        mir::MirBinaryOp::Add => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_add(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "add")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_add(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "add")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Sub => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_sub(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "sub")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_sub(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "sub")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Mul => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_mul(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "mul")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_mul(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "mul")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Div => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_div(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "div")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_unsigned_div(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "div")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Mod => {
            if lhs_ty.is_float_type() && rhs_ty.is_float_type() {
                ctx.builder
                    .build_float_rem(llvm_lhs.into_float_value(), llvm_rhs.into_float_value(), "rem")
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_unsigned_rem(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "rem")
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::And => ctx
            .builder
            .build_and(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "and")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Or => ctx
            .builder
            .build_or(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "or")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Xor => ctx
            .builder
            .build_xor(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "xor")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Shl => ctx
            .builder
            .build_left_shift(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), "shl")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Shr => ctx
            .builder
            .build_right_shift(llvm_lhs.into_int_value(), llvm_rhs.into_int_value(), false, "shr")
            .unwrap()
            .into(),
        mir::MirBinaryOp::Rol => {
            let bit_width = lhs_ty.into_int_type().get_bit_width();
            let mask = ctx
                .llvm
                .custom_width_int_type(bit_width)
                .const_int((bit_width - 1) as u64, false);
            let shift = ctx
                .builder
                .build_and(llvm_rhs.into_int_value(), mask, "rol_mask")
                .unwrap();
            let inv_shift = ctx
                .builder
                .build_int_sub(
                    ctx.llvm
                        .custom_width_int_type(bit_width)
                        .const_int(bit_width as u64, false),
                    shift,
                    "rol_inv",
                )
                .unwrap();
            let left = ctx
                .builder
                .build_left_shift(llvm_lhs.into_int_value(), shift, "rol_left")
                .unwrap();
            let right = ctx
                .builder
                .build_right_shift(llvm_lhs.into_int_value(), inv_shift, false, "rol_right")
                .unwrap();
            ctx.builder.build_or(left, right, "rol").unwrap().into()
        }
        mir::MirBinaryOp::Ror => {
            let bit_width = lhs_ty.into_int_type().get_bit_width();
            let mask = ctx
                .llvm
                .custom_width_int_type(bit_width)
                .const_int((bit_width - 1) as u64, false);
            let shift = ctx
                .builder
                .build_and(llvm_rhs.into_int_value(), mask, "ror_mask")
                .unwrap();
            let inv_shift = ctx
                .builder
                .build_int_sub(
                    ctx.llvm
                        .custom_width_int_type(bit_width)
                        .const_int(bit_width as u64, false),
                    shift,
                    "ror_inv",
                )
                .unwrap();
            let right = ctx
                .builder
                .build_right_shift(llvm_lhs.into_int_value(), shift, false, "ror_right")
                .unwrap();
            let left = ctx
                .builder
                .build_left_shift(llvm_lhs.into_int_value(), inv_shift, "ror_left")
                .unwrap();
            ctx.builder.build_or(right, left, "ror").unwrap().into()
        }
        mir::MirBinaryOp::LogicAnd => {
            let lhs_zero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::EQ,
                    llvm_lhs.into_int_value(),
                    lhs_ty.into_int_type().const_zero(),
                    "lhs_bool",
                )
                .unwrap();
            let rhs_zero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::EQ,
                    llvm_rhs.into_int_value(),
                    rhs_ty.into_int_type().const_zero(),
                    "rhs_bool",
                )
                .unwrap();
            let lhs_true = ctx.builder.build_not(lhs_zero, "lhs_true").unwrap();
            let rhs_true = ctx.builder.build_not(rhs_zero, "rhs_true").unwrap();
            ctx.builder.build_and(lhs_true, rhs_true, "land").unwrap().into()
        }
        mir::MirBinaryOp::LogicOr => {
            let lhs_nonzero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::NE,
                    llvm_lhs.into_int_value(),
                    lhs_ty.into_int_type().const_zero(),
                    "lhs_bool",
                )
                .unwrap();
            let rhs_nonzero = ctx
                .builder
                .build_int_compare(
                    inkwell::IntPredicate::NE,
                    llvm_rhs.into_int_value(),
                    rhs_ty.into_int_type().const_zero(),
                    "rhs_bool",
                )
                .unwrap();
            ctx.builder.build_or(lhs_nonzero, rhs_nonzero, "lor").unwrap().into()
        }
        mir::MirBinaryOp::Lt => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OLT,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "lt",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::ULT,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "lt",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Gt => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OGT,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "gt",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::UGT,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "gt",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Lte => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OLE,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "lte",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::ULE,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "lte",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Gte => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OGE,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "gte",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::UGE,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "gte",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Eq => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::OEQ,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "eq",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::EQ,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "eq",
                    )
                    .unwrap()
                    .into()
            }
        }
        mir::MirBinaryOp::Ne => {
            if lhs_ty.is_float_type() {
                ctx.builder
                    .build_float_compare(
                        inkwell::FloatPredicate::ONE,
                        llvm_lhs.into_float_value(),
                        llvm_rhs.into_float_value(),
                        "ne",
                    )
                    .unwrap()
                    .into()
            } else {
                ctx.builder
                    .build_int_compare(
                        inkwell::IntPredicate::NE,
                        llvm_lhs.into_int_value(),
                        llvm_rhs.into_int_value(),
                        "ne",
                    )
                    .unwrap()
                    .into()
            }
        }
    }
}
