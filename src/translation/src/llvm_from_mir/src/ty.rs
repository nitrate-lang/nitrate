use inkwell::AddressSpace;
use inkwell::types::{BasicType, BasicTypeEnum};
use nitrate_llvm::LLVMContext;
use nitrate_mir::prelude as mir;
use std::collections::HashMap;

/// Context for MIR type → LLVM type translation.
pub struct TypegenCtx<'ctx, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub module: &'module inkwell::module::Module<'ctx>,
}

impl<'ctx, 'module> TypegenCtx<'ctx, 'module> {
    /// Size in bytes of an LLVM type, or `None` for unsized/function types.
    pub fn size_of(&self, ty: BasicTypeEnum<'ctx>) -> Option<u64> {
        ty.size_of().and_then(|v| v.get_zero_extended_constant())
    }

    /// ABI alignment in bytes of an LLVM type.
    pub fn abi_align(&self, ty: BasicTypeEnum<'ctx>) -> u32 {
        match ty {
            BasicTypeEnum::ArrayType(t) => self.llvm.target_data().get_abi_alignment(&t),
            BasicTypeEnum::FloatType(t) => self.llvm.target_data().get_abi_alignment(&t),
            BasicTypeEnum::IntType(t) => self.llvm.target_data().get_abi_alignment(&t),
            BasicTypeEnum::PointerType(t) => self.llvm.target_data().get_abi_alignment(&t),
            BasicTypeEnum::StructType(t) => self.llvm.target_data().get_abi_alignment(&t),
            BasicTypeEnum::VectorType(t) => self.llvm.target_data().get_abi_alignment(&t),
            BasicTypeEnum::ScalableVectorType(t) => self.llvm.target_data().get_abi_alignment(&t),
        }
    }
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
            // A named struct is cached in the LLVM context by its name, so a
            // repeated `gen_ty` of the same struct returns the identical type.
            if let Some(existing) = ctx.module.get_struct_type(name) {
                return existing.into();
            }

            // Build a lookup from field name → MirTypeId.
            let field_map: HashMap<_, _> = fields.iter().map(|(n, t)| (n, t)).collect();

            // Create an opaque struct first, then set its body. Padding cells
            // are emitted as `[size x i8]` arrays.
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
            // A named enum struct is cached by its `.enum` name.
            let struct_name = format!("{}.enum", name);
            if let Some(existing) = ctx.module.get_struct_type(&struct_name) {
                return existing.into();
            }

            // Compute the payload storage: the maximum payload size (rounded up
            // to the maximum payload alignment) across all variants, as a byte
            // array. Followed by the discriminant tag whose width depends on the
            // number of variants.
            let tag_type = match variants.len() {
                ..=256 => ctx.llvm.i8_type(),
                ..=65_536 => ctx.llvm.i16_type(),
                _ => ctx.llvm.i32_type(),
            };

            let mut max_size: u64 = 0;
            let mut max_align: u64 = 1;
            for variant in variants.iter() {
                if let Some(payload_ty) = &variant.payload {
                    let llvm_payload = gen_ty(&*payload_ty, ctx);
                    if let Some(size) = ctx.size_of(llvm_payload) {
                        max_size = max_size.max(size);
                    }
                    let align = ctx.abi_align(llvm_payload) as u64;
                    max_align = max_align.max(align);
                }
            }
            let payload_size = max_size.div_ceil(max_align) * max_align;

            let enum_struct = ctx.llvm.opaque_struct_type(&struct_name);
            enum_struct.set_body(
                &[
                    ctx.llvm.i8_type().array_type(payload_size as u32).into(),
                    tag_type.into(),
                ],
                false,
            );
            enum_struct.into()
        }
        mir::MirType::Function { .. } | mir::MirType::Reference { .. } | mir::MirType::Pointer { .. } => {
            // LLVM uses opaque pointers — all pointers are `ptr`.
            // Function values are also represented as pointers.
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
            // String is represented as a pointer to null-terminated byte data.
            ctx.llvm.ptr_type(AddressSpace::default()).into()
        }
    }
}
