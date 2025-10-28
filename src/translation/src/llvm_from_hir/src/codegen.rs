use inkwell::module::Module;
use nitrate_hir::{hir, prelude::*};
use nitrate_hir_validate::ValidHir;
use nitrate_llvm::LLVMContext;

pub fn generate_code<'ctx>(hir: ValidHir<hir::Module>, ctx: &'ctx LLVMContext) -> Module<'ctx> {
    let hir = hir.into_inner();

    let _module = ctx.create_module(hir.name.unwrap_or_default().to_string().as_str());
    let _bb = ctx.create_builder();

    unimplemented!()
}
