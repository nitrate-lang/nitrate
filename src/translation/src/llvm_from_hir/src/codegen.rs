use inkwell::module::Module;
use nitrate_hir::prelude as hir;
use nitrate_hir_validate::ValidHir;
use nitrate_llvm::LLVMContext;

pub(crate) trait CodeGen<'ctx> {
    type Output;

    fn generate(
        &self,
        ctx: &'ctx LLVMContext,
        store: &hir::Store,
        tab: &hir::SymbolTab,
    ) -> Self::Output;
}

pub fn generate_code<'ctx>(
    hir: ValidHir<hir::Module>,
    ctx: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) -> Module<'ctx> {
    hir.into_inner().generate(ctx, store, tab)
}
