use std::collections::BTreeSet;

use crate::codegen::CodeGen;
use crate::expr::expr_codegen;
use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::BasicType;
use inkwell::values::{AsValueRef, FunctionValue, GlobalValue};
use nitrate_hir::hir::{IntoStoreId, PtrSize};
use nitrate_hir::{Store, SymbolTab, hir::Visibility, prelude as hir};
use nitrate_hir_mangle::mangle_name;
use nitrate_llvm::LLVMContext;

#[link(name = "nitrate_extra_llvm_ffi", kind = "static")]
unsafe extern "C" {
    fn nitrate_llvm_appendToGlobalCtors(
        module: LLVMModuleRef,
        function: LLVMValueRef,
        priority: u32,
    ) -> ();
}

pub(crate) fn get_ptr_size(ctx: &LLVMContext) -> PtrSize {
    let int_type = ctx.ptr_sized_int_type(&ctx.target_data(), None);
    match int_type.get_bit_width() {
        32 => PtrSize::U32,
        64 => PtrSize::U64,
        bits => panic!("Unsupported pointer size: {} bits", bits),
    }
}

fn generate_global<'ctx>(
    package_name: &str,
    visibility: Visibility,
    name: &str,
    init: Option<&hir::ValueId>,
    llvm_global: &mut GlobalValue<'ctx>,
    llvm_module: &Module<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) {
    let linkage = match visibility {
        Visibility::Pub => Linkage::External,
        Visibility::Pro => Linkage::Internal,
        Visibility::Sec => Linkage::Private,
    };

    llvm_global.set_linkage(linkage);

    let hir_constructor_type = hir::Type::Function {
        function_type: hir::FunctionType {
            attributes: BTreeSet::new(),
            params: Vec::new(),
            return_type: hir::Type::Unit.into_id(store),
        }
        .into_id(store),
    };

    let constructor_name = mangle_name(
        package_name,
        &format!("{}_ctor", name),
        &hir_constructor_type,
        store,
    );

    let llvm_constructor_function = llvm_module.add_function(
        constructor_name.as_str(),
        ctx.void_type().fn_type(&[], false),
        Some(Linkage::Private),
    );

    let builder = ctx.create_builder();

    let entry = ctx.append_basic_block(llvm_constructor_function, "entry");
    builder.position_at_end(entry);

    let init_value = store[init.expect("Initial value missing")].borrow();
    let llvm_init_value = expr_codegen(&init_value, &builder, None, None, ctx, store, tab);

    let global_ptr = llvm_global.as_pointer_value();
    builder.build_store(global_ptr, llvm_init_value).unwrap();

    builder.build_return(None).unwrap();

    unsafe {
        nitrate_llvm_appendToGlobalCtors(
            llvm_module.as_mut_ptr(),
            llvm_constructor_function.as_value_ref(),
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
    visibility: Visibility,
    body: Option<&hir::BlockId>,
    llvm_function: &FunctionValue<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &Store,
    tab: &SymbolTab,
) {
    let linkage = match visibility {
        Visibility::Pub => Linkage::External,
        Visibility::Pro => Linkage::Internal,
        Visibility::Sec => Linkage::Private,
    };

    llvm_function.set_linkage(linkage);

    if let Some(body) = body {
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

        let package_name = "";

        for item in &self.items {
            match item {
                hir::Item::TypeAliasDef(_) | hir::Item::StructDef(_) | hir::Item::EnumDef(_) => {}

                hir::Item::Module(id) => {
                    let _submodule = store[id].borrow().generate(ctx, store, tab);

                    // TODO: Handle submodules
                }

                hir::Item::GlobalVariable(id) => {
                    let hir_global = store[id].borrow();
                    let hir_global_ty = &store[&hir_global.ty];

                    let global_name =
                        mangle_name(package_name, &hir_global.name, hir_global_ty, store);

                    let global_ty = hir_global_ty.generate(ctx, store, tab);
                    let mut llvm_global = module.add_global(global_ty, None, global_name.as_str());

                    generate_global(
                        package_name,
                        hir_global.visibility,
                        global_name.as_ref(),
                        hir_global.init.as_ref(),
                        &mut llvm_global,
                        &module,
                        ctx,
                        store,
                        tab,
                    );
                }

                hir::Item::Function(id) => {
                    let hir_fn = store[id].borrow();
                    let hir_fn_type = hir::Type::Function {
                        function_type: hir_fn.get_type(store).into_id(store),
                    };

                    let mut param_types = Vec::with_capacity(hir_fn.params.len());
                    for param in &hir_fn.params {
                        let param_type_id = store[param].borrow().ty;
                        let param_type = store[&param_type_id].generate(ctx, store, tab);
                        param_types.push(param_type.into());
                    }

                    let variadic = hir_fn
                        .attributes
                        .contains(&hir::FunctionAttribute::Variadic);

                    let return_type = &store[&hir_fn.return_type].generate(ctx, store, tab);
                    let llvm_fn_type = return_type.fn_type(&param_types, variadic);
                    let fn_name = mangle_name(package_name, &hir_fn.name, &hir_fn_type, store);
                    let llvm_function = module.add_function(fn_name.as_str(), llvm_fn_type, None);

                    generate_function(
                        hir_fn.visibility,
                        hir_fn.body.as_ref(),
                        &mut llvm_function.clone(),
                        ctx,
                        store,
                        tab,
                    );
                }
            }
        }

        module
    }
}
