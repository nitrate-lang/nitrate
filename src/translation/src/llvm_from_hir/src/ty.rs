use inkwell::{
    AddressSpace,
    types::{BasicType, BasicTypeEnum, FunctionType, StructType},
};

use crate::{gen_symbol::get_ptr_size, gencode::CodeGen};
use nitrate_hir::prelude as hir;
use nitrate_llvm::LLVMContext;

impl<'ctx> CodeGen<'ctx> for hir::StructType {
    type Output = StructType<'ctx>;

    fn generate(
        &self,
        ctx: &'ctx LLVMContext,
        store: &hir::Store,
        tab: &hir::SymbolTab,
    ) -> Self::Output {
        let mut field_types = Vec::with_capacity(self.fields.len());
        for field in &self.fields {
            let field_type = store[&field.ty].generate(ctx, store, tab);
            field_types.push(field_type);
        }

        let packed = self.attributes.contains(&hir::StructAttribute::Packed);
        ctx.struct_type(&field_types, packed)
    }
}

impl<'ctx> CodeGen<'ctx> for hir::FunctionType {
    type Output = FunctionType<'ctx>;

    fn generate(
        &self,
        ctx: &'ctx LLVMContext,
        store: &hir::Store,
        tab: &hir::SymbolTab,
    ) -> Self::Output {
        let return_type = &store[&self.return_type];
        let return_type = return_type.generate(ctx, store, tab);
        let return_type = BasicTypeEnum::try_from(return_type).unwrap();

        let mut param_types = Vec::with_capacity(self.params.len());
        for param in &self.params {
            let param_type = store[&param.1].generate(ctx, store, tab);
            param_types.push(param_type.into());
        }

        let variadic = self.attributes.contains(&hir::FunctionAttribute::Variadic);

        return_type.fn_type(&param_types, variadic)
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Type {
    type Output = BasicTypeEnum<'ctx>;

    fn generate(
        &self,
        ctx: &'ctx LLVMContext,
        store: &hir::Store,
        tab: &hir::SymbolTab,
    ) -> Self::Output {
        match self {
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
                let llvm_element_type = store[element_type].generate(ctx, store, tab);
                llvm_element_type.array_type(*len).into()
            }

            hir::Type::Tuple { element_types } => {
                let mut llvm_element_types = Vec::with_capacity(element_types.len());
                for element_type in element_types {
                    let llvm_element_type = store[element_type].generate(ctx, store, tab);
                    llvm_element_types.push(llvm_element_type);
                }

                ctx.struct_type(&llvm_element_types, false).into()
            }

            hir::Type::Struct { struct_type } => {
                store[struct_type].generate(ctx, store, tab).into()
            }

            hir::Type::Enum { .. } => {
                let layout_ctx = hir::LayoutCtx {
                    store,
                    tab,
                    ptr_size: get_ptr_size(ctx),
                };

                let size =
                    hir::get_size_of(self, &layout_ctx).expect("Failed to get size of enum type");
                ctx.i8_type().array_type(size as u32).into()
            }

            hir::Type::Refine { base, .. } => store[base].generate(ctx, store, tab).into(),

            hir::Type::SliceRef { .. } => {
                let ptr = ctx.ptr_type(AddressSpace::default());
                let size = ctx.ptr_sized_int_type(&ctx.target_data(), None);
                ctx.struct_type(&[ptr.into(), size.into()], false).into()
            }

            hir::Type::Function { .. }
            | hir::Type::Reference { .. }
            | hir::Type::Pointer { .. } => {
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

                        enum_type.generate(ctx, store, tab)
                    }

                    hir::TypeDefinition::StructDef(id) => {
                        let struct_def = store[id].borrow();
                        let struct_type = hir::Type::Struct {
                            struct_type: struct_def.struct_id,
                        };

                        struct_type.generate(ctx, store, tab)
                    }

                    hir::TypeDefinition::TypeAliasDef(id) => {
                        let type_alias_def = store[id].borrow();
                        let aliased_type = &store[&type_alias_def.type_id];
                        aliased_type.generate(ctx, store, tab)
                    }
                }
            }

            hir::Type::InferredFloat | hir::Type::InferredInteger | hir::Type::Inferred { .. } => {
                panic!("Inferred types should have been resolved before code generation")
            }
        }
    }
}
