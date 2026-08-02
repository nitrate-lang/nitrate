use crate::context::CodegenCtx;
use crate::stmt::{gen_statement, gen_terminator};
use crate::ty::{TypegenCtx, gen_ty};
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicMetadataTypeEnum, BasicType, BasicTypeEnum};
use inkwell::values::{FunctionValue, PointerValue};
use nitrate_llvm::LLVMContext;
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;
use std::collections::HashMap;

/// Generate LLVM IR for a single MIR function.
pub fn gen_function<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, llvm_function: FunctionValue<'ctx>) {
    // Create entry block
    let entry = ctx.llvm.append_basic_block(llvm_function, "entry");
    ctx.position_at_end(entry);

    // Allocate locals (SSA registers become allocas)
    for (i, local_decl) in ctx.mir_func.locals.iter().enumerate() {
        let llvm_ty = gen_ty(&*local_decl.ty, &mut ctx.ty_ctx());
        let alloca = ctx.builder.build_alloca(llvm_ty, &format!("local_{}", i)).unwrap();
        ctx.locals.insert(i as u32, (alloca, llvm_ty));
    }

    // Map parameters to their allocas
    for (i, param_id) in ctx.mir_func.params.iter().enumerate() {
        let param_idx = param_id.as_usize() as u32;
        if let Some(llvm_param) = llvm_function.get_nth_param(i as u32) {
            if let Some((alloca, _)) = ctx.locals.get(&param_idx).copied() {
                ctx.builder.build_store(alloca, llvm_param).unwrap();
            }
        }
    }

    // Create all basic blocks first (to allow forward references)
    for bb_id in ctx.mir_func.blocks.iter() {
        ctx.get_llvm_block(bb_id);
    }

    // Branch from entry to the first basic block
    let entry_target = ctx.get_llvm_block(&ctx.mir_func.entry_block);
    ctx.builder.build_unconditional_branch(entry_target).unwrap();

    // Generate code for each basic block
    for bb_id in ctx.mir_func.blocks.iter() {
        let bb_data = bb_id.borrow();
        let llvm_bb = ctx.get_llvm_block(bb_id);
        ctx.position_at_end(llvm_bb);

        for stmt in bb_data.statements.iter() {
            gen_statement(ctx, stmt);
        }
        gen_terminator(ctx, &bb_data.terminator);
    }
}

/// Generate LLVM IR module from a MIR module.
///
/// This function translates the MIR representation into LLVM IR using the
/// Inkwell library. It handles:
/// - Function declarations and definitions
/// - Type generation (MirType → LLVM type)
/// - Basic block codegen (statements + terminators)
/// - Place/Operand/Rvalue evaluation
///
/// The conversion is straightforward because MIR basic blocks map directly
/// to LLVM basic blocks, and MIR terminators map directly to LLVM branch
/// instructions.
pub fn generate_llvmir_from_mir<'ctx>(
    package_name: &str,
    mir_module: &mir::MirModule,
    mir_store: &mir::MirStore,
    llvm: &'ctx LLVMContext,
) -> Module<'ctx> {
    let module = llvm.create_module(package_name);
    let mut globals: HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)> = HashMap::new();

    // First pass: declare all global variables
    for (global_name, global_ty) in mir_module.globals.iter() {
        let llvm_ty = mir::using_storage(mir_store, || {
            gen_ty(global_ty, &mut TypegenCtx { llvm, module: &module })
        });
        let global = module.add_global(llvm_ty, None, global_name);
        global.set_initializer(&llvm_ty.const_zero());
        global.set_linkage(Linkage::External);
        globals.insert(global_name.clone(), (global.as_pointer_value(), llvm_ty));
    }

    // Generate each function
    for func_id in mir_module.functions.iter() {
        mir::using_storage(mir_store, || {
            let mir_func_borrowed = func_id.borrow();

            if mir_func_borrowed.is_extern() {
                // For extern functions, just declare
                let return_ty = gen_ty(&mir_func_borrowed.return_ty, &mut TypegenCtx { llvm, module: &module });
                let param_tys: Vec<BasicMetadataTypeEnum<'ctx>> = mir_func_borrowed
                    .params
                    .iter()
                    .map(|pid| {
                        let local = pid.borrow();
                        gen_ty(&local.ty, &mut TypegenCtx { llvm, module: &module }).into()
                    })
                    .collect();
                let fn_ty = return_ty.fn_type(&param_tys, false);
                module.add_function(&mir_func_borrowed.name, fn_ty, Some(Linkage::External));
            } else {
                // For functions with bodies, declare and define
                let return_ty = gen_ty(&mir_func_borrowed.return_ty, &mut TypegenCtx { llvm, module: &module });
                let param_tys: Vec<BasicMetadataTypeEnum<'ctx>> = mir_func_borrowed
                    .params
                    .iter()
                    .map(|pid| {
                        let local = pid.borrow();
                        gen_ty(&local.ty, &mut TypegenCtx { llvm, module: &module }).into()
                    })
                    .collect();
                let fn_ty = return_ty.fn_type(&param_tys, false);
                let llvm_function = module.add_function(&mir_func_borrowed.name, fn_ty, Some(Linkage::External));

                let builder = llvm.create_builder();
                let mut ctx = CodegenCtx::new(llvm, &module, builder, &mir_func_borrowed, &globals, llvm_function);
                gen_function(&mut ctx, llvm_function);
            }
        });
    }

    // Verify the module
    if let Err(e) = module.verify() {
        eprintln!("LLVM Module Verification Error: {}", e.to_string());
        eprintln!("Generated LLVM Module:\n");
        eprintln!("{}", module.print_to_string().to_string());
        panic!("Generated LLVM module is invalid");
    }

    module
}
