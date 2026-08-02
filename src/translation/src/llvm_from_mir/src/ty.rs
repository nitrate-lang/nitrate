use inkwell::AddressSpace;
use inkwell::types::{BasicType, BasicTypeEnum};
use nitrate_llvm::LLVMContext;
use nitrate_mir::prelude as mir;

/// Context for MIR type → LLVM type translation.
pub struct TypegenCtx<'ctx, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub module: &'module inkwell::module::Module<'ctx>,
}

/// Generate an LLVM return type for a function declaration.
/// Maps Unit/Never to void; all other types use gen_ty.
pub fn gen_fn_ret_ty<'ctx>(mir_type: &mir::MirType, ctx: &mut TypegenCtx<'ctx, '_>) -> Option<BasicTypeEnum<'ctx>> {
    match mir_type {
        mir::MirType::Never | mir::MirType::Unit => None, // void return
        other => Some(gen_ty(other, ctx)),
    }
}

/// Generate an LLVM type from a MIR type.
pub fn gen_ty<'ctx>(mir_type: &mir::MirType, ctx: &mut TypegenCtx<'ctx, '_>) -> BasicTypeEnum<'ctx> {
    match mir_type {
        mir::MirType::Never | mir::MirType::Unit => ctx.llvm.struct_type(&[], false).into(),
        mir::MirType::Bool => ctx.llvm.bool_type().into(),
        mir::MirType::U8 | mir::MirType::I8 => ctx.llvm.i8_type().into(),
        mir::MirType::U16 | mir::MirType::I16 => ctx.llvm.i16_type().into(),
        mir::MirType::U32 | mir::MirType::I32 => ctx.llvm.i32_type().into(),
        mir::MirType::U64 | mir::MirType::I64 => ctx.llvm.i64_type().into(),
        mir::MirType::U128 | mir::MirType::I128 => ctx.llvm.i128_type().into(),
        mir::MirType::USize => ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None).into(),
        mir::MirType::F32 => ctx.llvm.f32_type().into(),
        mir::MirType::F64 => ctx.llvm.f64_type().into(),
        mir::MirType::Array { element_type, len } => {
            let elem_ty = gen_ty(&*element_type, ctx);
            elem_ty.array_type(*len).into()
        }
        mir::MirType::Tuple { element_types } => {
            let field_types: Vec<BasicTypeEnum<'ctx>> = element_types.iter().map(|t| gen_ty(&*t, ctx)).collect();
            ctx.llvm.struct_type(&field_types, false).into()
        }
        mir::MirType::Struct { name, layout, fields } => {
            // Check if struct is already declared
            if let Some(existing) = ctx.module.get_struct_type(name) {
                return existing.into();
            }

            // Build a lookup from field name → MirTypeId
            let field_map: std::collections::HashMap<_, _> = fields.iter().map(|(n, t)| (n, t)).collect();

            // Create opaque struct first, then set body
            let struct_type = ctx.llvm.opaque_struct_type(name);
            let field_types: Vec<BasicTypeEnum<'ctx>> = layout
                .iter()
                .map(|cell| match cell {
                    mir::MirStructLayoutCell::Field { field_name } => {
                        if let Some(ty) = field_map.get(field_name) {
                            gen_ty(&*ty, ctx)
                        } else {
                            ctx.llvm.i8_type().into()
                        }
                    }
                    mir::MirStructLayoutCell::Padding(size) => ctx.llvm.i8_type().array_type(size.get()).into(),
                })
                .collect();
            struct_type.set_body(&field_types, false);
            struct_type.into()
        }
        mir::MirType::Enum { name, variants } => {
            // Enum layout: { payload: [u8; max_payload_size], tag: tag_type }
            let tag_type = match variants.len() {
                ..=256 => ctx.llvm.i8_type(),
                ..=65_536 => ctx.llvm.i16_type(),
                _ => ctx.llvm.i32_type(),
            };
            // Placeholder payload: 8 bytes (enough for most types)
            let payload_type = ctx.llvm.i64_type();
            let struct_name = format!("{}.enum", name);
            let enum_struct = ctx.llvm.opaque_struct_type(&struct_name);
            enum_struct.set_body(&[payload_type.into(), tag_type.into()], false);
            enum_struct.into()
        }
        mir::MirType::Function { .. } | mir::MirType::Reference { .. } | mir::MirType::Pointer { .. } => {
            // LLVM uses opaque pointers - all pointers are ptr type
            // Function types are also represented as pointers
            ctx.llvm.ptr_type(AddressSpace::default()).into()
        }
        mir::MirType::SliceRef { .. } | mir::MirType::SlicePtr { .. } => {
            // Fat pointer: { data_ptr, len }
            let ptr = ctx.llvm.ptr_type(AddressSpace::default());
            let size = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            ctx.llvm.struct_type(&[ptr.into(), size.into()], false).into()
        }
        mir::MirType::Range => ctx.llvm.struct_type(&[], false).into(),
        mir::MirType::Str => {
            // String is represented as { ptr, len } (same as slice)
            let ptr = ctx.llvm.ptr_type(AddressSpace::default());
            let size = ctx.llvm.ptr_sized_int_type(ctx.llvm.target_data(), None);
            ctx.llvm.struct_type(&[ptr.into(), size.into()], false).into()
        }
    }
}
