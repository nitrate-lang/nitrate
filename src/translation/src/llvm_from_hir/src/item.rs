use crate::codegen::CodeGen;
use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::BasicType;
use inkwell::values::{AsValueRef, FunctionValue, GlobalValue};
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

#[link(name = "nitrate_extra_llvm_ffi", kind = "static")]
unsafe extern "C" {
    fn nitrate_llvm_appendToGlobalCtors(
        module: LLVMModuleRef,
        function: LLVMValueRef,
        priority: u32,
    ) -> ();
}

fn generate_global<'ctx>(
    hir_global: &hir::GlobalVariable,
    llvm_global: &mut GlobalValue<'ctx>,
    llvm_module: &Module<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) {
    let linkage = match hir_global.visibility {
        Visibility::Pub => Linkage::External,
        Visibility::Pro => Linkage::Internal,
        Visibility::Sec => Linkage::Private,
    };

    llvm_global.set_linkage(linkage);

    let constructor_type = ctx.void_type().fn_type(&[], false);
    let constructor_function = llvm_module.add_function(
        format!("{}_ctor", hir_global.name).as_str(),
        constructor_type,
        None,
    );

    // TODO: insert constructor with LLVM appendToGlobalCtors

    let builder = ctx.create_builder();

    let entry_block = ctx.append_basic_block(constructor_function, "entry");
    builder.position_at_end(entry_block);

    let initial_value = hir_global.init.as_ref().expect("Initial value missing");
    let initial_value = store[initial_value].borrow().generate(ctx, store, tab);

    let global_ptr = llvm_global.as_pointer_value();
    builder.build_store(global_ptr, initial_value).unwrap();

    builder.build_return(None).unwrap();

    unsafe {
        nitrate_llvm_appendToGlobalCtors(
            llvm_module.as_mut_ptr(),
            constructor_function.as_value_ref(),
            65535,
        );
    }
}

pub(crate) fn generate_function_body<'ctx>(
    hir_block: &hir::Block,
    builder: inkwell::builder::Builder<'ctx>,
    return_value: inkwell::values::PointerValue<'ctx>,
    end_block: inkwell::basic_block::BasicBlock<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) {
    builder.build_unconditional_branch(end_block).unwrap();
    // TODO: implement block generation
}

fn generate_function<'ctx>(
    hir_function: &hir::Function,
    llvm_function: &FunctionValue<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) {
    let linkage = match hir_function.visibility {
        Visibility::Pub => Linkage::External,
        Visibility::Pro => Linkage::Internal,
        Visibility::Sec => Linkage::Private,
    };

    llvm_function.set_linkage(linkage);

    if let Some(body) = &hir_function.body {
        let builder = ctx.create_builder();

        /*******************************************************/
        /* Entry Block */
        let entry = ctx.append_basic_block(*llvm_function, "entry");
        builder.position_at_end(entry);

        /* Allocate space for the return value */
        let return_type = llvm_function.get_type().get_return_type().unwrap();
        let return_value_storage = builder.build_alloca(return_type, "ret_val").unwrap();

        /*******************************************************/
        /* End Block */
        let end = ctx.append_basic_block(*llvm_function, "end");
        builder.position_at_end(end);

        let ret_value = builder
            .build_load(return_type, return_value_storage, "ret_val_load")
            .unwrap();

        builder.build_return(Some(&ret_value)).unwrap();

        /*******************************************************/
        /* Generate Body */
        builder.position_at_end(entry);

        let block = store[body].borrow();
        generate_function_body(&block, builder, return_value_storage, end, ctx, store, tab);
    }
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
                    let hir_global = store[id].borrow();
                    let ty = store[&hir_global.ty].generate(ctx, store, tab);
                    let mut llvm_global = module.add_global(ty, None, &hir_global.name);

                    generate_global(&hir_global, &mut llvm_global, &module, ctx, store, tab);
                }

                hir::Item::Function(id) => {
                    let hir_function = store[id].borrow();

                    let mut param_types = Vec::with_capacity(hir_function.params.len());
                    for param in &hir_function.params {
                        let param_type_id = store[param].borrow().ty;
                        let param_type = store[&param_type_id].generate(ctx, store, tab);
                        param_types.push(param_type.into());
                    }

                    let variadic = hir_function
                        .attributes
                        .contains(&hir::FunctionAttribute::Variadic);

                    let return_type = &store[&hir_function.return_type].generate(ctx, store, tab);
                    let fn_type = return_type.fn_type(&param_types, variadic);
                    let llvm_function = module.add_function(&hir_function.name, fn_type, None);

                    generate_function(&hir_function, &mut llvm_function.clone(), ctx, store, tab);
                }
            }
        }

        module
    }
}
