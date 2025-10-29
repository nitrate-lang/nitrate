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
                // TODO: implement
                unimplemented!()
            }

            hir::Value::BStringLit(thin_vec) => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::InferredInteger(_) => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::InferredFloat(ordered_float) => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::StructObject {
                struct_path,
                fields,
            } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::EnumVariant {
                enum_path,
                variant,
                value,
            } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Binary { left, op, right } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Unary { op, operand } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::FieldAccess { expr, field } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::IndexAccess { collection, index } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Assign { place, value } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Deref { place } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Cast { expr, to } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Borrow {
                exclusive,
                mutable,
                place,
            } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::List { elements } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Tuple { elements } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::While { condition, body } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Loop { body } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Break { label } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Continue { label } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Return { value } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Block { block } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Closure { captures, callee } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Call {
                callee,
                positional,
                named,
            } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::MethodCall {
                object,
                method_name,
                positional,
                named,
            } => {
                // TODO: implement
                unimplemented!()
            }

            hir::Value::Symbol { path } => {
                // TODO: implement
                unimplemented!()
            }
        }
    }
}
