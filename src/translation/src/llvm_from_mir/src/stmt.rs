use core::panic;

use inkwell::types::BasicType;
use inkwell::values::BasicValueEnum;
use nitrate_mir::prelude as mir;

use crate::context::CodegenCtx;
use crate::operand::gen_operand;
use crate::place::{gen_place, get_place_type_for_load};
use crate::rvalue::gen_rvalue;
use crate::ty::gen_ty;

/// Generate LLVM IR for a MIR statement.
pub fn gen_statement<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, stmt: &mir::Statement) {
    match stmt {
        mir::Statement::Assign(place, rvalue) => {
            let val = gen_rvalue(ctx, rvalue);
            let ptr = gen_place(ctx, place);
            ctx.builder.build_store(ptr, val).unwrap();
        }
        mir::Statement::SetDiscriminant { place, variant_index } => {
            // Store the variant index into the enum's discriminant tag field.
            // The enum layout is `{ [N x i8] payload, tag }`, so the tag is
            // field index 1.
            let place_ty = get_place_type_for_load(ctx, place);
            if let mir::MirType::Enum { variants, .. } = &*place_ty {
                let enum_llvm_ty = gen_ty(&place_ty, &mut ctx.ty_ctx());
                let enum_ptr = gen_place(ctx, place);
                let tag_type = match variants.len() {
                    ..=256 => ctx.llvm.i8_type(),
                    ..=65_536 => ctx.llvm.i16_type(),
                    _ => ctx.llvm.i32_type(),
                };
                let tag_ptr = unsafe {
                    ctx.builder
                        .build_in_bounds_gep(
                            enum_llvm_ty,
                            enum_ptr,
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
            } else {
                panic!("SetDiscriminant on non-enum type {:?}", &*place_ty);
            }
        }
        mir::Statement::StorageLive(_local_id) => {
            // In LLVM, the alloca already allocates storage; no-op.
        }
        mir::Statement::StorageDead(_local_id) => {
            // Without optimizations, this is a no-op.
        }
    }
}

/// Add the block arguments from a predecessor's terminator to the target's phi
/// nodes. Block arguments on terminators carry the actual values bound to a
/// successor block's formal parameters.
fn add_phi_incoming_from_operands<'ctx>(
    ctx: &mut CodegenCtx<'ctx, '_>,
    target: &mir::BasicBlockId,
    block_args: &[mir::Operand],
) {
    if block_args.is_empty() {
        return;
    }

    let curr_bb = ctx.curr_block.expect("no current block for phi incoming");

    // Evaluate all operands first (before mutably borrowing block_phi_nodes).
    let arg_vals: Vec<BasicValueEnum<'ctx>> = block_args.iter().map(|arg| gen_operand(ctx, arg)).collect();

    if let Some(phi_list) = ctx.block_phi_nodes.get(&target.as_usize()) {
        for (i, phi_node) in phi_list.iter().enumerate() {
            if let Some(arg_val) = arg_vals.get(i) {
                phi_node.borrow_mut().add_incoming(&[(arg_val, curr_bb)]);
            }
        }
    }
}

/// Generate LLVM IR for a MIR terminator.
pub fn gen_terminator<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, terminator: &mir::Terminator) {
    match terminator {
        mir::Terminator::Goto { target, args } => {
            let target_bb = ctx.get_llvm_block(target);
            add_phi_incoming_from_operands(ctx, target, args);
            ctx.builder.build_unconditional_branch(target_bb).unwrap();
        }
        mir::Terminator::If {
            condition,
            true_target,
            true_args,
            false_target,
            false_args,
        } => {
            let cond_val = gen_operand(ctx, condition);
            let true_bb = ctx.get_llvm_block(true_target);
            let false_bb = ctx.get_llvm_block(false_target);

            add_phi_incoming_from_operands(ctx, true_target, true_args);
            add_phi_incoming_from_operands(ctx, false_target, false_args);

            ctx.builder
                .build_conditional_branch(cond_val.into_int_value(), true_bb, false_bb)
                .unwrap();
        }
        mir::Terminator::SwitchInt {
            discr,
            targets,
            otherwise,
            otherwise_args,
        } => {
            let discr_val = gen_operand(ctx, discr);
            let discr_int = discr_val.into_int_value();
            let otherwise_bb = ctx.get_llvm_block(otherwise);

            add_phi_incoming_from_operands(ctx, otherwise, otherwise_args);

            let cases: Vec<(inkwell::values::IntValue<'ctx>, inkwell::basic_block::BasicBlock<'ctx>)> = targets
                .iter()
                .map(|(val, target, target_args)| {
                    let target_bb = ctx.get_llvm_block(target);
                    let int_type = ctx.llvm.custom_width_int_type(discr_int.get_type().get_bit_width());
                    add_phi_incoming_from_operands(ctx, target, target_args);
                    let const_val = if *val > u64::MAX as u128 {
                        let low = (*val & 0xFFFF_FFFF_FFFF_FFFF) as u64;
                        let high = ((*val >> 64) & 0xFFFF_FFFF_FFFF_FFFF) as u64;
                        int_type.const_int_arbitrary_precision(&[low, high])
                    } else {
                        int_type.const_int(*val as u64, false)
                    };
                    (const_val, target_bb)
                })
                .collect();

            ctx.builder.build_switch(discr_int, otherwise_bb, &cases).unwrap();
        }
        mir::Terminator::Return { value } => match value {
            Some(op) => {
                let val = gen_operand(ctx, op);
                ctx.builder.build_return(Some(&val)).unwrap();
            }
            None => {
                ctx.builder.build_return(None).unwrap();
            }
        },
        mir::Terminator::Unreachable => {
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::Call {
            callee,
            args,
            destination,
            target,
            target_args,
        } => {
            let llvm_args: Vec<BasicValueEnum<'ctx>> = args.iter().map(|a| gen_operand(ctx, a)).collect();
            let arg_types: Vec<inkwell::types::BasicMetadataTypeEnum<'ctx>> =
                llvm_args.iter().map(|v| v.get_type().into()).collect();

            let call_result: Option<BasicValueEnum<'ctx>> = if let mir::Operand::Copy(mir::Place::Static(callee_name))
            | mir::Operand::Move(mir::Place::Static(callee_name)) =
                callee
            {
                // Direct call to a known function by name.
                let llvm_fn = ctx.module.get_function(callee_name).unwrap_or_else(|| {
                    panic!(
                        "function '{}' not found in LLVM module. \
                         Ensure all functions are lowered to MIR and declared before codegen.",
                        callee_name
                    )
                });
                let arg_values: Vec<inkwell::values::BasicMetadataValueEnum<'ctx>> =
                    llvm_args.iter().map(|v| (*v).into()).collect();
                let call = ctx.builder.build_direct_call(llvm_fn, &arg_values, "call").unwrap();
                call.try_as_basic_value().left()
            } else {
                // Indirect call via function pointer.
                let callee_val = gen_operand(ctx, callee);
                if !callee_val.is_pointer_value() {
                    panic!("Call target must be a function pointer");
                }
                let callee_ptr = callee_val.into_pointer_value();
                let fn_ty = if let Some(dest) = destination {
                    let dest_ty = get_place_type_for_load(ctx, dest);
                    let llvm_dest_ty = gen_ty(&dest_ty, &mut ctx.ty_ctx());
                    llvm_dest_ty.fn_type(&arg_types, false)
                } else {
                    ctx.llvm.void_type().fn_type(&arg_types, false)
                };
                let arg_values: Vec<inkwell::values::BasicMetadataValueEnum<'ctx>> =
                    llvm_args.iter().map(|v| (*v).into()).collect();
                let call = ctx
                    .builder
                    .build_indirect_call(fn_ty, callee_ptr, &arg_values, "call")
                    .unwrap();
                call.try_as_basic_value().left()
            };

            // Store the result to the destination if returning.
            if let Some(dest) = destination {
                let dest_ptr = gen_place(ctx, dest);
                if let Some(result_val) = call_result {
                    ctx.builder.build_store(dest_ptr, result_val).unwrap();
                }
            }

            // Add phi incoming values and branch if returning; otherwise the
            // call diverges and is followed by unreachable.
            if let Some(t) = target {
                add_phi_incoming_from_operands(ctx, t, target_args);
                let target_bb = ctx.get_llvm_block(t);
                ctx.builder.build_unconditional_branch(target_bb).unwrap();
            } else {
                ctx.builder.build_unreachable().unwrap();
            }
        }
    }
}
