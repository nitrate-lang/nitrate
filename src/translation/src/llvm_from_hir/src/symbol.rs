use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::BasicType;
use inkwell::values::{AsValueRef, FunctionValue, GlobalValue};
use nitrate_hir_validate::ValidHir;

use crate::rvalue::gen_rval;
use crate::rvalue::{CodegenCtx, gen_block};
use crate::ty::gen_ty;
use nitrate_hir::{IntoStoreId, prelude as hir};
use nitrate_hir_mangle::mangle_name;
use nitrate_llvm::LLVMContext;
use std::collections::{BTreeSet, HashMap};

#[link(name = "nitrate_extra_llvm_ffi", kind = "static")]
unsafe extern "C" {
    fn nitrate_llvm_appendToGlobalCtors(
        module: LLVMModuleRef,
        function: LLVMValueRef,
        priority: u32,
    ) -> ();
}

pub struct SymbolGenCtx<'ctx, 'store, 'tab, 'package_name> {
    pub store: &'store hir::Store,
    pub tab: &'tab hir::SymbolTab,
    pub llvm: &'ctx LLVMContext,
    pub module: Module<'ctx>,
    pub package_name: &'package_name str,
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
        ctx.package_name,
        &format!("{}_ctor", name),
        &hir_ctor_type,
        ctx.store,
    );

    let llvm_ctor_function = ctx.module.add_function(
        ctor_name.as_str(),
        ctx.llvm.void_type().fn_type(&[], false),
        Some(Linkage::Private),
    );

    let bb = ctx.llvm.create_builder();
    let mut rval_ctx = CodegenCtx {
        store: ctx.store,
        tab: ctx.tab,
        llvm: ctx.llvm,
        bb: &bb,
        locals: HashMap::new(),
        default_continue_target: Vec::new(),
        default_break_target: Vec::new(),
    };

    /***********************************************************************/
    // 3. Generate Constructor Body
    let entry = ctx.llvm.append_basic_block(llvm_ctor_function, "entry");
    let endb = ctx.llvm.append_basic_block(llvm_ctor_function, "end");

    bb.position_at_end(entry);
    let init_value = ctx.store[init.expect("Initial value missing")].borrow();
    let llvm_init_value = gen_rval(&mut rval_ctx, &init_value).expect("rvalue lowering err");

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
    /***********************************************************************/
    // 1. Set Linkage
    llvm_function.set_linkage(match visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
    });

    if let Some(body) = body {
        let bb = ctx.llvm.create_builder();

        let entry = ctx.llvm.append_basic_block(*llvm_function, "entry");

        /*******************************************************************/
        /* Generate Body */
        bb.position_at_end(entry);

        let mut rval_ctx = CodegenCtx {
            store: ctx.store,
            tab: ctx.tab,
            llvm: ctx.llvm,
            bb: &bb,
            locals: HashMap::new(),
            default_continue_target: Vec::new(),
            default_break_target: Vec::new(),
        };

        let body = &ctx.store[body].borrow();
        gen_block(&mut rval_ctx, body).expect("rvalue lowering err");
    }
}

pub(crate) fn gen_module<'ctx>(ctx: &'ctx SymbolGenCtx, module: &hir::Module) {
    for item in &module.items {
        match item {
            hir::Item::TypeAliasDef(_) | hir::Item::StructDef(_) | hir::Item::EnumDef(_) => {}

            hir::Item::Module(id) => gen_module(ctx, &ctx.store[id].borrow()),

            hir::Item::GlobalVariable(id) => {
                let hir_global = ctx.store[id].borrow();
                let hir_global_ty = &ctx.store[&hir_global.ty];

                let global_name = &hir_global.name;

                let global_ty = gen_ty(hir_global_ty, ctx.llvm, ctx.store, ctx.tab);
                let mut llvm_global = ctx.module.add_global(global_ty, None, global_name);
                llvm_global.set_initializer(&global_ty.const_zero());

                gen_global(
                    ctx,
                    hir_global.visibility,
                    global_name.as_ref(),
                    hir_global.init.as_ref(),
                    &mut llvm_global,
                );
            }

            hir::Item::Function(id) => {
                let hir_fn = ctx.store[id].borrow();

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
                let llvm_function = ctx.module.add_function(&hir_fn.name, llvm_fn_type, None);

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

pub fn generate_llvmir<'ctx>(
    package_name: &str,
    hir: ValidHir<hir::Module>,
    llvm: &'ctx LLVMContext,
    store: &hir::Store,
    tab: &hir::SymbolTab,
) -> Module<'ctx> {
    let hir = hir.into_inner();
    let module_name = hir.name.to_string();
    let module = llvm.create_module(&module_name);

    let ctx = SymbolGenCtx {
        store,
        tab,
        llvm,
        module,
        package_name,
    };

    gen_module(&ctx, &hir);

    if ctx.module.verify().is_err() {
        panic!("Generated LLVM module is invalid");
    }

    ctx.module
}
