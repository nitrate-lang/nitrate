use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, BasicTypeEnum};
use inkwell::values::{AsValueRef, FunctionValue, GlobalValue, PointerValue};
use nitrate_hir_validate::ValidHir;
use nitrate_nstring::NString;
use thin_vec::ThinVec;

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
    pub llvm: &'ctx LLVMContext,
    pub store: &'store hir::Store,
    pub tab: &'tab hir::SymbolTab,
    pub module: Module<'ctx>,
    pub globals: HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
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

fn gen_global<'ctx>(ctx: &mut SymbolGenCtx<'ctx, '_, '_, '_>, hir_global: &hir::GlobalVariable) {
    let hir_global_ty = &ctx.store[&hir_global.ty];
    let global_ty = gen_ty(hir_global_ty, ctx.llvm, ctx.store, ctx.tab);

    let llvm_global = ctx.module.add_global(global_ty, None, &hir_global.name);
    llvm_global.set_initializer(&global_ty.const_zero());
    llvm_global.set_linkage(match hir_global.visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
    });

    /***********************************************************************/
    // Create Constructor Function
    let hir_ctor_type = hir::Type::Function {
        function_type: hir::FunctionType {
            attributes: BTreeSet::new(),
            params: ThinVec::new(),
            return_type: hir::Type::Unit.into_id(ctx.store),
        }
        .into_id(ctx.store),
    };

    let ctor_name = mangle_name(
        ctx.package_name,
        &format!("{}_ctor", hir_global.name),
        &hir_ctor_type,
        ctx.store,
    );

    let llvm_ctor_function = ctx.module.add_function(
        ctor_name.as_str(),
        ctx.llvm.void_type().fn_type(&[], false),
        Some(Linkage::Private),
    );

    /***********************************************************************/
    // Fill Constructor Body
    let bb = ctx.llvm.create_builder();
    let mut val_ctx = CodegenCtx::new(ctx.llvm, &ctx.module, ctx.store, ctx.tab, &bb, &ctx.globals);

    let entry = ctx.llvm.append_basic_block(llvm_ctor_function, "entry");
    bb.position_at_end(entry);

    let init_value = ctx.store[&hir_global.init].borrow();
    let llvm_init_value = gen_rval(&mut val_ctx, &init_value).expect("rvalue lowering err");
    let global_ptr = llvm_global.as_pointer_value();
    bb.build_store(global_ptr, llvm_init_value).unwrap();
    bb.build_return(None).unwrap();

    /***********************************************************************/
    // Register Global

    ctx.globals.insert(
        hir_global.name.to_owned(),
        (llvm_global.as_pointer_value(), global_ty),
    );

    /***********************************************************************/
    // Register Constructor

    unsafe {
        nitrate_llvm_appendToGlobalCtors(
            ctx.module.as_mut_ptr(),
            llvm_ctor_function.as_value_ref(),
            65535,
        );
    }
}

fn gen_function_decl<'ctx>(
    ctx: &'ctx SymbolGenCtx,
    hir_function: &hir::Function,
) -> FunctionValue<'ctx> {
    if let Some(existing_function) = ctx.module.get_function(&hir_function.name) {
        return existing_function;
    }

    let mut param_types = Vec::with_capacity(hir_function.params.len());
    for param in &hir_function.params {
        let param_type_id = ctx.store[param].borrow().ty;
        let param_type = gen_ty(&ctx.store[&param_type_id], ctx.llvm, ctx.store, ctx.tab);
        param_types.push(param_type.into());
    }

    let return_type = gen_ty(
        &ctx.store[&hir_function.return_type],
        ctx.llvm,
        ctx.store,
        ctx.tab,
    );

    let variadic = hir_function
        .attributes
        .contains(&hir::FunctionAttribute::Variadic);

    let llvm_fn_type = return_type.fn_type(&param_types, variadic);
    let llvm_function = ctx
        .module
        .add_function(&hir_function.name, llvm_fn_type, None);

    llvm_function.set_linkage(match hir_function.visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
    });

    llvm_function
}

fn gen_function<'ctx>(
    ctx: &'ctx SymbolGenCtx,
    hir_function: &hir::Function,
) -> FunctionValue<'ctx> {
    let llvm_function = gen_function_decl(ctx, hir_function);

    if let Some(body) = &hir_function.body {
        let bb = ctx.llvm.create_builder();
        let mut val_ctx =
            CodegenCtx::new(ctx.llvm, &ctx.module, ctx.store, ctx.tab, &bb, &ctx.globals);

        let entry = ctx.llvm.append_basic_block(llvm_function, "entry");
        bb.position_at_end(entry);

        let body = &ctx.store[body].borrow();
        gen_block(&mut val_ctx, body).expect("rvalue lowering err");
    }

    llvm_function
}

pub(crate) fn gen_module<'ctx>(ctx: &mut SymbolGenCtx<'ctx, '_, '_, '_>, module: &hir::Module) {
    for item in &module.items {
        match item {
            hir::Item::TypeAliasDef(_) | hir::Item::StructDef(_) | hir::Item::EnumDef(_) => {}

            hir::Item::Module(id) => gen_module(ctx, &ctx.store[id].borrow()),

            hir::Item::GlobalVariable(id) => {
                let hir_global = ctx.store[id].borrow();
                gen_global(ctx, &hir_global);
            }

            hir::Item::Function(id) => {
                let hir_fn = ctx.store[id].borrow();
                gen_function(ctx, &hir_fn);
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

    let mut ctx = SymbolGenCtx {
        store,
        tab,
        llvm,
        module,
        globals: HashMap::new(),
        package_name,
    };

    for function_id in tab.functions() {
        let function = store[function_id].borrow();
        gen_function_decl(&ctx, &function);
    }

    gen_module(&mut ctx, &hir);

    if ctx.module.verify().is_err() {
        panic!("Generated LLVM module is invalid");
    }

    ctx.module
}
