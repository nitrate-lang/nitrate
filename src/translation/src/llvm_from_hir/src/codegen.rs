use inkwell::module::Module;
use nitrate_hir::hir;
use nitrate_hir_validate::ValidHir;
use nitrate_llvm::LLVMContext;

pub fn generate_code<'ctx>(hir: ValidHir<hir::Module>, ctx: &'ctx LLVMContext) -> Module<'ctx> {
    let hir = hir.into_inner();

    let module = ctx.create_module(hir.name.unwrap_or_default().to_string().as_str());
    let bb = ctx.create_builder();

    // TODO: Implement full code generation from HIR to LLVM IR

    let i64_type = ctx.i64_type();
    let fn_type = i64_type.fn_type(&[i64_type.into(), i64_type.into(), i64_type.into()], false);
    let function = module.add_function("sum", fn_type, None);
    let basic_block = ctx.append_basic_block(function, "entry");

    bb.position_at_end(basic_block);

    let x = function.get_nth_param(0).unwrap().into_int_value();
    let y = function.get_nth_param(1).unwrap().into_int_value();
    let z = function.get_nth_param(2).unwrap().into_int_value();

    let sum = bb.build_int_add(x, y, "sum").unwrap();
    let sum = bb.build_int_add(sum, z, "sum").unwrap();

    bb.build_return(Some(&sum)).unwrap();

    module
}
