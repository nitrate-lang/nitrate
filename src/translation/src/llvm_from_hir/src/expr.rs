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

            hir::Value::StringLit(thin_str) => todo!(),
            hir::Value::BStringLit(thin_vec) => todo!(),
            hir::Value::InferredInteger(_) => todo!(),
            hir::Value::InferredFloat(ordered_float) => todo!(),
            hir::Value::StructObject {
                struct_path,
                fields,
            } => todo!(),
            hir::Value::EnumVariant {
                enum_path,
                variant,
                value,
            } => todo!(),
            hir::Value::Binary { left, op, right } => todo!(),
            hir::Value::Unary { op, operand } => todo!(),
            hir::Value::FieldAccess { expr, field } => todo!(),
            hir::Value::IndexAccess { collection, index } => todo!(),
            hir::Value::Assign { place, value } => todo!(),
            hir::Value::Deref { place } => todo!(),
            hir::Value::Cast { expr, to } => todo!(),
            hir::Value::Borrow {
                exclusive,
                mutable,
                place,
            } => todo!(),
            hir::Value::List { elements } => todo!(),
            hir::Value::Tuple { elements } => todo!(),
            hir::Value::If {
                condition,
                true_branch,
                false_branch,
            } => todo!(),
            hir::Value::While { condition, body } => todo!(),
            hir::Value::Loop { body } => todo!(),
            hir::Value::Break { label } => todo!(),
            hir::Value::Continue { label } => todo!(),
            hir::Value::Return { value } => todo!(),
            hir::Value::Block { block } => todo!(),
            hir::Value::Closure { captures, callee } => todo!(),
            hir::Value::Call {
                callee,
                positional,
                named,
            } => todo!(),
            hir::Value::MethodCall {
                object,
                method_name,
                positional,
                named,
            } => todo!(),
            hir::Value::Symbol { path } => todo!(),
        }
    }
}
