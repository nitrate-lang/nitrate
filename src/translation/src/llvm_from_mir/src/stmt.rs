use core::panic;

use crate::context::CodegenCtx;
use crate::operand::gen_operand;
use crate::place::gen_place;
use crate::rvalue::gen_rvalue;
use inkwell::types::BasicType;
use inkwell::values::BasicValueEnum;
use nitrate_mir::prelude as mir;

/// Generate LLVM IR for a MIR statement.
pub fn gen_statement<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, stmt: &mir::Statement) {
    match stmt {
        mir::Statement::Assign(place, rvalue) => {
            let val = gen_rvalue(ctx, rvalue);
            let ptr = gen_place(ctx, place);
            ctx.builder.build_store(ptr, val).unwrap();
        }
        mir::Statement::SetDiscriminant {
            place: _,
            variant_index: _,
        } => {
            // Set the discriminant field of an enum.
            // For now, a no-op placeholder.
        }
        mir::Statement::StorageLive(_local_id) => {
            // Mark storage as live. In LLVM, this is a no-op (alloca already allocates).
        }
        mir::Statement::StorageDead(_local_id) => {
            // Mark storage as dead. In LLVM without optimizations, this is a no-op.
        }
    }
}

/// Generate LLVM IR for a MIR terminator.
pub fn gen_terminator<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, terminator: &mir::Terminator) {
    match terminator {
        mir::Terminator::Goto { target } => {
            let target_bb = ctx.get_llvm_block(target);
            ctx.builder.build_unconditional_branch(target_bb).unwrap();
        }
        mir::Terminator::If {
            condition,
            true_target,
            false_target,
        } => {
            let cond_val = gen_operand(ctx, condition);
            let true_bb = ctx.get_llvm_block(true_target);
            let false_bb = ctx.get_llvm_block(false_target);
            ctx.builder
                .build_conditional_branch(cond_val.into_int_value(), true_bb, false_bb)
                .unwrap();
        }
        mir::Terminator::SwitchInt {
            discr,
            targets,
            otherwise,
        } => {
            let discr_val = gen_operand(ctx, discr);
            let discr_int = discr_val.into_int_value();
            let otherwise_bb = ctx.get_llvm_block(otherwise);

            // Build case list
            let cases: Vec<(inkwell::values::IntValue<'ctx>, inkwell::basic_block::BasicBlock<'ctx>)> = targets
                .iter()
                .map(|(val, target)| {
                    let target_bb = ctx.get_llvm_block(target);
                    let int_type = ctx.llvm.custom_width_int_type(discr_int.get_type().get_bit_width());
                    // For values > u64::MAX, use arbitrary precision.
                    let const_val = if *val > u64::MAX as u128 {
                        let low = (*val & 0xFFFFFFFFFFFFFFFF) as u64;
                        let high = ((*val >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
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
        mir::Terminator::Unwind { target: _ } => {
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::Unreachable => {
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::Call { callee, args } => {
            // Diverging call: call function then unreachable
            let callee_val = gen_operand(ctx, callee);
            let llvm_args: Vec<BasicValueEnum<'ctx>> = args.iter().map(|a| gen_operand(ctx, a)).collect();

            if callee_val.is_pointer_value() {
                let callee_ptr = callee_val.into_pointer_value();
                let void_ty = ctx.llvm.void_type();
                let arg_types: Vec<inkwell::types::BasicMetadataTypeEnum<'ctx>> =
                    llvm_args.iter().map(|v| v.get_type().into()).collect();
                let fn_ty = void_ty.fn_type(&arg_types, false);
                let arg_values: Vec<inkwell::values::BasicMetadataValueEnum<'ctx>> =
                    llvm_args.iter().map(|v| (*v).into()).collect();
                ctx.builder
                    .build_indirect_call(fn_ty, callee_ptr, &arg_values, "")
                    .unwrap();
            } else {
                panic!("Call target must be a function pointer");
            }
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::CallReturn {
            callee,
            args,
            destination,
            target,
        } => {
            let callee_val = gen_operand(ctx, callee);
            let llvm_args: Vec<BasicValueEnum<'ctx>> = args.iter().map(|a| gen_operand(ctx, a)).collect();

            let call_result = if callee_val.is_pointer_value() {
                let callee_ptr = callee_val.into_pointer_value();
                let arg_types: Vec<inkwell::types::BasicMetadataTypeEnum<'ctx>> =
                    llvm_args.iter().map(|v| v.get_type().into()).collect();
                let dest_ty = crate::place::get_place_type_for_load(ctx, destination);
                let llvm_dest_ty = crate::ty::gen_ty(&dest_ty, &mut ctx.ty_ctx());
                let fn_ty = llvm_dest_ty.fn_type(&arg_types, false);
                let arg_values: Vec<inkwell::values::BasicMetadataValueEnum<'ctx>> =
                    llvm_args.iter().map(|v| (*v).into()).collect();
                ctx.builder
                    .build_indirect_call(fn_ty, callee_ptr, &arg_values, "call")
                    .unwrap()
            } else {
                panic!("Call target must be a function pointer");
            };

            // Store result to destination
            let dest_ptr = gen_place(ctx, destination);
            if call_result.try_as_basic_value().is_left() {
                let result_val = call_result.try_as_basic_value().left().unwrap();
                ctx.builder.build_store(dest_ptr, result_val).unwrap();
            }

            let target_bb = ctx.get_llvm_block(target);
            ctx.builder.build_unconditional_branch(target_bb).unwrap();
        }
        mir::Terminator::Resume => {
            ctx.builder.build_unreachable().unwrap();
        }
        mir::Terminator::Abort => {
            ctx.builder.build_unreachable().unwrap();
        }
    }
}
