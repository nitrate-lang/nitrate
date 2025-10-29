use core::panic;

use crate::codegen::CodeGen;
use inkwell::values::BasicValueEnum;
use nitrate_hir::{Store, SymbolTab, prelude as hir};
use nitrate_llvm::LLVMContext;

impl<'ctx> CodeGen<'ctx> for hir::Value {
    type Output = BasicValueEnum<'ctx>;

    fn generate(&self, ctx: &'ctx LLVMContext, _store: &Store, _tab: &SymbolTab) -> Self::Output {
        match self {
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

            hir::Value::StringLit(thin_str) => {
                // TODO: implement string literal codegen
                unimplemented!()
            }

            hir::Value::BStringLit(thin_vec) => {
                // TODO: implement binary string literal codegen
                unimplemented!()
            }

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
                unimplemented!()
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
}
