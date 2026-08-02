use core::panic;

use crate::context::CodegenCtx;
use crate::operand::{gen_binary_op, gen_literal, gen_operand};
use crate::place::{gen_place, get_place_type_for_load};
use crate::ty::gen_ty;
use inkwell::types::{BasicType, BasicTypeEnum};
use inkwell::values::BasicValueEnum;
use nitrate_mir::prelude as mir;

pub fn gen_rvalue<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, rvalue: &mir::Rvalue) -> BasicValueEnum<'ctx> {
    match rvalue {
        mir::Rvalue::Use(operand) => gen_operand(ctx, operand),

        mir::Rvalue::Ref { region: _, place } => {
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
                panic!("Len on non-slice type");
            }
        }

        mir::Rvalue::Cast { value, target_ty } => {
            let val = gen_operand(ctx, value);
            let target_llvm_ty = gen_ty(&*target_ty, &mut ctx.ty_ctx());
            let val_ty = val.get_type();

            if val_ty == target_llvm_ty {
                return val;
            }

            if val_ty.is_int_type() && target_llvm_ty.is_int_type() {
                let src_width = val_ty.into_int_type().get_bit_width();
                let dst_width = target_llvm_ty.into_int_type().get_bit_width();
                match src_width.cmp(&dst_width) {
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
                let f32_ty: BasicTypeEnum = ctx.llvm.f32_type().into();
                let f64_ty: BasicTypeEnum = ctx.llvm.f64_type().into();
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
                    ctx.builder.build_bit_cast(val, target_llvm_ty, "cast").unwrap()
                }
            } else if val_ty.is_int_type() && target_llvm_ty.is_float_type() {
                ctx.builder
                    .build_unsigned_int_to_float(val.into_int_value(), target_llvm_ty.into_float_type(), "cast")
                    .unwrap()
                    .into()
            } else if val_ty.is_float_type() && target_llvm_ty.is_int_type() {
                ctx.builder
                    .build_float_to_unsigned_int(val.into_float_value(), target_llvm_ty.into_int_type(), "cast")
                    .unwrap()
                    .into()
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
            let llvm_ty = gen_ty(&*ty_id, &mut ctx.ty_ctx());
            let ret_ty = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            match op {
                mir::NullaryOp::SizeOf => {
                    let size = llvm_ty
                        .size_of()
                        .map(|s| s.const_cast(ret_ty, false))
                        .unwrap_or_else(|| ret_ty.const_zero());
                    size.into()
                }
                mir::NullaryOp::AlignOf => {
                    let size = llvm_ty
                        .size_of()
                        .map(|s| s.const_cast(ret_ty, false))
                        .unwrap_or_else(|| ret_ty.const_zero());
                    size.into()
                }
            }
        }

        mir::Rvalue::Aggregate(kind, operands) => match kind {
            mir::AggregateKind::Tuple => {
                let mut vals: Vec<BasicValueEnum<'ctx>> = Vec::with_capacity(operands.len());
                for op in operands {
                    vals.push(gen_operand(ctx, op));
                }
                let llvm_val_types: Vec<BasicTypeEnum<'ctx>> = vals.iter().map(|v| v.get_type()).collect();
                let struct_ty = ctx.llvm.struct_type(&llvm_val_types, false);
                let alloca = ctx.builder.build_alloca(struct_ty, "tuple").unwrap();
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
            mir::AggregateKind::Array(_elem_ty) => {
                let mut vals: Vec<BasicValueEnum<'ctx>> = Vec::with_capacity(operands.len());
                for op in operands {
                    vals.push(gen_operand(ctx, op));
                }
                assert!(!vals.is_empty(), "Array aggregate must have at least one element");
                let elem_ty = vals[0].get_type();
                let arr_ty = elem_ty.array_type(vals.len() as u32);
                let alloca = ctx.builder.build_alloca(arr_ty, "array").unwrap();
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
            mir::AggregateKind::Struct(_name, _field_names) => {
                let mut vals: Vec<BasicValueEnum<'ctx>> = Vec::with_capacity(operands.len());
                for op in operands {
                    vals.push(gen_operand(ctx, op));
                }
                let llvm_val_types: Vec<BasicTypeEnum<'ctx>> = vals.iter().map(|v| v.get_type()).collect();
                let struct_ty = ctx.llvm.struct_type(&llvm_val_types, false);
                let alloca = ctx.builder.build_alloca(struct_ty, "struct").unwrap();
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
            mir::AggregateKind::Enum(_name, _variant_name) => gen_literal(ctx, &mir::MirLiteral::Unit),
        },
    }
}
