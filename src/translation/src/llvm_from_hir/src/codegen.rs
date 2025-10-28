use inkwell::module::Module;
use nitrate_hir::{Store, SymbolTab, hir};
use nitrate_hir_validate::ValidHir;
use nitrate_llvm::LLVMContext;

pub(crate) trait CodeGen<'ctx> {
    type Output;

    fn generate(
        self,
        ctx: &'ctx LLVMContext,
        store: &Store,
        symbol_table: &SymbolTab,
    ) -> Self::Output;
}

pub fn generate_code<'ctx>(
    hir: ValidHir<hir::Module>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    symbol_table: &SymbolTab,
) -> Module<'ctx> {
    let hir = hir.into_inner();

    hir.generate(ctx, store, symbol_table)
}
