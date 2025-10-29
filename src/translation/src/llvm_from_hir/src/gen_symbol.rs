use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::BasicType;
use inkwell::values::{AsValueRef, FunctionValue, GlobalValue};

use crate::gen_rval::RvalGenCtx;
use crate::gen_rval::gen_rval;
use crate::ty::gen_ty;
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

pub struct SymbolGenCtx<'ctx, 'store, 'tab> {
    pub store: &'store hir::Store,
    pub tab: &'tab hir::SymbolTab,
    pub llvm: &'ctx LLVMContext,
    pub module: Module<'ctx>,
}

pub(crate) fn get_ptr_size(ctx: &LLVMContext) -> hir::PtrSize {
    let int_type = ctx.ptr_sized_int_type(&ctx.target_data(), None);
    match int_type.get_bit_width() {
        32 => hir::PtrSize::U32,
        64 => hir::PtrSize::U64,
        bits => panic!("Unsupported pointer size: {} bits", bits),
    }
}

fn gen_global<'ctx>(
    ctx: &'ctx SymbolGenCtx,
    package_name: &str,
    visibility: hir::Visibility,
    name: &str,
    init: Option<&hir::ValueId>,
    llvm_global: &mut GlobalValue<'ctx>,
) {
    /***********************************************************************/
    // 1. Set Linkage
    llvm_global.set_linkage(match visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
    });

    /***********************************************************************/
    // 2. Create Constructor Function
    let hir_ctor_type = hir::Type::Function {
        function_type: hir::FunctionType {
            attributes: BTreeSet::new(),
            params: Vec::new(),
            return_type: hir::Type::Unit.into_id(ctx.store),
        }
        .into_id(ctx.store),
    };

    let ctor_name = mangle_name(
        package_name,
        &format!("{}_ctor", name),
        &hir_ctor_type,
        ctx.store,
    );

    let llvm_ctor_function = ctx.module.add_function(
        ctor_name.as_str(),
        ctx.llvm.void_type().fn_type(&[], false),
        Some(Linkage::Private),
    );

    /***********************************************************************/
    // 3. Generate Constructor Body
    let bb = ctx.llvm.create_builder();

    let entry = ctx.llvm.append_basic_block(llvm_ctor_function, "entry");
    let endb = ctx.llvm.append_basic_block(llvm_ctor_function, "end");

    bb.position_at_end(entry);

    let ret = bb
        .build_alloca(ctx.llvm.struct_type(&[], false), "ret_val")
        .unwrap();

    let rval_ctx = RvalGenCtx {
        store: ctx.store,
        tab: ctx.tab,
        llvm: ctx.llvm,
        bb: &bb,
        ret: &ret,
        endb: &endb,
    };

    let init_value = ctx.store[init.expect("Initial value missing")].borrow();
    let llvm_init_value = gen_rval(&rval_ctx, &init_value);

    let global_ptr = llvm_global.as_pointer_value();
    bb.build_store(global_ptr, llvm_init_value).unwrap();

    bb.position_at_end(endb);
    bb.build_return(None).unwrap();

    /***********************************************************************/
    // 4. Register Constructor

    unsafe {
        nitrate_llvm_appendToGlobalCtors(
            ctx.module.as_mut_ptr(),
            llvm_ctor_function.as_value_ref(),
            65535,
        );
    }
}

fn gen_function<'ctx>(
    ctx: &'ctx SymbolGenCtx,
    visibility: hir::Visibility,
    body: Option<&hir::BlockId>,
    llvm_function: &FunctionValue<'ctx>,
) {
    let linkage = match visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
    };

    llvm_function.set_linkage(linkage);

    if let Some(body) = body {
        let bb = ctx.llvm.create_builder();

        /*******************************************************/
        /* Entry Block */
        let entry = ctx.llvm.append_basic_block(*llvm_function, "entry");
        bb.position_at_end(entry);

        /* Allocate space for the return value */
        let return_type = llvm_function.get_type().get_return_type().unwrap();
        let ret = bb.build_alloca(return_type, "ret_val").unwrap();

        /*******************************************************/
        /* End Block */
        let end = ctx.llvm.append_basic_block(*llvm_function, "end");
        bb.position_at_end(end);

        let ret_value = bb.build_load(return_type, ret, "ret_val_load").unwrap();

        bb.build_return(Some(&ret_value)).unwrap();

        /*******************************************************/
        /* Generate Body */
        bb.position_at_end(entry);

        for element in &ctx.store[body].borrow().elements {
            match element {
                hir::BlockElement::Local(_id) => {
                    // TODO: Create local variable
                }

                hir::BlockElement::Expr(_) => {
                    panic!(
                        "Non-statement expression cannot appear in block during code generation"
                    );
                }

                hir::BlockElement::Stmt(id) => {
                    let stmt = &ctx.store[id].borrow();
                    let rval_ctx = RvalGenCtx {
                        store: ctx.store,
                        tab: ctx.tab,
                        llvm: ctx.llvm,
                        bb: &bb,
                        ret: &ret,
                        endb: &end,
                    };

                    gen_rval(&rval_ctx, stmt);
                }
            }
        }
    }
}

pub(crate) fn gen_module<'ctx>(ctx: &'ctx SymbolGenCtx, module: &hir::Module) {
    let package_name = "";

    for item in &module.items {
        match item {
            hir::Item::TypeAliasDef(_) | hir::Item::StructDef(_) | hir::Item::EnumDef(_) => {}

            hir::Item::Module(id) => {
                let _submodule = gen_module(ctx, &ctx.store[id].borrow());
                // TODO: Handle submodules
            }

            hir::Item::GlobalVariable(id) => {
                let hir_global = ctx.store[id].borrow();
                let hir_global_ty = &ctx.store[&hir_global.ty];

                let global_name =
                    mangle_name(package_name, &hir_global.name, hir_global_ty, ctx.store);

                let global_ty = gen_ty(hir_global_ty, ctx.llvm, ctx.store, ctx.tab);
                let mut llvm_global = ctx.module.add_global(global_ty, None, global_name.as_str());
                llvm_global.set_initializer(&global_ty.const_zero());

                gen_global(
                    ctx,
                    package_name,
                    hir_global.visibility,
                    global_name.as_ref(),
                    hir_global.init.as_ref(),
                    &mut llvm_global,
                );
            }

            hir::Item::Function(id) => {
                let hir_fn = ctx.store[id].borrow();
                let hir_fn_type = hir::Type::Function {
                    function_type: hir_fn.get_type(ctx.store).into_id(ctx.store),
                };

                let mut param_types = Vec::with_capacity(hir_fn.params.len());
                for param in &hir_fn.params {
                    let param_type_id = ctx.store[param].borrow().ty;
                    let param_type =
                        gen_ty(&ctx.store[&param_type_id], ctx.llvm, ctx.store, ctx.tab);
                    param_types.push(param_type.into());
                }

                let variadic = hir_fn
                    .attributes
                    .contains(&hir::FunctionAttribute::Variadic);

                let return_type = gen_ty(
                    &ctx.store[&hir_fn.return_type],
                    ctx.llvm,
                    ctx.store,
                    ctx.tab,
                );

                let llvm_fn_type = return_type.fn_type(&param_types, variadic);
                let fn_name = mangle_name(package_name, &hir_fn.name, &hir_fn_type, ctx.store);
                let llvm_function = ctx
                    .module
                    .add_function(fn_name.as_str(), llvm_fn_type, None);

                gen_function(
                    ctx,
                    hir_fn.visibility,
                    hir_fn.body.as_ref(),
                    &mut llvm_function.clone(),
                );
            }
        }
    }
}
