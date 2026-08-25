//! Shared helpers for llvm_from_mir unit and regression tests.

use crate::generate_llvmir_from_mir;
use nitrate_llvm::{LLVMContext, OptLevel};
use nitrate_mir::prelude as mir;

/// Create a host-target LLVM context with no optimization.
pub fn llvm_ctx() -> LLVMContext {
    let triple = LLVMContext::default_target_triple();
    LLVMContext::new(&triple, OptLevel::None).expect("failed to create LLVM context")
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

/// Register a function `name(a: lhs_ty, b: rhs_ty) -> ret_ty` that computes
/// `a OP b` on the dynamic parameters and returns the result.
///
/// Parameters are used (rather than constants) so that LLVM does not
/// constant-fold the operation away; the emitted instruction remains visible
/// in the IR for assertion.
pub fn fn_binary(
    b: &mut mir::MirBuilder,
    name: &str,
    lhs_ty: mir::MirTypeId,
    rhs_ty: mir::MirTypeId,
    ret_ty: mir::MirTypeId,
    op: mir::MirBinaryOp,
) -> mir::MirFunctionId {
    let mut f = b.start_function(name.into(), ret_ty);
    let a = f.add_param("a".into(), lhs_ty, false);
    let b = f.add_param("b".into(), rhs_ty, false);
    let tmp = f.new_temp(ret_ty.clone(), false);
    f.create_block();
    let place = mir::Place::Local(tmp.clone());
    f.push_assign(
        place.clone(),
        mir::Rvalue::BinaryOp {
            op,
            lhs: mir::Operand::Copy(mir::Place::Local(a)),
            rhs: mir::Operand::Copy(mir::Place::Local(b)),
        },
    );
    f.ret(Some(mir::Operand::Copy(place)));
    f.finish_function()
}

/// Register a function `name(a: ty) -> ty` that applies `op` to a dynamic
/// parameter and returns the result.
pub fn fn_unary(b: &mut mir::MirBuilder, name: &str, ty: mir::MirTypeId, op: mir::MirUnaryOp) -> mir::MirFunctionId {
    let mut f = b.start_function(name.into(), ty.clone());
    let a = f.add_param("a".into(), ty.clone(), false);
    let tmp = f.new_temp(ty.clone(), false);
    f.create_block();
    let place = mir::Place::Local(tmp.clone());
    f.push_assign(
        place.clone(),
        mir::Rvalue::UnaryOp {
            op,
            operand: mir::Operand::Copy(mir::Place::Local(a)),
        },
    );
    f.ret(Some(mir::Operand::Copy(place)));
    f.finish_function()
}

/// Register a function `name(a: src_ty) -> target_ty` that casts a dynamic
/// parameter to the target type and returns it.
pub fn fn_cast(
    b: &mut mir::MirBuilder,
    name: &str,
    src_ty: mir::MirTypeId,
    target_ty: mir::MirTypeId,
) -> mir::MirFunctionId {
    let mut f = b.start_function(name.into(), target_ty.clone());
    let a = f.add_param("a".into(), src_ty, false);
    let tmp = f.new_temp(target_ty.clone(), false);
    f.create_block();
    let place = mir::Place::Local(tmp.clone());
    f.push_assign(
        place.clone(),
        mir::Rvalue::Cast {
            value: mir::Operand::Copy(mir::Place::Local(a)),
            target_ty: target_ty.clone(),
        },
    );
    f.ret(Some(mir::Operand::Copy(place)));
    f.finish_function()
}
