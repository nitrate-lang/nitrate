use inkwell::{
    AddressSpace,
    types::{BasicType, BasicTypeEnum, FunctionType, StructType},
};

use crate::symbol::get_ptr_size;
use nitrate_hir::prelude as hir;
use nitrate_llvm::LLVMContext;

fn gen_struct_ty<'ctx>(
    hir_struct: &hir::StructType,
    ctx: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) -> StructType<'ctx> {
    let mut field_types = Vec::with_capacity(hir_struct.fields.len());

    for hir_field in &hir_struct.fields {
        // FIXME: insert padding

        let hir_field_ty = &store[&hir_field.ty];
        field_types.push(gen_ty(hir_field_ty, ctx, store, tab));
    }

    let is_packed = hir_struct
        .attributes
        .contains(&hir::StructAttribute::Packed);

    ctx.struct_type(&field_types, is_packed)
}

pub(crate) fn gen_function_ty<'ctx>(
    hir_func_type: &hir::FunctionType,
    ctx: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) -> FunctionType<'ctx> {
    let mut param_types = Vec::with_capacity(hir_func_type.params.len());
    for hir_param in &hir_func_type.params {
        let hir_param = &store[&hir_param.1];
        param_types.push(gen_ty(hir_param, ctx, store, tab).into());
    }

    let variadic = hir_func_type
        .attributes
        .contains(&hir::FunctionAttribute::Variadic);

    let return_type = gen_ty(&store[&hir_func_type.return_type], ctx, store, tab);
    return_type.fn_type(&param_types, variadic)
}

pub(crate) fn gen_ty<'ctx>(
    hir_type: &hir::Type,
    ctx: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) -> BasicTypeEnum<'ctx> {
    match hir_type {
        hir::Type::Never | hir::Type::Unit => ctx.struct_type(&[], false).into(),
        hir::Type::Bool => ctx.bool_type().into(),
        hir::Type::U8 | hir::Type::I8 => ctx.i8_type().into(),
        hir::Type::U16 | hir::Type::I16 => ctx.i16_type().into(),
        hir::Type::U32 | hir::Type::I32 => ctx.i32_type().into(),
        hir::Type::U64 | hir::Type::I64 => ctx.i64_type().into(),
        hir::Type::U128 | hir::Type::I128 => ctx.i128_type().into(),
        hir::Type::USize => ctx.ptr_sized_int_type(&ctx.target_data(), None).into(),
        hir::Type::F32 => ctx.f32_type().into(),
        hir::Type::F64 => ctx.f64_type().into(),

        hir::Type::Array { element_type, len } => {
            let element_type = &store[element_type];
            let llvm_element_type = gen_ty(element_type, ctx, store, tab);
            llvm_element_type.array_type(*len).into()
        }

        hir::Type::Tuple { element_types } => {
            let mut llvm_element_types = Vec::with_capacity(element_types.len());
            for element_type in element_types {
                // FIXME: insert padding

                let hir_element_type = &store[element_type];
                llvm_element_types.push(gen_ty(hir_element_type, ctx, store, tab));
            }

            ctx.struct_type(&llvm_element_types, false).into()
        }

        hir::Type::Struct { struct_type } => {
            let hir_struct = &store[struct_type];
            gen_struct_ty(hir_struct, ctx, store, tab).into()
        }

        hir::Type::Enum { enum_type } => {
            let hir_enum = &store[enum_type];

            let layout_ctx = hir::LayoutCtx {
                ptr_size: get_ptr_size(ctx),
                store,
                tab,
            };

            let payload_size = hir::get_size_of(hir_type, &layout_ctx).expect("enum size error");
            let payload_type = ctx.i8_type().array_type(payload_size as u32);

            let tag_type = match hir_enum.variants.len() {
                ..=256 => ctx.i8_type(),
                ..=65_536 => ctx.i16_type(),
                ..=4_294_967_296 => ctx.i32_type(),
                _ => ctx.i64_type(),
            };

            // FIXME: insert padding

            ctx.struct_type(&[payload_type.into(), tag_type.into()], false)
                .into()
        }

        hir::Type::Refine { base, .. } => {
            let hir_base = &store[base];
            gen_ty(hir_base, ctx, store, tab)
        }

        hir::Type::SliceRef { .. } => {
            let ptr = ctx.ptr_type(AddressSpace::default());
            let size = ctx.ptr_sized_int_type(&ctx.target_data(), None);
            ctx.struct_type(&[ptr.into(), size.into()], false).into()
        }

        hir::Type::Function { .. } | hir::Type::Reference { .. } | hir::Type::Pointer { .. } => {
            /* LLVM doesn't distinguish between pointer types anymore */
            ctx.ptr_type(AddressSpace::default()).into()
        }

        hir::Type::Symbol { path } => {
            match tab
                .get_type(path)
                .expect("Unknown type name encountered during code generation")
            {
                hir::TypeDefinition::EnumDef(id) => {
                    let enum_def = store[id].borrow();
                    let enum_type = hir::Type::Enum {
                        enum_type: enum_def.enum_id,
                    };

                    gen_ty(&enum_type, ctx, store, tab)
                }

                hir::TypeDefinition::StructDef(id) => {
                    let struct_def = store[id].borrow();
                    let struct_type = hir::Type::Struct {
                        struct_type: struct_def.struct_id,
                    };

                    gen_ty(&struct_type, ctx, store, tab)
                }

                hir::TypeDefinition::TypeAliasDef(id) => {
                    let type_alias_def = store[id].borrow();
                    let aliased_type = &store[&type_alias_def.type_id];
                    gen_ty(aliased_type, ctx, store, tab)
                }
            }
        }

        hir::Type::InferredFloat | hir::Type::InferredInteger | hir::Type::Inferred { .. } => {
            panic!("Inferred types should have been resolved before code generation")
        }
    }
}
