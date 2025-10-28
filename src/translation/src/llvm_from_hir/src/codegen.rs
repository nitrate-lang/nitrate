use inkwell::module::Module;
use nitrate_hir::{hir, prelude::*};
use nitrate_llvm::LLVMContext;

pub fn generate_code<'ctx>(hir: hir::Module, ctx: &'ctx LLVMContext) -> Module<'ctx> {
    let _module = ctx.create_module(hir.name.unwrap_or_default().to_string().as_str());
    let _bb = ctx.create_builder();

    unimplemented!()
}
