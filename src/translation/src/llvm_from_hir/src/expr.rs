use core::panic;

use crate::codegen::CodeGen;
use inkwell::{
    basic_block::BasicBlock,
    values::{BasicValueEnum, PointerValue},
};
use nitrate_hir::{Store, SymbolTab, prelude as hir};
use nitrate_hir_get_type::{TypeInferenceCtx, get_type};
use nitrate_llvm::LLVMContext;

fn codegen_binexpr_add<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    let lhs = expr_codegen(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = expr_codegen(rhs, bb, ret_alloc, end_block, ctx, store, tab);
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

fn codegen_binexpr_sub<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    let lhs = expr_codegen(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = expr_codegen(rhs, bb, ret_alloc, end_block, ctx, store, tab);
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

fn codegen_binexpr_mul<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    let lhs = expr_codegen(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let rhs = expr_codegen(rhs, bb, ret_alloc, end_block, ctx, store, tab);
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

fn codegen_binexpr_div<'ctx>(
    lhs: &hir::Value,
    rhs: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret_alloc: Option<&PointerValue<'ctx>>,
    end_block: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    let llvm_lhs = expr_codegen(lhs, bb, ret_alloc, end_block, ctx, store, tab);
    let llvm_rhs = expr_codegen(rhs, bb, ret_alloc, end_block, ctx, store, tab);
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

pub(crate) fn expr_codegen<'ctx>(
    hir_value: &hir::Value,
    bb: &inkwell::builder::Builder<'ctx>,
    ret: Option<&PointerValue<'ctx>>,
    endb: Option<&BasicBlock<'ctx>>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> BasicValueEnum<'ctx> {
    match hir_value {
        hir::Value::Unit => ctx.const_struct(&[], false).into(),
        hir::Value::Bool(x) => ctx.bool_type().const_int(*x as u64, false).into(),
        hir::Value::I8(x) => ctx.i8_type().const_int(*x as u64, true).into(),
        hir::Value::I16(x) => ctx.i16_type().const_int(*x as u64, true).into(),
        hir::Value::I32(x) => ctx.i32_type().const_int(*x as u64, true).into(),
        hir::Value::I64(x) => ctx.i64_type().const_int(*x as u64, true).into(),

        hir::Value::I128(x) => {
            let x = **x;
            let low = (x & 0xFFFFFFFFFFFFFFFF) as u64;
            let high = ((x >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
            let i128 = ctx.i128_type().const_int_arbitrary_precision(&[low, high]);
            i128.into()
        }

        hir::Value::U8(x) => ctx.i8_type().const_int(*x as u64, false).into(),
        hir::Value::U16(x) => ctx.i16_type().const_int(*x as u64, false).into(),
        hir::Value::U32(x) => ctx.i32_type().const_int(*x as u64, false).into(),
        hir::Value::U64(x) => ctx.i64_type().const_int(*x as u64, false).into(),

        hir::Value::U128(x) => {
            let x = **x;
            let low = (x & 0xFFFFFFFFFFFFFFFF) as u64;
            let high = ((x >> 64) & 0xFFFFFFFFFFFFFFFF) as u64;
            let u128 = ctx.i128_type().const_int_arbitrary_precision(&[low, high]);
            u128.into()
        }

        hir::Value::F32(x) => ctx.f32_type().const_float(x.into_inner() as f64).into(),
        hir::Value::F64(x) => ctx.f64_type().const_float(x.into_inner()).into(),

        hir::Value::USize32(x) => ctx
            .ptr_sized_int_type(ctx.target_data(), None)
            .const_int(*x as u64, false)
            .into(),

        hir::Value::USize64(x) => ctx
            .ptr_sized_int_type(ctx.target_data(), None)
            .const_int(*x as u64, false)
            .into(),

        hir::Value::StringLit(x) => ctx.const_string(x.as_bytes(), false).into(),
        hir::Value::BStringLit(x) => ctx.const_string(x.as_slice(), false).into(),

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
            // TODO: implement binary operation codegen

            let lhs = &store[left].borrow();
            let rhs = &store[right].borrow();

            match op {
                hir::BinaryOp::Add => codegen_binexpr_add(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Sub => codegen_binexpr_sub(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Mul => codegen_binexpr_mul(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Div => codegen_binexpr_div(lhs, rhs, bb, ret, endb, ctx, store, tab),
                hir::BinaryOp::Mod => todo!(),
                hir::BinaryOp::And => todo!(),
                hir::BinaryOp::Or => todo!(),
                hir::BinaryOp::Xor => todo!(),
                hir::BinaryOp::Shl => todo!(),
                hir::BinaryOp::Shr => todo!(),
                hir::BinaryOp::Rol => todo!(),
                hir::BinaryOp::Ror => todo!(),
                hir::BinaryOp::LogicAnd => todo!(),
                hir::BinaryOp::LogicOr => todo!(),
                hir::BinaryOp::Lt => todo!(),
                hir::BinaryOp::Gt => todo!(),
                hir::BinaryOp::Lte => todo!(),
                hir::BinaryOp::Gte => todo!(),
                hir::BinaryOp::Eq => todo!(),
                hir::BinaryOp::Ne => todo!(),
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
