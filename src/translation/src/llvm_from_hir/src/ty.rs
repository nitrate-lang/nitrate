use crate::codegen::CodeGen;
use inkwell::types::{AnyTypeEnum, BasicMetadataTypeEnum, BasicTypeEnum};
use nitrate_hir::{Store, SymbolTab, prelude as hir};
use nitrate_llvm::LLVMContext;

impl<'ctx> CodeGen<'ctx> for hir::StructAttribute {
    type Output = ();

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

impl<'ctx> CodeGen<'ctx> for hir::StructFieldAttribute {
    type Output = ();

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

impl<'ctx> CodeGen<'ctx> for hir::StructField {
    type Output = ();

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

impl<'ctx> CodeGen<'ctx> for hir::StructType {
    type Output = ();

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

impl<'ctx> CodeGen<'ctx> for hir::EnumAttribute {
    type Output = ();

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

impl<'ctx> CodeGen<'ctx> for hir::EnumVariantAttribute {
    type Output = ();

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

impl<'ctx> CodeGen<'ctx> for hir::EnumVariant {
    type Output = ();

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
    type Output = ();

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

impl<'ctx> CodeGen<'ctx> for hir::FunctionAttribute {
    type Output = ();

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
    type Output = ();

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
    type Output = BasicMetadataTypeEnum<'ctx>;

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
            hir::Type::USize => todo!(),
            hir::Type::F32 => ctx.f32_type().into(),
            hir::Type::F64 => ctx.f64_type().into(),

            hir::Type::Opaque { name } => todo!(),

            hir::Type::Array { element_type, len } => todo!(),

            hir::Type::Tuple { element_types } => todo!(),

            hir::Type::Slice { element_type } => todo!(),

            hir::Type::Struct { struct_type } => todo!(),

            hir::Type::Enum { enum_type } => todo!(),

            hir::Type::Refine { base, min, max } => todo!(),

            hir::Type::Bitfield { base, bits } => todo!(),

            hir::Type::Function { function_type } => todo!(),

            hir::Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => todo!(),

            hir::Type::Pointer {
                exclusive,
                mutable,
                to,
            } => todo!(),

            hir::Type::Symbol { path } => todo!(),

            hir::Type::InferredFloat => todo!(),

            hir::Type::InferredInteger => todo!(),

            hir::Type::Inferred { id } => todo!(),
        }
    }
}
