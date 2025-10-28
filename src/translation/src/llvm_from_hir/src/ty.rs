use crate::codegen::CodeGen;
use inkwell::{
    AddressSpace,
    types::{ArrayType, BasicType, BasicTypeEnum, FunctionType, PointerType, StructType},
};
use nitrate_hir::{Store, SymbolTab, hir::TypeDefinition, prelude as hir};
use nitrate_llvm::LLVMContext;

impl<'ctx> CodeGen<'ctx> for hir::StructType {
    type Output = StructType<'ctx>;

    fn generate(
        &self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::EnumType {
    type Output = ArrayType<'ctx>;

    fn generate(
        &self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::FunctionType {
    type Output = PointerType<'ctx>;

    fn generate(
        &self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Type {
    type Output = BasicTypeEnum<'ctx>;

    fn generate(
        &self,
        ctx: &'ctx LLVMContext,
        store: &Store,
        symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement

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
                let llvm_element_type = store[element_type].generate(ctx, store, symbol_table);
                llvm_element_type.array_type(*len).into()
            }

            hir::Type::Tuple { element_types } => {
                let mut llvm_element_types = Vec::with_capacity(element_types.len());
                for element_type in element_types {
                    let llvm_element_type = store[element_type].generate(ctx, store, symbol_table);
                    llvm_element_types.push(llvm_element_type);
                }

                ctx.struct_type(&llvm_element_types, false).into()
            }

            hir::Type::Slice { element_type } => {
                // TODO: implement slice type
                unimplemented!()
            }

            hir::Type::Struct { struct_type } => {
                store[struct_type].generate(ctx, store, symbol_table).into()
            }

            hir::Type::Enum { enum_type } => {
                store[enum_type].generate(ctx, store, symbol_table).into()
            }

            hir::Type::Refine { base, .. } => store[base].generate(ctx, store, symbol_table).into(),

            hir::Type::Bitfield { base, bits } => {
                // TODO: implement bitfield type
                unimplemented!()
            }

            hir::Type::Function { function_type } => store[function_type]
                .generate(ctx, store, symbol_table)
                .into(),

            hir::Type::Reference { .. } | hir::Type::Pointer { .. } => {
                /* LLVM doesn't distinguish between pointer types anymore */
                ctx.ptr_type(AddressSpace::default()).into()
            }

            hir::Type::Symbol { path } => {
                match symbol_table
                    .get_type(path)
                    .expect("Unknown type name encountered during code generation")
                {
                    TypeDefinition::EnumDef(id) => {
                        let enum_def = store[id].borrow();
                        let enum_type = hir::Type::Enum {
                            enum_type: enum_def.enum_id,
                        };

                        enum_type.generate(ctx, store, symbol_table)
                    }

                    TypeDefinition::StructDef(id) => {
                        let struct_def = store[id].borrow();
                        let struct_type = hir::Type::Struct {
                            struct_type: struct_def.struct_id,
                        };

                        struct_type.generate(ctx, store, symbol_table)
                    }

                    TypeDefinition::TypeAliasDef(id) => {
                        let type_alias_def = store[id].borrow();
                        let aliased_type = &store[&type_alias_def.type_id];
                        aliased_type.generate(ctx, store, symbol_table)
                    }
                }
            }

            hir::Type::InferredFloat | hir::Type::InferredInteger | hir::Type::Inferred { .. } => {
                panic!("Inferred types should have been resolved before code generation")
            }
        }
    }
}
