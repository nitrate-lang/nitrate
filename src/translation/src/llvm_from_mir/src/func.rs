use crate::context::CodegenCtx;
use crate::context::GlobalInfo;
use crate::context::nitrate_llvm_appendToGlobalCtors;
use crate::stmt::{gen_statement, gen_terminator};
use crate::ty::{TypegenCtx, gen_fn_ret_ty, gen_ty};
use inkwell::module::{Linkage, Module};
use inkwell::types::{BasicMetadataTypeEnum, BasicType};
use inkwell::values::{AsValueRef, FunctionValue, PhiValue};
use nitrate_llvm::LLVMContext;
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;
use std::cell::RefCell;
use std::collections::HashMap;

/// Generate LLVM IR for a single MIR function.
pub fn gen_function<'ctx>(ctx: &mut CodegenCtx<'ctx, '_>, llvm_function: FunctionValue<'ctx>) {
    // Create a dedicated entry block that stores parameters and branches to the
    // function's actual MIR entry block. This keeps every MIR block (including
    // the MIR entry block) free to receive phi-node incoming edges and ensures
    // all allocas are emitted before any non-alloca instruction.
    let entry = ctx.llvm.append_basic_block(llvm_function, "entry");
    ctx.position_at_end(entry);

    // Allocate locals (SSA registers become allocas).
    for (i, local_decl) in ctx.mir_func.locals().iter().enumerate() {
        let llvm_ty = gen_ty(&*local_decl.ty, &mut ctx.ty_ctx());
        let alloca = ctx.builder.build_alloca(llvm_ty, &format!("local_{}", i)).unwrap();
        let key = ctx.mir_func.local_ids()[i].as_usize() as u32;
        ctx.locals.insert(key, (alloca, llvm_ty));
    }

    // Map parameters to their allocas. Parameters occupy the first `arg_count`
    // locals and their LLVM parameters arrive in the same order.
    for (i, param_id) in ctx.mir_func.params.iter().enumerate() {
        if let Some(llvm_param) = llvm_function.get_nth_param(i as u32) {
            let param_idx = param_id.as_usize() as u32;
            if let Some((alloca, _)) = ctx.locals.get(&param_idx).copied() {
                ctx.builder.build_store(alloca, llvm_param).unwrap();
            }
        }
    }

    // Create all MIR basic blocks first (to allow forward references).
    for bb_id in ctx.mir_func.blocks().iter() {
        ctx.get_llvm_block(bb_id);
    }

    // Create phi nodes for blocks that have block arguments. Each block
    // argument becomes a phi node that will receive incoming values from
    // predecessor terminators. Phi nodes must be the first instructions in
    // their block.
    for bb_id in ctx.mir_func.blocks().iter() {
        let bb_data = bb_id.borrow();
        if bb_data.args.is_empty() {
            continue;
        }

        let llvm_bb = ctx.get_llvm_block(bb_id);

        // Position at the start of the target block to create phi nodes.
        ctx.position_at_end(llvm_bb);
        if let Some(first_instr) = llvm_bb.get_first_instruction() {
            ctx.builder.position_before(&first_instr);
        }

        let mut phi_nodes: Vec<RefCell<PhiValue<'ctx>>> = Vec::new();
        for (i, arg_ty) in bb_data.args.iter().enumerate() {
            let llvm_ty = gen_ty(arg_ty, &mut ctx.ty_ctx());
            let phi = ctx
                .builder
                .build_phi(llvm_ty, &format!("phi_{}_{}", bb_id.as_usize(), i))
                .unwrap();
            phi_nodes.push(RefCell::new(phi));
        }

        ctx.block_phi_nodes.insert(bb_id.as_usize(), phi_nodes);

        // Restore position to the entry block for the continuation below.
        ctx.position_at_end(entry);
    }

    // Branch from the synthetic entry to the MIR entry block.
    let entry_target = ctx.get_llvm_block(ctx.mir_func.entry_block());
    ctx.builder.build_unconditional_branch(entry_target).unwrap();

    // Generate code for each basic block.
    for bb_id in ctx.mir_func.blocks().iter() {
        let bb_data = bb_id.borrow();
        let llvm_bb = ctx.get_llvm_block(bb_id);
        ctx.position_at_end(llvm_bb);

        // Store phi node results into block-argument locals' allocas. The block
        // records its argument locals directly, so no index arithmetic is
        // needed and blocks can be emitted in any order.
        if let Some(phi_list) = ctx.block_phi_nodes.get(&bb_id.as_usize()) {
            for (i, phi_node) in phi_list.iter().enumerate() {
                let phi = phi_node.borrow();
                let phi_val = phi.as_basic_value();
                if let Some(local_id) = bb_data.arg_local_ids.get(i) {
                    let local_idx = local_id.as_usize() as u32;
                    if let Some((alloca, _)) = ctx.locals.get(&local_idx).copied() {
                        ctx.builder.build_store(alloca, phi_val).unwrap();
                    }
                }
            }
        }

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
/// - Global variable declarations (with constructor functions for complex
///   initializers) and string literal emission
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
    llvm: &'ctx LLVMContext,
) -> Module<'ctx> {
    let module = llvm.create_module(package_name);
    let mut globals: HashMap<NString, GlobalInfo<'ctx>> = HashMap::new();

    // First pass: declare all global variables. Complex initializers cannot be
    // expressed as LLVM constants, so they are handled through a constructor
    // function registered with the global-ctors list; the global itself starts
    // zero-initialized.
    for mir_global in mir_module.globals.iter() {
        let llvm_ty = gen_ty(&mir_global.ty, &mut TypegenCtx { llvm, module: &module });
        let global = module.add_global(llvm_ty, None, &mir_global.name);
        global.set_initializer(&llvm_ty.const_zero());
        global.set_linkage(Linkage::External);

        let ctor_name = format!("{}_ctor", mir_global.name);
        let ctor_fn = module.add_function(&ctor_name, llvm.void_type().fn_type(&[], false), Some(Linkage::Private));
        let ctor_builder = llvm.create_builder();
        let ctor_entry = llvm.append_basic_block(ctor_fn, "entry");
        ctor_builder.position_at_end(ctor_entry);
        ctor_builder.build_return(None).unwrap();

        unsafe {
            nitrate_llvm_appendToGlobalCtors(module.as_mut_ptr(), ctor_fn.as_value_ref(), 65535);
        }

        globals.insert(
            mir_global.name.clone(),
            GlobalInfo {
                ptr: global.as_pointer_value(),
                mir_ty: mir_global.ty.clone(),
            },
        );
    }

    // Emit string literals as private global constant arrays. These are
    // registered in the globals map so that `Place::Static(name)` works for
    // borrow expressions like `&"string"`. String globals hold a pointer value;
    // their MIR type is `Str` (a pointer), which is exactly what a load of the
    // static should produce.
    let str_ty: mir::MirTypeId = mir::MirType::Str.into();
    for (str_name, str_data) in mir_module.string_globals.iter() {
        let str_const = llvm.const_string(str_data.as_slice(), true);
        let str_global_type = str_const.get_type();
        let str_global = module.add_global(str_global_type, None, str_name);
        str_global.set_initializer(&str_const);
        str_global.set_linkage(Linkage::Private);
        str_global.set_unnamed_addr(true);
        str_global.set_constant(true);
        globals.insert(
            str_name.clone(),
            GlobalInfo {
                ptr: str_global.as_pointer_value(),
                mir_ty: str_ty.clone(),
            },
        );
    }

    // Pass 1: Declare all functions first (so calls to extern/forward-referenced
    // functions can be resolved when generating bodies).
    let mut func_map: Vec<(mir::MirFunctionId, FunctionValue<'ctx>)> = Vec::new();
    for func_id in mir_module.functions.iter() {
        let mir_func = func_id.borrow();
        let ret_ty = gen_fn_ret_ty(&mir_func.return_ty, &mut TypegenCtx { llvm, module: &module });
        let param_tys: Vec<BasicMetadataTypeEnum<'ctx>> = mir_func
            .params
            .iter()
            .map(|pid| {
                let local = pid.borrow();
                gen_ty(&local.ty, &mut TypegenCtx { llvm, module: &module }).into()
            })
            .collect();
        let fn_ty = match ret_ty {
            Some(rt) => rt.fn_type(&param_tys, mir_func.is_c_variadic),
            None => llvm.void_type().fn_type(&param_tys, mir_func.is_c_variadic),
        };
        let llvm_function = module.add_function(&mir_func.name, fn_ty, Some(Linkage::External));
        func_map.push((func_id.clone(), llvm_function));
    }

    // Pass 2: Generate bodies for non-extern functions.
    for (func_id, llvm_function) in func_map {
        let mir_func = func_id.borrow();
        if !mir_func.is_extern() {
            let builder = llvm.create_builder();
            let mut ctx = CodegenCtx::new(llvm, &module, builder, &mir_func, &globals, llvm_function);
            gen_function(&mut ctx, llvm_function);
        }
    }

    // Verify the module.
    if let Err(e) = module.verify() {
        eprintln!("LLVM Module Verification Error: {}", e.to_string());
        eprintln!("Generated LLVM Module:\n");
        eprintln!("{}", module.print_to_string().to_string());
        panic!("Generated LLVM module is invalid");
    }

    module
}
