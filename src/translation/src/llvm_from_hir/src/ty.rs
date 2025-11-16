use inkwell::{
    AddressSpace,
    types::{BasicType, BasicTypeEnum, FunctionType, StructType},
};
use nitrate_llvm::LLVMContext;

pub struct TypegenCtx<'ctx, 'store, 'tab> {
    pub llvm: &'ctx LLVMContext,
    pub store: &'store hir::Store,
    pub tab: &'tab hir::SymbolTab,
}

use crate::symbol::get_ptr_size;
use nitrate_hir::prelude as hir;

fn gen_struct_ty<'ctx>(
    hir_struct: &hir::StructType,
    ctx: &mut TypegenCtx<'ctx, '_, '_>,
) -> StructType<'ctx> {
    let mut field_types = Vec::with_capacity(hir_struct.fields.len());

    for hir_field in &hir_struct.fields {
        // FIXME: insert padding

        let hir_field_ty = &ctx.store[&hir_field.ty];
        field_types.push(gen_ty(hir_field_ty, ctx));
    }

    let is_packed = hir_struct
        .attributes
        .contains(&hir::StructAttribute::Packed);

    ctx.llvm.struct_type(&field_types, is_packed)
}

pub(crate) fn gen_function_ty<'ctx>(
    hir_func_type: &hir::FunctionType,
    ctx: &mut TypegenCtx<'ctx, '_, '_>,
) -> FunctionType<'ctx> {
    let mut param_types = Vec::with_capacity(hir_func_type.params.len());
    for hir_param in &hir_func_type.params {
        let hir_param = &ctx.store[&hir_param.1];
        param_types.push(gen_ty(hir_param, ctx).into());
    }

    let variadic = hir_func_type
        .attributes
        .contains(&hir::FunctionAttribute::CVariadic);

    let return_type = gen_ty(&ctx.store[&hir_func_type.return_type], ctx);
    return_type.fn_type(&param_types, variadic)
}

pub(crate) fn gen_ty<'ctx>(
    hir_type: &hir::Type,
    ctx: &mut TypegenCtx<'ctx, '_, '_>,
) -> BasicTypeEnum<'ctx> {
    match hir_type {
        hir::Type::Never | hir::Type::Unit => ctx.llvm.struct_type(&[], false).into(),
        hir::Type::Bool => ctx.llvm.bool_type().into(),
        hir::Type::U8 | hir::Type::I8 => ctx.llvm.i8_type().into(),
        hir::Type::U16 | hir::Type::I16 => ctx.llvm.i16_type().into(),
        hir::Type::U32 | hir::Type::I32 => ctx.llvm.i32_type().into(),
        hir::Type::U64 | hir::Type::I64 => ctx.llvm.i64_type().into(),
        hir::Type::U128 | hir::Type::I128 => ctx.llvm.i128_type().into(),
        hir::Type::USize => ctx
            .llvm
            .ptr_sized_int_type(&ctx.llvm.target_data(), None)
            .into(),
        hir::Type::F32 => ctx.llvm.f32_type().into(),
        hir::Type::F64 => ctx.llvm.f64_type().into(),
        hir::Type::Array { element_type, len } => {
            let element_type = &ctx.store[element_type];
            let llvm_element_type = gen_ty(element_type, ctx);
            llvm_element_type.array_type(*len).into()
        }

        hir::Type::Tuple { element_types } => {
            let mut llvm_element_types = Vec::with_capacity(element_types.len());
            for element_type in element_types {
                // FIXME: insert padding

                let hir_element_type = &ctx.store[element_type];
                llvm_element_types.push(gen_ty(hir_element_type, ctx));
            }

            ctx.llvm.struct_type(&llvm_element_types, false).into()
        }

        hir::Type::Struct { struct_type } => {
            let hir_struct = &ctx.store[struct_type];
            gen_struct_ty(hir_struct, ctx).into()
        }

        hir::Type::Enum { enum_type } => {
            let hir_enum = &ctx.store[enum_type];
            let layout_ctx = hir::LayoutCtx {
                ptr_size: get_ptr_size(ctx.llvm),
                store: ctx.store,
                tab: ctx.tab,
            };

            let payload_size = hir::get_size_of(hir_type, &layout_ctx).expect("enum size error");
            let payload_type = ctx.llvm.i8_type().array_type(payload_size as u32);
            let tag_type = match hir_enum.variants.len() {
                ..=256 => ctx.llvm.i8_type(),
                ..=65_536 => ctx.llvm.i16_type(),
                ..=4_294_967_296 => ctx.llvm.i32_type(),
                _ => ctx.llvm.i64_type(),
            };

            // FIXME: insert padding

            ctx.llvm
                .struct_type(&[payload_type.into(), tag_type.into()], false)
                .into()
        }

        hir::Type::Refine { base, .. } => {
            let hir_base = &ctx.store[base];
            gen_ty(hir_base, ctx)
        }

        hir::Type::SliceRef { .. } => {
            let ptr = ctx.llvm.ptr_type(AddressSpace::default());
            let size = ctx.llvm.ptr_sized_int_type(&ctx.llvm.target_data(), None);
            ctx.llvm
                .struct_type(&[ptr.into(), size.into()], false)
                .into()
        }

        hir::Type::Function { .. } | hir::Type::Reference { .. } | hir::Type::Pointer { .. } => {
            /* LLVM doesn't distinguish between pointer types anymore */
            ctx.llvm.ptr_type(AddressSpace::default()).into()
        }

        hir::Type::InferredFloat | hir::Type::InferredInteger | hir::Type::Inferred { .. } => {
            panic!("Inferred types should have been resolved before code generation")
        }
    }
}
