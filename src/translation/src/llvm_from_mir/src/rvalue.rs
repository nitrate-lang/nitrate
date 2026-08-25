use crate::context::CodegenCtx;
use crate::operand::{gen_binary_op, gen_operand, operand_signedness};
use crate::place::{gen_place, get_place_type_for_load};
use crate::ty::gen_ty;
use core::panic;
use inkwell::AddressSpace;
use inkwell::types::BasicType;
use inkwell::values::BasicValueEnum;
use nitrate_mir::prelude as mir;

/// Generate the LLVM value for a MIR Rvalue.
///
/// `result_ty` is the MIR type of the value produced by this rvalue (the
/// destination place's type). It selects signed vs unsigned integer semantics
/// for binary operations.
pub fn gen_rvalue<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, rvalue: &mir::Rvalue) -> BasicValueEnum<'ctx> {
    match rvalue {
        mir::Rvalue::Use(operand) => gen_operand(ctx, operand),

        mir::Rvalue::Ref { region: _, place } => {
            // A borrow's value is the address of the place — no copy.
            let ptr = gen_place(ctx, place);
            ptr.into()
        }

        mir::Rvalue::Len(place) => {
            let slice_ptr = gen_place(ctx, place);
            let ptr_ty = get_place_type_for_load(ctx, place);
            if let mir::MirType::SliceRef { .. } | mir::MirType::SlicePtr { .. } = &*ptr_ty {
                let llvm_slice_ty = gen_ty(&ptr_ty, &mut ctx.ty_ctx());
                let size_ty = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
                let zero = ctx.llvm.i32_type().const_zero();
                let one = ctx.llvm.i32_type().const_int(1, false);
                let len_ptr = unsafe {
                    ctx.builder
                        .build_in_bounds_gep(llvm_slice_ty, slice_ptr, &[zero, one], "len_gep")
                        .unwrap()
                };
                ctx.builder.build_load(size_ty, len_ptr, "len_load").unwrap()
            } else {
                panic!("Len on non-slice type {:?}", &*ptr_ty);
            }
        }

        mir::Rvalue::Cast { value, target_ty } => {
            let val = gen_operand(ctx, value);
            let target_mir: &mir::MirType = target_ty;
            let target_llvm_ty = gen_ty(target_mir, &mut ctx.ty_ctx());
            let val_ty = val.get_type();

            if val_ty == target_llvm_ty {
                return val;
            }

            if val_ty.is_int_type() && target_llvm_ty.is_int_type() {
                let src_width = val_ty.into_int_type().get_bit_width();
                let dst_width = target_llvm_ty.into_int_type().get_bit_width();
                let src_signed = operand_signedness(ctx, value).unwrap_or(false);
                match src_width.cmp(&dst_width) {
                    std::cmp::Ordering::Less if src_signed => ctx
                        .builder
                        .build_int_s_extend(val.into_int_value(), target_llvm_ty.into_int_type(), "cast")
                        .unwrap()
                        .into(),
                    std::cmp::Ordering::Less => ctx
                        .builder
                        .build_int_z_extend(val.into_int_value(), target_llvm_ty.into_int_type(), "cast")
                        .unwrap()
                        .into(),
                    std::cmp::Ordering::Greater => ctx
                        .builder
                        .build_int_truncate(val.into_int_value(), target_llvm_ty.into_int_type(), "cast")
                        .unwrap()
                        .into(),
                    std::cmp::Ordering::Equal => val,
                }
            } else if val_ty.is_float_type() && target_llvm_ty.is_float_type() {
                let f32_ty = ctx.llvm.f32_type().into();
                let f64_ty = ctx.llvm.f64_type().into();
                if val_ty == f32_ty && target_llvm_ty == f64_ty {
                    ctx.builder
                        .build_float_ext(val.into_float_value(), target_llvm_ty.into_float_type(), "cast")
                        .unwrap()
                        .into()
                } else if val_ty == f64_ty && target_llvm_ty == f32_ty {
                    ctx.builder
                        .build_float_trunc(val.into_float_value(), target_llvm_ty.into_float_type(), "cast")
                        .unwrap()
                        .into()
                } else {
                    val
                }
            } else if val_ty.is_int_type() && target_llvm_ty.is_float_type() {
                let src_signed = operand_signedness(ctx, value).unwrap_or(false);
                if src_signed {
                    ctx.builder
                        .build_signed_int_to_float(val.into_int_value(), target_llvm_ty.into_float_type(), "cast")
                        .unwrap()
                        .into()
                } else {
                    ctx.builder
                        .build_unsigned_int_to_float(val.into_int_value(), target_llvm_ty.into_float_type(), "cast")
                        .unwrap()
                        .into()
                }
            } else if val_ty.is_float_type() && target_llvm_ty.is_int_type() {
                if target_mir.is_signed_primitive() {
                    ctx.builder
                        .build_float_to_signed_int(val.into_float_value(), target_llvm_ty.into_int_type(), "cast")
                        .unwrap()
                        .into()
                } else {
                    ctx.builder
                        .build_float_to_unsigned_int(val.into_float_value(), target_llvm_ty.into_int_type(), "cast")
                        .unwrap()
                        .into()
                }
            } else if val_ty.is_pointer_type() && target_llvm_ty.is_pointer_type() {
                ctx.builder.build_bit_cast(val, target_llvm_ty, "cast").unwrap()
            } else if val_ty.is_pointer_type() && target_llvm_ty.is_int_type() {
                ctx.builder
                    .build_ptr_to_int(val.into_pointer_value(), target_llvm_ty.into_int_type(), "cast")
                    .unwrap()
                    .into()
            } else if val_ty.is_int_type() && target_llvm_ty.is_pointer_type() {
                ctx.builder
                    .build_int_to_ptr(val.into_int_value(), target_llvm_ty.into_pointer_type(), "cast")
                    .unwrap()
                    .into()
            } else {
                ctx.builder.build_bit_cast(val, target_llvm_ty, "cast").unwrap()
            }
        }

        mir::Rvalue::BinaryOp { op, lhs, rhs } => gen_binary_op(ctx, *op, lhs, rhs),

        // CheckedBinaryOp lowers identically to the unchecked operation; the
        // overflow flag is not modeled in LLVM IR yet.
        mir::Rvalue::CheckedBinaryOp { op, lhs, rhs } => gen_binary_op(ctx, *op, lhs, rhs),

        mir::Rvalue::UnaryOp { op, operand } => {
            let val = gen_operand(ctx, operand);
            match op {
                mir::MirUnaryOp::Neg => {
                    if val.get_type().is_float_type() {
                        ctx.builder
                            .build_float_neg(val.into_float_value(), "neg")
                            .unwrap()
                            .into()
                    } else {
                        ctx.builder.build_int_neg(val.into_int_value(), "neg").unwrap().into()
                    }
                }
                mir::MirUnaryOp::Not => ctx.builder.build_not(val.into_int_value(), "not").unwrap().into(),
            }
        }

        mir::Rvalue::NullaryOp(op, ty_id) => {
            let llvm_ty = gen_ty(ty_id, &mut ctx.ty_ctx());
            let ret_ty = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            match op {
                mir::NullaryOp::SizeOf => {
                    // Compute the byte size as a plain integer constant. This
                    // avoids inkwell's `size_of()` which emits a GEP-based
                    // constant expression (`ptrtoint (getelementptr ...)`) for
                    // aggregate/zero-sized types instead of a literal.
                    let size = ctx.ty_ctx().size_of(llvm_ty).unwrap_or(0);
                    ret_ty.const_int(size, false).into()
                }
                mir::NullaryOp::AlignOf => {
                    let align = ctx.ty_ctx().abi_align(llvm_ty);
                    ret_ty.const_int(align as u64, false).into()
                }
            }
        }

        mir::Rvalue::Aggregate(kind, operands) => gen_aggregate(ctx, kind, operands),
    }
}

/// Generate an aggregate construction value (tuple, array, struct, or enum).
pub fn gen_aggregate<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_>,
    kind: &mir::AggregateKind,
    operands: &[mir::Operand],
) -> BasicValueEnum<'ctx> {
    match kind {
        mir::AggregateKind::Tuple => {
            let vals: Vec<BasicValueEnum<'ctx>> = operands.iter().map(|op| gen_operand(ctx, op)).collect();
            let llvm_val_types: Vec<inkwell::types::BasicTypeEnum<'ctx>> = vals.iter().map(|v| v.get_type()).collect();
            let struct_ty = ctx.llvm.struct_type(&llvm_val_types, false);
            let alloca = ctx.builder.build_alloca(struct_ty, "tuple").unwrap();
            ctx.builder.build_store(alloca, struct_ty.const_zero()).unwrap();
            for (i, val) in vals.iter().enumerate() {
                let field_ptr = unsafe {
                    ctx.builder
                        .build_in_bounds_gep(
                            struct_ty,
                            alloca,
                            &[
                                ctx.llvm.i32_type().const_zero(),
                                ctx.llvm.i32_type().const_int(i as u64, false),
                            ],
                            "tuple_field",
                        )
                        .unwrap()
                };
                ctx.builder.build_store(field_ptr, *val).unwrap();
            }
            ctx.builder.build_load(struct_ty, alloca, "tuple_val").unwrap()
        }
        mir::AggregateKind::Array(elem_ty) => {
            let vals: Vec<BasicValueEnum<'ctx>> = operands.iter().map(|op| gen_operand(ctx, op)).collect();
            let elem_llvm_ty = gen_ty(elem_ty, &mut ctx.ty_ctx());
            let arr_ty = elem_llvm_ty.array_type(vals.len() as u32);
            let alloca = ctx.builder.build_alloca(arr_ty, "array").unwrap();
            ctx.builder.build_store(alloca, arr_ty.const_zero()).unwrap();
            for (i, val) in vals.iter().enumerate() {
                let elem_ptr = unsafe {
                    ctx.builder
                        .build_in_bounds_gep(
                            arr_ty,
                            alloca,
                            &[
                                ctx.llvm.i32_type().const_zero(),
                                ctx.llvm.i32_type().const_int(i as u64, false),
                            ],
                            "array_elem",
                        )
                        .unwrap()
                };
                ctx.builder.build_store(elem_ptr, *val).unwrap();
            }
            ctx.builder.build_load(arr_ty, alloca, "array_val").unwrap()
        }
        mir::AggregateKind::Struct(name, _field_names) => {
            let vals: Vec<BasicValueEnum<'ctx>> = operands.iter().map(|op| gen_operand(ctx, op)).collect();
            // Prefer the named struct type so the aggregate matches the
            // destination alloca's type; fall back to an anonymous struct.
            let struct_ty = match ctx.module.get_struct_type(name) {
                Some(named) => named,
                None => {
                    let llvm_val_types: Vec<inkwell::types::BasicTypeEnum<'ctx>> =
                        vals.iter().map(|v| v.get_type()).collect();
                    ctx.llvm.struct_type(&llvm_val_types, false)
                }
            };
            let alloca = ctx.builder.build_alloca(struct_ty, "struct").unwrap();
            ctx.builder.build_store(alloca, struct_ty.const_zero()).unwrap();
            for (i, val) in vals.iter().enumerate() {
                let field_ptr = unsafe {
                    ctx.builder
                        .build_in_bounds_gep(
                            struct_ty,
                            alloca,
                            &[
                                ctx.llvm.i32_type().const_zero(),
                                ctx.llvm.i32_type().const_int(i as u64, false),
                            ],
                            "struct_field",
                        )
                        .unwrap()
                };
                ctx.builder.build_store(field_ptr, *val).unwrap();
            }
            ctx.builder.build_load(struct_ty, alloca, "struct_val").unwrap()
        }
        mir::AggregateKind::Enum {
            variant_index, enum_ty, ..
        } => {
            let enum_llvm_ty = gen_ty(enum_ty, &mut ctx.ty_ctx());
            let alloca = ctx.builder.build_alloca(enum_llvm_ty, "enum").unwrap();
            ctx.builder.build_store(alloca, enum_llvm_ty.const_zero()).unwrap();

            if let mir::MirType::Enum { variants, .. } = &**enum_ty {
                let tag_type = match variants.len() {
                    ..=256 => ctx.llvm.i8_type(),
                    ..=65_536 => ctx.llvm.i16_type(),
                    _ => ctx.llvm.i32_type(),
                };

                if let Some(variant) = variants.get(*variant_index as usize).cloned() {
                    if let (Some(payload_mir_ty), Some(payload_operand)) = (variant.payload.as_ref(), operands.first())
                    {
                        let payload_llvm_ty = gen_ty(payload_mir_ty, &mut ctx.ty_ctx());
                        let payload_ptr = ctx
                            .builder
                            .build_bit_cast(alloca, ctx.llvm.ptr_type(AddressSpace::default()), "enum_payload_ptr")
                            .unwrap()
                            .into_pointer_value();
                        let payload_val = gen_operand(ctx, payload_operand);
                        ctx.builder.build_store(payload_ptr, payload_val).unwrap();
                        let _ = payload_llvm_ty;
                    }
                }

                let tag_ptr = unsafe {
                    ctx.builder
                        .build_in_bounds_gep(
                            enum_llvm_ty,
                            alloca,
                            &[
                                ctx.llvm.i32_type().const_zero(),
                                ctx.llvm.i32_type().const_int(1, false),
                            ],
                            "enum_tag",
                        )
                        .unwrap()
                };
                ctx.builder
                    .build_store(tag_ptr, tag_type.const_int(*variant_index as u64, false))
                    .unwrap();
            }

            ctx.builder.build_load(enum_llvm_ty, alloca, "enum_val").unwrap()
        }
    }
}
