use crate::ty::TypegenCtx;
use inkwell::basic_block::BasicBlock as LlvmBasicBlock;
use inkwell::builder::Builder;
use inkwell::llvm_sys::prelude::{LLVMModuleRef, LLVMValueRef};
use inkwell::module::Module;
use inkwell::types::BasicTypeEnum;
use inkwell::values::{FunctionValue, PhiValue, PointerValue};
use nitrate_llvm::LLVMContext;
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;
use std::cell::RefCell;
use std::collections::HashMap;

// ─────────────────────────────────────────────────────────────
// FFI for global constructors
// ─────────────────────────────────────────────────────────────

#[link(name = "nitrate_extra_llvm_ffi", kind = "static")]
unsafe extern "C" {
    pub fn nitrate_llvm_appendToGlobalCtors(module: LLVMModuleRef, function: LLVMValueRef, priority: u32) -> ();
}

// ─────────────────────────────────────────────────────────────
// Codegen context
// ─────────────────────────────────────────────────────────────

/// Per-function codegen context.
pub struct CodegenCtx<'ctx, 'module> {
    pub llvm: &'ctx LLVMContext,
    pub module: &'module Module<'ctx>,
    pub builder: Builder<'ctx>,
    pub mir_func: &'module mir::MirFunction,

    /// Map from LocalId → (alloca pointer, LLVM type)
    pub locals: HashMap<u32, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,

    /// Map from global name → (global pointer, LLVM type)
    pub globals: &'module HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,

    /// The current LLVM basic block being built
    pub curr_block: Option<LlvmBasicBlock<'ctx>>,

    /// Map from MIR BasicBlockId → LLVM BasicBlock
    pub blocks: HashMap<u32, LlvmBasicBlock<'ctx>>,

    /// The current function being compiled
    pub function: FunctionValue<'ctx>,

    /// Map from MIR BasicBlockId (as usize) → list of phi nodes for block arguments.
    /// Each phi node corresponds to one block argument and will receive incoming
    /// values from predecessor terminators that carry block arguments.
    pub block_phi_nodes: HashMap<usize, Vec<RefCell<PhiValue<'ctx>>>>,
}

impl<'ctx, 'module> CodegenCtx<'ctx, 'module> {
    pub fn new(
        llvm: &'ctx LLVMContext,
        module: &'module Module<'ctx>,
        builder: Builder<'ctx>,
        mir_func: &'module mir::MirFunction,
        globals: &'module HashMap<NString, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,
        function: FunctionValue<'ctx>,
    ) -> Self {
        Self {
            llvm,
            module,
            builder,
            mir_func,
            locals: HashMap::new(),
            globals,
            curr_block: None,
            blocks: HashMap::new(),
            function,
            block_phi_nodes: HashMap::new(),
        }
    }

    pub fn ty_ctx(&mut self) -> TypegenCtx<'ctx, 'module> {
        TypegenCtx {
            llvm: self.llvm,
            module: self.module,
        }
    }

    /// Get the LLVM block for a MIR basic block, creating it if necessary.
    pub fn get_llvm_block(&mut self, bb_id: &mir::BasicBlockId) -> LlvmBasicBlock<'ctx> {
        let idx = bb_id.as_usize();
        if let Some(block) = self.blocks.get(&(idx as u32)) {
            return *block;
        }
        let name = format!("bb{}", idx);
        let block = self.llvm.append_basic_block(self.function, &name);
        self.blocks.insert(idx as u32, block);
        block
    }

    pub fn position_at_end(&mut self, block: LlvmBasicBlock<'ctx>) {
        self.builder.position_at_end(block);
        self.curr_block = Some(block);
    }
}
