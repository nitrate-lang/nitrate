use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::BasicType;
use inkwell::values::{AsValueRef, FunctionValue, GlobalValue};

use crate::{gen_rval::gen_rval, gencode::CodeGen};
use nitrate_hir::{IntoStoreId, prelude as hir};
use nitrate_hir_mangle::mangle_name;
use nitrate_llvm::LLVMContext;
use std::collections::BTreeSet;

#[link(name = "nitrate_extra_llvm_ffi", kind = "static")]
unsafe extern "C" {
    fn nitrate_llvm_appendToGlobalCtors(
        module: LLVMModuleRef,
        function: LLVMValueRef,
        priority: u32,
    ) -> ();
}

pub(crate) fn get_ptr_size(ctx: &LLVMContext) -> hir::PtrSize {
    let int_type = ctx.ptr_sized_int_type(&ctx.target_data(), None);
    match int_type.get_bit_width() {
        32 => hir::PtrSize::U32,
        64 => hir::PtrSize::U64,
        bits => panic!("Unsupported pointer size: {} bits", bits),
    }
}

fn generate_global<'ctx>(
    package_name: &str,
    visibility: hir::Visibility,
    name: &str,
    init: Option<&hir::ValueId>,
    llvm_global: &mut GlobalValue<'ctx>,
    llvm_module: &Module<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) {
    let linkage = match visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
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

    let bb = ctx.create_builder();

    let entry = ctx.append_basic_block(llvm_constructor_function, "entry");
    bb.position_at_end(entry);

    let init_value = store[init.expect("Initial value missing")].borrow();
    let llvm_init_value = gen_rval(&init_value, &bb, None, None, ctx, store, tab);

    let global_ptr = llvm_global.as_pointer_value();
    bb.build_store(global_ptr, llvm_init_value).unwrap();

    bb.build_return(None).unwrap();

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
    bb: &inkwell::builder::Builder<'ctx>,
    ret: inkwell::values::PointerValue<'ctx>,
    endb: inkwell::basic_block::BasicBlock<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) {
    for element in &hir_block.elements {
        match element {
            hir::BlockElement::Local(_id) => {
                // TODO: Create local variable
            }

            hir::BlockElement::Expr(_) => {
                panic!("Non-statement expression cannot appear in block during code generation");
            }

            hir::BlockElement::Stmt(id) => {
                let stmt = &store[id].borrow();
                gen_rval(stmt, bb, Some(&ret), Some(&endb), ctx, store, tab);
            }
        }
    }
}

fn generate_function<'ctx>(
    visibility: hir::Visibility,
    body: Option<&hir::BlockId>,
    llvm_function: &FunctionValue<'ctx>,
    ctx: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) {
    let linkage = match visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
    };

    llvm_function.set_linkage(linkage);

    if let Some(body) = body {
        let bb = ctx.create_builder();

        /*******************************************************/
        /* Entry Block */
        let entry = ctx.append_basic_block(*llvm_function, "entry");
        bb.position_at_end(entry);

        /* Allocate space for the return value */
        let return_type = llvm_function.get_type().get_return_type().unwrap();
        let ret = bb.build_alloca(return_type, "ret_val").unwrap();

        /*******************************************************/
        /* End Block */
        let end = ctx.append_basic_block(*llvm_function, "end");
        bb.position_at_end(end);

        let ret_value = bb.build_load(return_type, ret, "ret_val_load").unwrap();

        bb.build_return(Some(&ret_value)).unwrap();

        /*******************************************************/
        /* Generate Body */
        bb.position_at_end(entry);

        let body = store[body].borrow();
        generate_function_body(&body, &bb, ret, end, ctx, store, tab);
    }
}

impl<'ctx> CodeGen<'ctx> for hir::Module {
    type Output = Module<'ctx>;

    fn generate(
        &self,
        ctx: &'ctx LLVMContext,
        store: &hir::Store,
        tab: &hir::SymbolTab,
    ) -> Self::Output {
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
                    llvm_global.set_initializer(&global_ty.const_zero());

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
