use crate::codegen::CodeGen;
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, BasicTypeEnum};
use inkwell::values::FunctionValue;
use nitrate_hir::hir::PtrSize;
use nitrate_hir::{Store, SymbolTab, hir::Visibility, prelude as hir};
use nitrate_llvm::LLVMContext;

pub(crate) fn get_ptr_size(ctx: &LLVMContext) -> PtrSize {
    let int_type = ctx.ptr_sized_int_type(&ctx.target_data(), None);
    match int_type.get_bit_width() {
        32 => PtrSize::U32,
        64 => PtrSize::U64,
        bits => panic!("Unsupported pointer size: {} bits", bits),
    }
}

impl<'ctx> CodeGen<'ctx> for hir::GlobalVariable {
    type Output = ();

    fn generate(&self, _ctx: &'ctx LLVMContext, _store: &Store, _tab: &SymbolTab) -> Self::Output {
        // TODO: implement global variable
        unimplemented!()
    }
}

fn generate_function<'ctx>(
    hir_function: &hir::Function,
    llvm_function: &mut FunctionValue<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) {
    // TODO: implement function body generation
    // unimplemented!()
}

impl<'ctx> CodeGen<'ctx> for hir::Module {
    type Output = Module<'ctx>;

    fn generate(&self, ctx: &'ctx LLVMContext, store: &Store, tab: &SymbolTab) -> Self::Output {
        let module = ctx.create_module(
            self.name
                .to_owned()
                .unwrap_or_default()
                .to_string()
                .as_str(),
        );

        for item in &self.items {
            match item {
                hir::Item::TypeAliasDef(_) | hir::Item::StructDef(_) | hir::Item::EnumDef(_) => {}

                hir::Item::Module(id) => {
                    let _submodule = store[id].borrow().generate(ctx, store, tab);

                    // TODO: Handle submodules
                }

                hir::Item::GlobalVariable(id) => {
                    // TODO: Handle global variables
                    unimplemented!()
                }

                hir::Item::Function(id) => {
                    let hir_function = store[id].borrow();

                    let linkage = match hir_function.visibility {
                        Visibility::Pub => Linkage::External,
                        Visibility::Pro => Linkage::Internal,
                        Visibility::Sec => Linkage::Private,
                    };

                    let return_type = &store[&hir_function.return_type];
                    let return_type = return_type.generate(ctx, store, tab);
                    let return_type = BasicTypeEnum::try_from(return_type).unwrap();

                    let mut param_types = Vec::with_capacity(hir_function.params.len());
                    for param in &hir_function.params {
                        let param_type_id = store[param].borrow().ty;
                        let param_type = store[&param_type_id].generate(ctx, store, tab);
                        param_types.push(param_type.into());
                    }

                    let variadic = hir_function
                        .attributes
                        .contains(&hir::FunctionAttribute::Variadic);

                    let function_type = return_type.fn_type(&param_types, variadic);

                    let llvm_function =
                        module.add_function(&hir_function.name, function_type, Some(linkage));

                    generate_function(&hir_function, &mut llvm_function.clone(), ctx, store, tab);
                }
            }
        }

        module
    }
}
