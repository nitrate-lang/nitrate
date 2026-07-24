use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, BasicTypeEnum};
use inkwell::values::{AsValueRef, FunctionValue, PointerValue};
use nitrate_hir_validate::ValidHir;
use nitrate_nstring::NString;
use std::ops::Deref;
use thin_vec::ThinVec;

use crate::rvalue::CodegenCtx;
use crate::rvalue::gen_rval;
use crate::ty::{TypegenCtx, gen_ty};
use nitrate_hir::prelude as hir;
use nitrate_hir_mangle::mangle_name;
use nitrate_llvm::LLVMContext;
use std::collections::{BTreeSet, HashMap};

#[link(name = "nitrate_extra_llvm_ffi", kind = "static")]
unsafe extern "C" {
    fn nitrate_llvm_appendToGlobalCtors(module: LLVMModuleRef, function: LLVMValueRef, priority: u32) -> ();
}

pub struct SymbolGenCtx<'ctx, 'tab, 'package_name, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub tab: &'tab hir::SymbolTab,
    pub module: &'module Module<'ctx>,
    pub globals: HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
    pub package_name: &'package_name str,
}

impl<'ctx, 'tab, 'package_name, 'module> SymbolGenCtx<'ctx, 'tab, 'package_name, 'module> {
    fn ty_ctx(&self) -> TypegenCtx<'ctx, 'tab, 'module> {
        TypegenCtx {
            llvm: self.llvm,
            tab: self.tab,
            module: self.module,
        }
    }
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
    let hir_global_ty = hir_global.ty.deref();
    let global_ty = gen_ty(hir_global_ty, &mut ctx.ty_ctx());

    let llvm_global = ctx.module.add_global(global_ty, None, &hir_global.mangled_name);
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
            return_type: hir::Type::Unit.into(),
        }
        .into(),
    };

    let ctor_name = mangle_name(
        ctx.package_name,
        &format!("{}_ctor", hir_global.mangled_name),
        &hir_ctor_type,
    );

    let llvm_ctor_function = ctx.module.add_function(
        ctor_name.as_str(),
        ctx.llvm.void_type().fn_type(&[], false),
        Some(Linkage::Private),
    );

    /***********************************************************************/
    // Fill Constructor Body
    let bb = ctx.llvm.create_builder();
    let mut val_ctx = CodegenCtx::new(ctx.llvm, &ctx.module, ctx.tab, &bb, &ctx.globals);

    let entry = ctx.llvm.append_basic_block(llvm_ctor_function, "entry");
    bb.position_at_end(entry);

    let init_value = &hir_global.initializer.borrow();
    let llvm_init_value = gen_rval(&mut val_ctx, &init_value);
    let global_ptr = llvm_global.as_pointer_value();
    bb.build_store(global_ptr, llvm_init_value).unwrap();
    bb.build_return(None).unwrap();

    /***********************************************************************/
    // Register Global

    ctx.globals.insert(
        hir_global.mangled_name.to_owned(),
        (llvm_global.as_pointer_value(), global_ty),
    );

    /***********************************************************************/
    // Register Constructor

    unsafe {
        nitrate_llvm_appendToGlobalCtors(ctx.module.as_mut_ptr(), llvm_ctor_function.as_value_ref(), 65535);
    }
}

fn gen_function_decl<'ctx>(
    ctx: &mut SymbolGenCtx<'ctx, '_, '_, '_>,
    hir_function: &hir::Function,
) -> FunctionValue<'ctx> {
    if let Some(existing_function) = ctx.module.get_function(&hir_function.mangled_name) {
        return existing_function;
    }

    let mut param_types = Vec::with_capacity(hir_function.params.len());
    for param in &hir_function.params {
        let param_type_id = param.borrow().ty;
        let param_type = gen_ty(&param_type_id, &mut ctx.ty_ctx());
        param_types.push(param_type.into());
    }

    let return_type = gen_ty(&hir_function.return_type, &mut ctx.ty_ctx());

    let variadic = hir_function.attributes.contains(&hir::FunctionAttribute::CVariadic);

    let llvm_fn_type = return_type.fn_type(&param_types, variadic);
    let llvm_function = ctx.module.add_function(&hir_function.mangled_name, llvm_fn_type, None);

    llvm_function.set_linkage(match hir_function.visibility {
        hir::Visibility::Pub => Linkage::External,
        hir::Visibility::Pro => Linkage::Internal,
        hir::Visibility::Sec => Linkage::Private,
    });

    llvm_function
}

fn gen_function<'ctx>(ctx: &mut SymbolGenCtx<'ctx, '_, '_, '_>, hir_function: &hir::Function) -> FunctionValue<'ctx> {
    let llvm_function = gen_function_decl(ctx, hir_function);

    if let Some(body) = &hir_function.body {
        let bb = ctx.llvm.create_builder();
        let mut val_ctx = CodegenCtx::new(ctx.llvm, &ctx.module, ctx.tab, &bb, &ctx.globals);

        let entry = ctx.llvm.append_basic_block(llvm_function, "entry");
        bb.position_at_end(entry);

        for (i, param_id) in hir_function.params.iter().enumerate() {
            let hir_param = &param_id.borrow();
            let llvm_param_type = gen_ty(&hir_param.ty, &mut ctx.ty_ctx());
            let llvm_param = llvm_function.get_nth_param(i as u32).unwrap();

            let alloca = bb
                .build_alloca(llvm_param_type, &format!("param_{}", hir_param.name))
                .unwrap();
            bb.build_store(alloca, llvm_param).unwrap();

            val_ctx
                .parameters
                .insert(hir_param.name.to_owned(), (alloca, llvm_param_type));
        }

        for element in body {
            match element {
                hir::BlockElement::Expr(expr) => {
                    gen_rval(&mut val_ctx, &expr.borrow());
                }

                hir::BlockElement::Local(local) => {
                    let hir_local = local.borrow();
                    let local_name = hir_local.name.to_owned();
                    let hir_local_ty = &hir_local.ty;
                    let hir_local_init = &hir_local.initializer.borrow();

                    let llvm_local_ty = gen_ty(hir_local_ty, &mut ctx.ty_ctx());
                    let llvm_local = val_ctx.bb.build_alloca(llvm_local_ty, &local_name).unwrap();
                    let llvm_init_value = gen_rval(&mut val_ctx, hir_local_init);
                    val_ctx.bb.build_store(llvm_local, llvm_init_value).unwrap();

                    val_ctx.locals.insert(local_name, (llvm_local, llvm_local_ty));
                }
            };
        }
    }

    llvm_function
}

pub fn generate_llvmir<'ctx>(
    package_name: &str,
    hir: ValidHir<hir::Module>,
    llvm: &'ctx LLVMContext,
    tab: &hir::SymbolTab,
) -> Module<'ctx> {
    let hir = hir.into_inner();
    let module_name = hir.name.to_string();
    let module = llvm.create_module(&module_name);

    let mut ctx = SymbolGenCtx {
        tab,
        llvm,
        module: &module,
        globals: HashMap::new(),
        package_name,
    };

    // Skip generic (uninstantiated) functions - only monomorphized concrete
    // instances should be compiled to LLVM IR. Generic functions are templates
    // that are instantiated by the Hindley-Milner monomorphization pass.
    for function_id in tab.functions() {
        let func = function_id.borrow();
        if func.generics.is_some() && func.generics.as_ref().map_or(false, |g| !g.is_empty()) {
            continue;
        }
        gen_function_decl(&mut ctx, &func);
    }

    for global_id in tab.globals() {
        gen_global(&mut ctx, &global_id.borrow());
    }

    for function_id in tab.functions() {
        let func = function_id.borrow();
        if func.generics.is_some() && func.generics.as_ref().map_or(false, |g| !g.is_empty()) {
            continue;
        }
        gen_function(&mut ctx, &func);
    }

    if let Err(e) = ctx.module.verify() {
        eprintln!("LLVM Module Verification Error: {}", e.to_string());
        eprintln!("Generated LLVM Module:\n");
        eprintln!("{}", ctx.module.print_to_string().to_string());
        panic!("Generated LLVM module is invalid");
    }

    module
}
