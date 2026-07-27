use crate::rvalue::CodegenCtx;
use crate::rvalue::gen_rval;
use crate::ty::{TypegenCtx, gen_ty};
use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicType, BasicTypeEnum};
use inkwell::values::{AsValueRef, FunctionValue, PointerValue};
use nitrate_hir::prelude as hir;
use nitrate_hir_mangle::mangle_name;
use nitrate_hir_validate::ValidHir;
use nitrate_llvm::LLVMContext;
use nitrate_nstring::NString;
use std::collections::{BTreeSet, HashMap};
use std::ops::Deref;
use thin_vec::ThinVec;

/// LLVM calling convention values (from llvm-c/Core.h)
/// See: https://llvm.org/docs/LangRef.html#calling-conventions
mod call_conv {
    pub const C: u32 = 0;
    pub const FAST: u32 = 8;
    pub const COLD: u32 = 9;
    pub const GHCC: u32 = 10;
    pub const HIPCC: u32 = 11;
    pub const WEBKIT_JS: u32 = 12;
    pub const ANYREG: u32 = 13;
    pub const PRESERVE_MOST: u32 = 14;
    pub const PRESERVE_ALL: u32 = 15;
    pub const SWIFT: u32 = 16;
    pub const CXX_FAST_TLS: u32 = 17;
    pub const TAIL: u32 = 18;
    pub const SWIFT_TAIL: u32 = 19;
    pub const X86_STDCALL: u32 = 64;
    pub const X86_FASTCALL: u32 = 65;
    pub const ARM_APCS: u32 = 66;
    pub const ARM_AAPCS: u32 = 67;
    pub const ARM_AAPCS_VFP: u32 = 68;
    pub const MSP430_INTR: u32 = 69;
    pub const X86_THISCALL: u32 = 70;
    pub const PTX_KERNEL: u32 = 71;
    pub const PTX_DEVICE: u32 = 72;
    pub const SPIR_FUNC: u32 = 75;
    pub const SPIR_KERNEL: u32 = 76;
    pub const INTEL_OCL_BI: u32 = 77;
    pub const X86_64_SYSV: u32 = 78;
    pub const WIN64: u32 = 79;
    pub const X86_VECTORCALL: u32 = 80;
    pub const HHVM: u32 = 81;
    pub const HHVM_C: u32 = 82;
    pub const X86_INTR: u32 = 83;
    pub const AVR_INTR: u32 = 84;
    pub const AVR_SIGNAL: u32 = 85;
    pub const AVR_BUILTIN: u32 = 86;
    pub const AMDGPU_VS: u32 = 87;
    pub const AMDGPU_GS: u32 = 88;
    pub const AMDGPU_PS: u32 = 89;
    pub const AMDGPU_CS: u32 = 90;
    pub const AMDGPU_KERNEL: u32 = 91;
    pub const X86_REGCALL: u32 = 92;
    pub const AMDGPU_HS: u32 = 93;
    pub const AMDGPU_ES: u32 = 94;
    pub const AMDGPU_LS: u32 = 95;
    pub const AMDGPU_CALL: u32 = 96;
}

/// Determine the LLVM calling convention for a given extern ABI string.
/// Maps Rust/C-style ABI names to their LLVM calling convention IDs.
fn get_abi_call_conv(abi: &hir::ExternAbi) -> u32 {
    match &*abi.name {
        // Standard/common conventions
        "C" | "cdecl" => call_conv::C,
        "system" => call_conv::C,
        "rust-intrinsic" | "platform-intrinsic" | "rust-call" => call_conv::C,
        "unadjusted" => call_conv::C,

        // x86 conventions
        "fastcall" | "x86-fastcall" => call_conv::X86_FASTCALL,
        "stdcall" | "x86-stdcall" => call_conv::X86_STDCALL,
        "thiscall" | "x86-thiscall" => call_conv::X86_THISCALL,
        "vectorcall" | "x86-vectorcall" => call_conv::X86_VECTORCALL,
        "regcall" | "x86-regcall" => call_conv::X86_REGCALL,
        "x86-intr" => call_conv::X86_INTR,

        // x86-64 conventions
        "win64" | "x86-64-win64" => call_conv::WIN64,
        "sysv64" | "x86-64-sysv" => call_conv::X86_64_SYSV,

        // ARM conventions
        "aapcs" | "arm-aapcs" => call_conv::ARM_AAPCS,
        "aapcs-vfp" | "arm-aapcs-vfp" => call_conv::ARM_AAPCS_VFP,
        "arm-apcs" => call_conv::ARM_APCS,

        // GPU conventions
        "ptx-kernel" => call_conv::PTX_KERNEL,
        "ptx-device" => call_conv::PTX_DEVICE,
        "amdgpu-kernel" => call_conv::AMDGPU_KERNEL,
        "amdgpu-vs" => call_conv::AMDGPU_VS,
        "amdgpu-gs" => call_conv::AMDGPU_GS,
        "amdgpu-ps" => call_conv::AMDGPU_PS,
        "amdgpu-cs" => call_conv::AMDGPU_CS,
        "amdgpu-hs" => call_conv::AMDGPU_HS,
        "amdgpu-es" => call_conv::AMDGPU_ES,
        "amdgpu-ls" => call_conv::AMDGPU_LS,
        "amdgpu-call" => call_conv::AMDGPU_CALL,

        // SPIR conventions
        "spir-func" | "spir-function" => call_conv::SPIR_FUNC,
        "spir-kernel" => call_conv::SPIR_KERNEL,
        "intel-ocl-bicc" => call_conv::INTEL_OCL_BI,

        // Special conventions
        "cold" => call_conv::COLD,
        "fast" => call_conv::FAST,
        "swift" => call_conv::SWIFT,
        "swift-tail" => call_conv::SWIFT_TAIL,
        "preserve-most" => call_conv::PRESERVE_MOST,
        "preserve-all" => call_conv::PRESERVE_ALL,
        "tail" => call_conv::TAIL,
        "cxx-fast-tls" => call_conv::CXX_FAST_TLS,
        "ghc" | "ghcc" => call_conv::GHCC,
        "hipcc" => call_conv::HIPCC,
        "webkit-js" => call_conv::WEBKIT_JS,
        "anyreg" => call_conv::ANYREG,

        // HHVM conventions
        "hhvm" | "hhvmc" => call_conv::HHVM,
        "hhvm-c" => call_conv::HHVM_C,

        // AVR conventions
        "avr-intr" => call_conv::AVR_INTR,
        "avr-signal" => call_conv::AVR_SIGNAL,
        "avr-builtin" => call_conv::AVR_BUILTIN,

        // MSP430
        "msp430-intr" => call_conv::MSP430_INTR,

        // Default to C calling convention for unknown/unrecognized ABIs
        _ => panic!(
            "Unrecognized extern ABI: \"{}\". See the Nitrate reference for supported calling conventions.",
            abi.name
        ),
    }
}

/// Determine if a function should use external linkage based on its attributes.
fn has_extern_abi(attrs: &BTreeSet<hir::FunctionAttribute>) -> bool {
    attrs.iter().any(|a| matches!(a, hir::FunctionAttribute::ExternAbi(_)))
}

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
    let int_type = ctx.ptr_sized_int_type(ctx.target_data(), None);
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
    let mut val_ctx = CodegenCtx::new(ctx.llvm, ctx.module, ctx.tab, &bb, &ctx.globals);

    let entry = ctx.llvm.append_basic_block(llvm_ctor_function, "entry");
    bb.position_at_end(entry);

    let init_value = &hir_global.initializer.borrow();
    let llvm_init_value = gen_rval(&mut val_ctx, init_value);
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

    // Determine linkage: extern ABI functions without a body are external declarations,
    // so they must have External linkage regardless of visibility.
    // Otherwise, use visibility-based linkage (Pub→External, Pro→Internal, Sec→Private).
    let is_extern_decl = hir_function.body.is_none() && has_extern_abi(&hir_function.attributes);
    if is_extern_decl {
        llvm_function.set_linkage(Linkage::External);
    } else {
        llvm_function.set_linkage(match hir_function.visibility {
            hir::Visibility::Pub => Linkage::External,
            hir::Visibility::Pro => Linkage::Internal,
            hir::Visibility::Sec => Linkage::Private,
        });
    }

    // Apply calling convention based on extern ABI attribute
    for attr in &hir_function.attributes {
        if let hir::FunctionAttribute::ExternAbi(abi) = attr {
            llvm_function.set_call_conventions(get_abi_call_conv(abi));
            break;
        }
    }

    llvm_function
}

fn gen_function<'ctx>(ctx: &mut SymbolGenCtx<'ctx, '_, '_, '_>, hir_function: &hir::Function) -> FunctionValue<'ctx> {
    let llvm_function = gen_function_decl(ctx, hir_function);

    if let Some(body) = &hir_function.body {
        let bb = ctx.llvm.create_builder();
        let mut val_ctx = CodegenCtx::new(ctx.llvm, ctx.module, ctx.tab, &bb, &ctx.globals);

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
    // First pass: emit declarations for all non-generic functions.
    // This includes both functions with bodies and public extern declarations
    // (like printf) so that codegen can reference them.
    for function_id in tab.functions() {
        let func = function_id.borrow();
        if func.generics.is_some() && func.generics.as_ref().is_some_and(|g| !g.is_empty()) {
            continue;
        }
        // Skip abstract trait methods (bodyless, non-public) - they are
        // placeholder declarations that won't have a corresponding definition.
        // But emit declarations for extern functions regardless of visibility.
        if func.body.is_none() && func.visibility != hir::Visibility::Pub && !has_extern_abi(&func.attributes) {
            continue;
        }
        gen_function_decl(&mut ctx, &func);
    }

    for global_id in tab.globals() {
        gen_global(&mut ctx, &global_id.borrow());
    }

    // Second pass: generate function definitions for non-generic functions with bodies
    for function_id in tab.functions() {
        let func = function_id.borrow();
        if func.generics.is_some() && func.generics.as_ref().is_some_and(|g| !g.is_empty()) {
            continue;
        }
        if func.body.is_none() {
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
