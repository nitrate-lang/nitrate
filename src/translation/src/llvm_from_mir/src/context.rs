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
// Global info
// ─────────────────────────────────────────────────────────────

/// Information about a global value needed by codegen: its pointer and its
/// original MIR type, so `gen_operand` can load the correct type rather than
/// guessing from the LLVM type.
#[derive(Clone, Copy)]
pub struct GlobalInfo<'ctx> {
    pub ptr: PointerValue<'ctx>,
    pub mir_ty: mir::MirTypeId,
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

    /// Map from LocalId (as usize → u32) → (alloca pointer, LLVM type)
    pub locals: HashMap<u32, (PointerValue<'ctx>, BasicTypeEnum<'ctx>)>,

    /// Map from global name → global info.
    pub globals: &'module HashMap<NString, GlobalInfo<'ctx>>,

    /// The current LLVM basic block being built.
    pub curr_block: Option<LlvmBasicBlock<'ctx>>,

    /// Map from MIR BasicBlockId → LLVM BasicBlock.
    pub blocks: HashMap<u32, LlvmBasicBlock<'ctx>>,

    /// The current function being compiled.
    pub function: FunctionValue<'ctx>,

    /// Map from MIR BasicBlockId (as usize) → list of phi nodes for block
    /// arguments. Each phi node corresponds to one block argument and receives
    /// incoming values from predecessor terminators.
    pub block_phi_nodes: HashMap<usize, Vec<RefCell<PhiValue<'ctx>>>>,

    /// Cache of string-literal globals created during operand evaluation, keyed
    /// by the literal bytes so duplicate literals share one global.
    pub str_literal_cache: HashMap<NString, PointerValue<'ctx>>,

    /// Counter for generating unique names for string-literal globals.
    pub str_literal_counter: u64,
}

impl<'ctx, 'module> CodegenCtx<'ctx, 'module> {
    pub fn new(
        llvm: &'ctx LLVMContext,
        module: &'module Module<'ctx>,
        builder: Builder<'ctx>,
        mir_func: &'module mir::MirFunction,
        globals: &'module HashMap<NString, GlobalInfo<'ctx>>,
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
            str_literal_cache: HashMap::new(),
            str_literal_counter: 0,
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

    /// Get or create a private global constant holding the given bytes as a
    /// null-terminated C string, returning a pointer to its first byte.
    pub fn intern_string_literal(&mut self, bytes: &[u8]) -> PointerValue<'ctx> {
        let key: NString = String::from_utf8_lossy(bytes).into_owned().into();
        if let Some(ptr) = self.str_literal_cache.get(&key) {
            return *ptr;
        }

        let name = format!("__nitrate_lit_str_{}", self.str_literal_counter);
        self.str_literal_counter += 1;

        let str_const = self.llvm.const_string(bytes, true);
        let str_ty = str_const.get_type();
        let str_global = self.module.add_global(str_ty, None, &name);
        str_global.set_initializer(&str_const);
        str_global.set_linkage(inkwell::module::Linkage::Private);
        str_global.set_unnamed_addr(true);
        str_global.set_constant(true);

        let ptr = str_global.as_pointer_value();
        self.str_literal_cache.insert(key, ptr);
        ptr
    }
}
