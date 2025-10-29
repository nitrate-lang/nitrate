use inkwell::module::Module;

use nitrate_hir::prelude as hir;
use nitrate_hir_validate::ValidHir;
use nitrate_llvm::LLVMContext;

use crate::gen_symbol::{SymbolGenCtx, gen_module};

pub fn generate_code<'ctx>(
    hir: ValidHir<hir::Module>,
    llvm: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) -> Module<'ctx> {
    let hir = hir.into_inner();
    let module_name = hir.name.to_owned().unwrap_or_default().to_string();
    let module = llvm.create_module(&module_name);

    let ctx = SymbolGenCtx {
        store,
        tab,
        llvm,
        module,
    };

    gen_module(&ctx, &hir);
    ctx.module
}
