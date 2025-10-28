use inkwell::module::Module;
use nitrate_hir::{Store, SymbolTab, hir};
use nitrate_hir_validate::ValidHir;
use nitrate_llvm::LLVMContext;

pub(crate) trait CodeGen<'ctx> {
    type Output;

    fn generate(&self, ctx: &'ctx LLVMContext, store: &Store, tab: &SymbolTab) -> Self::Output;
}

pub fn generate_code<'ctx>(
    hir: ValidHir<hir::Module>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) -> Module<'ctx> {
    hir.into_inner().generate(ctx, store, tab)
}
