use crate::codegen::CodeGen;
use nitrate_hir::{Store, SymbolTab, prelude as hir};
use nitrate_llvm::LLVMContext;

impl<'ctx> CodeGen<'ctx> for hir::StructAttribute {
    type Output = ();

    fn generate(
        self,
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
        self,
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
        self,
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
        self,
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
        self,
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
        self,
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
        self,
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
        self,
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
        self,
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
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Type {
    type Output = ();

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        // TODO: implement
        unimplemented!()
    }
}
