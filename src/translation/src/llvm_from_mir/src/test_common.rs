//! Shared helpers for llvm_from_mir unit and regression tests.

use std::format;

use crate::generate_llvmir_from_mir;
use nitrate_llvm::{LLVMContext, OptLevel};
use nitrate_mir::prelude as mir;
use nitrate_nstring::NString;

/// Create a host-target LLVM context with no optimization.
pub fn llvm_ctx() -> LLVMContext {
    let triple = LLVMContext::default_target_triple();
    LLVMContext::new(&triple, OptLevel::None).expect("failed to create LLVM context")
}

/// Intern a string.
pub fn nstr(s: &str) -> NString {
    s.into()
}

/// An LLVM type string for a pointer-sized integer (`i32`/`i64`).
pub fn usize_type(llvm: &LLVMContext) -> String {
    format!("i{}", llvm.ptr_size() * 8)
}

/// A harness that builds a MIR module and lowers it to LLVM IR text, keeping
/// the MIR store installed in thread-local storage for the whole codegen phase.
pub struct Harness {
    pub llvm: LLVMContext,
    pub store: mir::MirStore,
}

impl Default for Harness {
    fn default() -> Self {
        Self::new()
    }
}

impl Harness {
    pub fn new() -> Self {
        Self {
            llvm: llvm_ctx(),
            store: mir::MirStore::new(),
        }
    }

    /// Build a MIR module and lower it to LLVM IR in a single TLS scope.
    pub fn build_ir(&self, build: impl FnOnce(&mut mir::MirBuilder)) -> String {
        mir::using_storage(&self.store, || {
            let mut builder = mir::MirBuilder::new();
            build(&mut builder);
            let module = builder.build_module(mir::PtrSize::U64);
            let llvm_module = generate_llvmir_from_mir("test", &module, &self.llvm);
            llvm_module.print_to_string().to_string()
        })
    }

    /// Build a MIR module inside TLS. The returned module must be consumed via
    /// [`Self::ir`] or [`Self::verify`], which re-install the store for codegen.
    pub fn build_module(&self, build: impl FnOnce(&mut mir::MirBuilder)) -> mir::MirModule {
        mir::using_storage(&self.store, || {
            let mut builder = mir::MirBuilder::new();
            build(&mut builder);
            builder.build_module(mir::PtrSize::U64)
        })
    }

    /// Lower a prebuilt MIR module to IR text (re-installs the store for codegen).
    pub fn ir(&self, module: &mir::MirModule) -> String {
        mir::using_storage(&self.store, || {
            let llvm_module = generate_llvmir_from_mir("test", module, &self.llvm);
            llvm_module.print_to_string().to_string()
        })
    }

    /// Lower a prebuilt MIR module and verify the result is valid LLVM IR.
    pub fn verify(&self, module: &mir::MirModule) -> bool {
        mir::using_storage(&self.store, || {
            let llvm_module = generate_llvmir_from_mir("test", module, &self.llvm);
            llvm_module.verify().is_ok()
        })
    }
}

/// Register a function that returns a single operand with a unique generated
/// name to avoid symbol collisions.
pub fn fn_return(b: &mut mir::MirBuilder, name: &str, ret_ty: mir::MirTypeId, op: mir::Operand) -> mir::MirFunctionId {
    let mut f = b.start_function(name.into(), ret_ty);
    f.create_block();
    f.ret(Some(op));
    f.finish_function()
}

/// Register a function that returns `()` without an operand.
pub fn fn_void(b: &mut mir::MirBuilder, name: &str) -> mir::MirFunctionId {
    let mut f = b.start_function(name.into(), mir::MirType::Unit.into());
    f.create_block();
    f.ret(None);
    f.finish_function()
}

/// Register a function whose body is simply `unreachable`.
pub fn fn_unreachable(b: &mut mir::MirBuilder, name: &str, ret_ty: mir::MirTypeId) -> mir::MirFunctionId {
    let mut f = b.start_function(name.into(), ret_ty);
    f.create_block();
    f.unreachable();
    f.finish_function()
}

/// Register a function that computes a binary op on two operands and returns
/// the result (through an intermediate temp local).
pub fn fn_binary(
    b: &mut mir::MirBuilder,
    name: &str,
    ret_ty: mir::MirTypeId,
    op: mir::MirBinaryOp,
    lhs: mir::Operand,
    rhs: mir::Operand,
) -> mir::MirFunctionId {
    let mut f = b.start_function(name.into(), ret_ty);
    let tmp = f.new_temp(ret_ty.clone(), false);
    f.create_block();
    let place = mir::Place::Local(tmp.clone());
    f.push_assign(place.clone(), mir::Rvalue::BinaryOp { op, lhs, rhs });
    f.ret(Some(mir::Operand::Copy(place)));
    f.finish_function()
}
