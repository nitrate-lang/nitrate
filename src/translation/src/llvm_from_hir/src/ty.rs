use core::panic;
use inkwell::{
    AddressSpace,
    types::{BasicType, BasicTypeEnum, FunctionType, StructType},
};
use nitrate_llvm::LLVMContext;
use std::ops::Deref;

pub struct TypegenCtx<'ctx, 'tab, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub tab: &'tab hir::SymbolTab,
    pub module: &'module inkwell::module::Module<'ctx>,
}

use crate::symbol::get_ptr_size;
use nitrate_hir::{StructMemoryLayoutCell, prelude as hir};

fn gen_struct_ty<'ctx>(hir_struct_def: &hir::StructDef, ctx: &mut TypegenCtx<'ctx, '_, '_>) -> StructType<'ctx> {
    if let Some(struct_type) = ctx.module.get_struct_type(&hir_struct_def.name) {
        return struct_type;
    }

    let mut field_types = Vec::with_capacity(hir_struct_def.fields.len());
    for cell in &hir_struct_def.layout {
        match cell {
            StructMemoryLayoutCell::Field { field_name } => {
                let hir_field = hir_struct_def
                    .fields
                    .get(field_name)
                    .expect("expected field to exist in struct");
                let hir_field_ty = hir_field.ty.deref();
                field_types.push(gen_ty(hir_field_ty, ctx));
            }

            StructMemoryLayoutCell::Padding(size) => {
                let padding_type = ctx.llvm.i8_type().array_type((*size).get() as u32);
                field_types.push(padding_type.into());
            }
        }
    }

    let is_packed = hir_struct_def.attributes.contains(&hir::StructAttribute::Packed);

    let struct_type = ctx.llvm.opaque_struct_type(&hir_struct_def.name);
    struct_type.set_body(&field_types, is_packed);
    struct_type
}

pub(crate) fn gen_function_ty<'ctx>(
    hir_func_type: &hir::FunctionType,
    ctx: &mut TypegenCtx<'ctx, '_, '_>,
) -> FunctionType<'ctx> {
    let mut param_types = Vec::with_capacity(hir_func_type.params.len());
    for hir_param in &hir_func_type.params {
        let hir_param = hir_param.1.deref();
        param_types.push(gen_ty(hir_param, ctx).into());
    }

    let variadic = hir_func_type.attributes.contains(&hir::FunctionAttribute::CVariadic);

    let return_type = gen_ty(&hir_func_type.return_type, ctx);
    return_type.fn_type(&param_types, variadic)
}

pub(crate) fn gen_ty<'ctx>(hir_type: &hir::Type, ctx: &mut TypegenCtx<'ctx, '_, '_>) -> BasicTypeEnum<'ctx> {
    match hir_type {
        hir::Type::Never { .. } | hir::Type::Unit { .. } => ctx.llvm.struct_type(&[], false).into(),
        hir::Type::Bool { .. } => ctx.llvm.bool_type().into(),
        hir::Type::U8 { .. } | hir::Type::I8 { .. } => ctx.llvm.i8_type().into(),
        hir::Type::U16 { .. } | hir::Type::I16 { .. } => ctx.llvm.i16_type().into(),
        hir::Type::U32 { .. } | hir::Type::I32 { .. } => ctx.llvm.i32_type().into(),
        hir::Type::U64 { .. } | hir::Type::I64 { .. } => ctx.llvm.i64_type().into(),
        hir::Type::U128 { .. } | hir::Type::I128 { .. } => ctx.llvm.i128_type().into(),
        hir::Type::USize { .. } => ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None).into(),
        hir::Type::F32 { .. } => ctx.llvm.f32_type().into(),
        hir::Type::F64 { .. } => ctx.llvm.f64_type().into(),
        hir::Type::Array { element_type, len, .. } => {
            let llvm_element_type = gen_ty(element_type, ctx);
            llvm_element_type.array_type(*len).into()
        }

        hir::Type::Tuple { element_types, .. } => {
            let mut llvm_element_types = Vec::with_capacity(element_types.len());
            for element_type in element_types {
                // FIXME: insert padding
                llvm_element_types.push(gen_ty(element_type, ctx));
            }

            ctx.llvm.struct_type(&llvm_element_types, false).into()
        }

        hir::Type::Struct { def, .. } => gen_struct_ty(&def.borrow(), ctx).into(),

        hir::Type::Enum { def, .. } => {
            let layout_ctx = hir::LayoutCtx {
                ptr_size: get_ptr_size(ctx.llvm),
                tab: ctx.tab,
            };

            let payload_size = hir::get_size_of(hir_type, &layout_ctx).expect("enum size error");
            let payload_type = ctx.llvm.i8_type().array_type(payload_size as u32);
            let tag_type = match def.borrow().variants.len() {
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

        hir::Type::TypeAlias { def, .. } => gen_ty(&def.borrow().type_id, ctx),

        hir::Type::Refine { base, .. } => gen_ty(base, ctx),

        hir::Type::SliceRef { .. } => {
            let ptr = ctx.llvm.ptr_type(AddressSpace::default());
            let size = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            ctx.llvm.struct_type(&[ptr.into(), size.into()], false).into()
        }

        hir::Type::SlicePtr { .. } => {
            let ptr = ctx.llvm.ptr_type(AddressSpace::default());
            let size = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            ctx.llvm.struct_type(&[ptr.into(), size.into()], false).into()
        }

        hir::Type::Function { .. } | hir::Type::Reference { .. } | hir::Type::Pointer { .. } => {
            /* LLVM doesn't distinguish between pointer types anymore */
            ctx.llvm.ptr_type(AddressSpace::default()).into()
        }

        hir::Type::TraitObject { .. } => {
            // Trait objects are opaque pointers for now
            ctx.llvm.ptr_type(AddressSpace::default()).into()
        }

        hir::Type::Parameterized { .. } => {
            panic!(
                "Cannot generate LLVM type for uninstantiated generic type: {:?}",
                hir_type
            )
        }

        hir::Type::InferredFloat { .. } | hir::Type::InferredInteger { .. } | hir::Type::Inferred { .. } => {
            panic!("Inferred types should have been resolved before code generation")
        }

        hir::Type::GenericParam { .. } => {
            panic!("Generic parameters should have been monomorphized before code generation")
        }
        hir::Type::Range { .. } => ctx.llvm.struct_type(&[], false).into(),
    }
}
