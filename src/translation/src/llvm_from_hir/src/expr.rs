use crate::codegen::CodeGen;
use inkwell::values::BasicValueEnum;
use nitrate_hir::{Store, SymbolTab, prelude as hir};
use nitrate_llvm::LLVMContext;

impl<'ctx> CodeGen<'ctx> for hir::Block {
    type Output = BasicValueEnum<'ctx>;

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        unimplemented!()
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Value {
    type Output = BasicValueEnum<'ctx>;

    fn generate(
        self,
        _ctx: &'ctx LLVMContext,
        _store: &Store,
        _symbol_table: &SymbolTab,
    ) -> Self::Output {
        unimplemented!()
    }
}
