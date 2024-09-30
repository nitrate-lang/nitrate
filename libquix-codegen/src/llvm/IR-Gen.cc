////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#define __QUIX_IMPL__
#define QXIR_USE_CPP_API

#include <core/LibMacro.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/InitializePasses.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <quix-codegen/Code.h>
#include <quix-codegen/Config.h>
#include <quix-core/Error.h>
#include <quix-qxir/Module.h>
#include <quix-qxir/Node.h>

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <streambuf>

class OStreamWriter : public std::streambuf {
  FILE *m_file;

public:
  OStreamWriter(FILE *file) : m_file(file) {}

  virtual std::streamsize xsputn(const char *s, std::streamsize n) override {
    return fwrite(s, 1, n, m_file);
  }

  virtual int overflow(int c) override { return fputc(c, m_file); }
};

class OStreamDiscard : public std::streambuf {
public:
  virtual std::streamsize xsputn(const char *s, std::streamsize n) override { return n; }
  virtual int overflow(int c) override { return c; }
};

class my_pwrite_ostream : public llvm::raw_pwrite_stream {
  std::ostream &m_os;

public:
  my_pwrite_ostream(std::ostream &os) : llvm::raw_pwrite_stream(true), m_os(os) {}

  void write_impl(const char *ptr, size_t size) override { m_os.write(ptr, size); }

  void pwrite_impl(const char *ptr, size_t size, uint64_t offset) override {
    auto curpos = current_pos();

    m_os.seekp(offset);
    m_os.write(ptr, size);
    m_os.seekp(curpos);
  }

  uint64_t current_pos() const override {
    auto pos = m_os.tellp();
    qcore_assert(pos != -1, "failed to get current position");

    return pos;
  }
};

/*

QIR_NODE_BINEXPR
QIR_NODE_UNEXPR
QIR_NODE_POST_UNEXPR
QIR_NODE_INT
QIR_NODE_FLOAT
QIR_NODE_STRING
QIR_NODE_LIST
QIR_NODE_CALL
QIR_NODE_SEQ
QIR_NODE_INDEX
QIR_NODE_EXTERN
QIR_NODE_LOCAL
QIR_NODE_RET
QIR_NODE_BRK
QIR_NODE_CONT
QIR_NODE_IF
QIR_NODE_WHILE
QIR_NODE_FOR
QIR_NODE_FORM
QIR_NODE_CASE
QIR_NODE_SWITCH
QIR_NODE_FN
QIR_NODE_ASM
QIR_NODE_U1_TY
QIR_NODE_U8_TY
QIR_NODE_U16_TY
QIR_NODE_U32_TY
QIR_NODE_U64_TY
QIR_NODE_U128_TY
QIR_NODE_I8_TY
QIR_NODE_I16_TY
QIR_NODE_I32_TY
QIR_NODE_I64_TY
QIR_NODE_I128_TY
QIR_NODE_F16_TY
QIR_NODE_F32_TY
QIR_NODE_F64_TY
QIR_NODE_F128_TY
QIR_NODE_VOID_TY
QIR_NODE_PTR_TY
QIR_NODE_OPAQUE_TY
QIR_NODE_STRING_TY
QIR_NODE_STRUCT_TY
QIR_NODE_UNION_TY
QIR_NODE_ARRAY_TY
QIR_NODE_FN_TY
*/

/* LLVM IR
std::unique_ptr<llvm::Module> module = action->takeModule();
    if (!module) {
      err << "error: failed to take module" << std::endl;
      return false;
    }

    if (llvm::verifyModule(*module, &err_os)) {
      err << "error: failed to verify module" << std::endl;
      return false;
    }

    ///==========================================================================

    // Create the analysis managers.
    // These must be declared in this order so that they are destroyed in the
    // correct order due to inter-analysis-manager references.
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // Create the new pass manager builder.
    // Take a look at the PassBuilder constructor parameters for more
    // customization, e.g. specifying a TargetMachine or various debugging
    // options.
    llvm::PassBuilder PB;

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

    // Optimize the IR!
    MPM.run(*module, MAM);

    ///==========================================================================

    llvm::raw_os_ostream ostream(code);
    module->print(ostream, nullptr);

    return true;
*/

/* LLVM OBJECT FILE
 std::unique_ptr<llvm::Module> module = action->takeModule();
    if (!module) {
      err << "error: failed to take module" << std::endl;
      return false;
    }

    if (llvm::verifyModule(*module, &err_os)) {
      err << "error: failed to verify module" << std::endl;
      return false;
    }

    module->setDataLayout(TargetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);

    ///==========================================================================

    // Create the analysis managers.
    // These must be declared in this order so that they are destroyed in the
    // correct order due to inter-analysis-manager references.
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // Create the new pass manager builder.
    // Take a look at the PassBuilder constructor parameters for more
    // customization, e.g. specifying a TargetMachine or various debugging
    // options.
    llvm::PassBuilder PB;

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

    // Optimize the IR!
    MPM.run(*module, MAM);

    ///==========================================================================

    /// FIXME: Figure out how to do this probably with streams
    std::error_code ec;
    llvm::SmallVector<char> output_buffer;
    llvm::raw_svector_ostream code_os(output_buffer);

    llvm::legacy::PassManager pass;
    TargetMachine->addPassesToEmitFile(pass, code_os, nullptr, llvm::CGFT_ObjectFile);
    if (!pass.run(*module)) {
      err << "error: failed to emit object code" << std::endl;
      return false;
    }

    code.write(output_buffer.data(), output_buffer.size());
*/

class ConvError : public std::runtime_error {
public:
  ConvError(const std::string &msg) : std::runtime_error(msg) {}
};

typedef std::function<bool(qmodule_t *, qcode_conf_t *, std::ostream &err, llvm::raw_ostream &out)>
    qcode_adapter_fn;

static bool qcode_adapter(qmodule_t *module, qcode_conf_t *conf, FILE *err, FILE *out,
                          qcode_adapter_fn impl) {
  try {
    std::unique_ptr<std::streambuf> err_stream_buf, out_stream_buf;

    {
      /* If the error stream is provided, use it. Otherwise, discard the output. */
      if (err) {
        err_stream_buf = std::make_unique<OStreamWriter>(err);
      } else {
        err_stream_buf = std::make_unique<OStreamDiscard>();
      }

      /* If the output stream is provided, use it. Otherwise, discard the output. */
      if (out) {
        out_stream_buf = std::make_unique<OStreamWriter>(out);
      } else {
        out_stream_buf = std::make_unique<OStreamDiscard>();
      }
    }

    std::ostream err_stream(err_stream_buf.get());
    std::ostream out_stream(out_stream_buf.get());

    {
      my_pwrite_ostream llvm_adapt(out_stream);
      if (!impl(module, conf, err_stream, llvm_adapt)) {
        err_stream.flush();
        out_stream.flush();
        err &&fflush(err);
        out &&fflush(out);
        return false;
      }
    }

    err_stream.flush();
    out_stream.flush();
    err &&fflush(err);
    out &&fflush(out);

    return true;
  } catch (std::exception &e) {
    fprintf(err, "%s", e.what());
    err &&fflush(err);
    out &&fflush(out);
    return false;
  }
}

static std::optional<std::unique_ptr<llvm::Module>> fabricate_llvmir(qmodule_t *module,
                                                                     qcode_conf_t *conf,
                                                                     std::ostream &err,
                                                                     llvm::raw_ostream &out);

LIB_EXPORT bool qcode_ir(qmodule_t *module, qcode_conf_t *conf, FILE *err, FILE *out) {
  return qcode_adapter(
      module, conf, err, out,
      [](qmodule_t *m, qcode_conf_t *c, std::ostream &e, llvm::raw_ostream &o) -> bool {
        auto module = fabricate_llvmir(m, c, e, o);
        if (!module) {
          e << "error: failed to fabricate LLVM IR" << std::endl;
          return false;
        }

        if (llvm::verifyModule(*module->get(), &o)) {
          e << "error: failed to verify module" << std::endl;
          return false;
        }

        module.value()->print(o, nullptr);

        return true;
      });
}

LIB_EXPORT bool qcode_asm(qmodule_t *module, qcode_conf_t *conf, FILE *err, FILE *out) {
  return qcode_adapter(
      module, conf, err, out,
      [](qmodule_t *m, qcode_conf_t *c, std::ostream &e, llvm::raw_ostream &o) -> bool {
        auto module = fabricate_llvmir(m, c, e, o);
        if (!module) {
          e << "error: failed to fabricate LLVM IR" << std::endl;
          return false;
        }

        if (llvm::verifyModule(*module->get(), &o)) {
          e << "error: failed to verify module" << std::endl;
          return false;
        }

        /// TODO: Optimization passes
        /// TODO: Assembly file generation

        return true;
      });
}

LIB_EXPORT bool qcode_obj(qmodule_t *module, qcode_conf_t *conf, FILE *err, FILE *out) {
  return qcode_adapter(
      module, conf, err, out,
      [](qmodule_t *m, qcode_conf_t *c, std::ostream &e, llvm::raw_ostream &o) -> bool {
        auto module = fabricate_llvmir(m, c, e, o);
        if (!module) {
          e << "error: failed to fabricate LLVM IR" << std::endl;
          return false;
        }

        if (llvm::verifyModule(*module->get(), &o)) {
          e << "error: failed to verify module" << std::endl;
          return false;
        }

        /// TODO: Optimization passes
        /// TODO: Object file generation

        return true;
      });
}

typedef std::unique_ptr<llvm::Module> ctx_t;
typedef std::unique_ptr<llvm::IRBuilder<>> craft_t;
typedef std::optional<llvm::Value *> val_t;
typedef std::optional<llvm::Type *> ty_t;

struct Mode {
  bool unused;
};

static val_t QIR_NODE_BINEXPR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::BinExpr *N);
static val_t QIR_NODE_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::UnExpr *N);
static val_t QIR_NODE_POST_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::PostUnExpr *N);
static val_t QIR_NODE_INT_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Int *N);
static val_t QIR_NODE_FLOAT_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Float *N);
static val_t QIR_NODE_STRING_C(ctx_t &m, craft_t &b, const Mode &s, qxir::String *N);
static val_t QIR_NODE_LIST_C(ctx_t &m, craft_t &b, const Mode &s, qxir::List *N);
static val_t QIR_NODE_CALL_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Call *N);
static val_t QIR_NODE_SEQ_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Seq *N);
static val_t QIR_NODE_INDEX_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Index *N);
static val_t QIR_NODE_EXTERN_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Extern *N);
static val_t QIR_NODE_LOCAL_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Local *N);
static val_t QIR_NODE_RET_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Ret *N);
static val_t QIR_NODE_BRK_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Brk *N);
static val_t QIR_NODE_CONT_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Cont *N);
static val_t QIR_NODE_IF_C(ctx_t &m, craft_t &b, const Mode &s, qxir::If *N);
static val_t QIR_NODE_WHILE_C(ctx_t &m, craft_t &b, const Mode &s, qxir::While *N);
static val_t QIR_NODE_FOR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::For *N);
static val_t QIR_NODE_FORM_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Form *N);
static val_t QIR_NODE_CASE_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Case *N);
static val_t QIR_NODE_SWITCH_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Switch *N);
static val_t QIR_NODE_FN_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Fn *N);
static val_t QIR_NODE_ASM_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Asm *N);
static ty_t QIR_NODE_U1_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U1Ty *N);
static ty_t QIR_NODE_U8_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U8Ty *N);
static ty_t QIR_NODE_U16_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U16Ty *N);
static ty_t QIR_NODE_U32_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U32Ty *N);
static ty_t QIR_NODE_U64_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U64Ty *N);
static ty_t QIR_NODE_U128_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U128Ty *N);
static ty_t QIR_NODE_I8_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I8Ty *N);
static ty_t QIR_NODE_I16_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I16Ty *N);
static ty_t QIR_NODE_I32_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I32Ty *N);
static ty_t QIR_NODE_I64_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I64Ty *N);
static ty_t QIR_NODE_I128_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I128Ty *N);
static ty_t QIR_NODE_F16_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F16Ty *N);
static ty_t QIR_NODE_F32_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F32Ty *N);
static ty_t QIR_NODE_F64_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F64Ty *N);
static ty_t QIR_NODE_F128_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F128Ty *N);
static ty_t QIR_NODE_VOID_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::VoidTy *N);
static ty_t QIR_NODE_PTR_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::PtrTy *N);
static ty_t QIR_NODE_OPAQUE_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::OpaqueTy *N);
static ty_t QIR_NODE_STRING_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::StringTy *N);
static ty_t QIR_NODE_STRUCT_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::StructTy *N);
static ty_t QIR_NODE_UNION_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::UnionTy *N);
static ty_t QIR_NODE_ARRAY_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::ArrayTy *N);
static ty_t QIR_NODE_FN_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::FnTy *N);

auto V(ctx_t &m, craft_t &b, const Mode &s, qxir::Expr *N) -> val_t {
  val_t R;

  switch (N->getKind()) {
    case QIR_NODE_BINEXPR: {
      R = QIR_NODE_BINEXPR_C(m, b, s, N->as<qxir::BinExpr>());
      break;
    }

    case QIR_NODE_UNEXPR: {
      R = QIR_NODE_UNEXPR_C(m, b, s, N->as<qxir::UnExpr>());
      break;
    }

    case QIR_NODE_POST_UNEXPR: {
      R = QIR_NODE_POST_UNEXPR_C(m, b, s, N->as<qxir::PostUnExpr>());
      break;
    }

    case QIR_NODE_INT: {
      R = QIR_NODE_INT_C(m, b, s, N->as<qxir::Int>());
      break;
    }

    case QIR_NODE_FLOAT: {
      R = QIR_NODE_FLOAT_C(m, b, s, N->as<qxir::Float>());
      break;
    }

    case QIR_NODE_STRING: {
      R = QIR_NODE_STRING_C(m, b, s, N->as<qxir::String>());
      break;
    }

    case QIR_NODE_LIST: {
      R = QIR_NODE_LIST_C(m, b, s, N->as<qxir::List>());
      break;
    }

    case QIR_NODE_CALL: {
      R = QIR_NODE_CALL_C(m, b, s, N->as<qxir::Call>());
      break;
    }

    case QIR_NODE_SEQ: {
      R = QIR_NODE_SEQ_C(m, b, s, N->as<qxir::Seq>());
      break;
    }

    case QIR_NODE_INDEX: {
      R = QIR_NODE_INDEX_C(m, b, s, N->as<qxir::Index>());
      break;
    }

    case QIR_NODE_EXTERN: {
      R = QIR_NODE_EXTERN_C(m, b, s, N->as<qxir::Extern>());
      break;
    }

    case QIR_NODE_LOCAL: {
      R = QIR_NODE_LOCAL_C(m, b, s, N->as<qxir::Local>());
      break;
    }

    case QIR_NODE_RET: {
      R = QIR_NODE_RET_C(m, b, s, N->as<qxir::Ret>());
      break;
    }

    case QIR_NODE_BRK: {
      R = QIR_NODE_BRK_C(m, b, s, N->as<qxir::Brk>());
      break;
    }

    case QIR_NODE_CONT: {
      R = QIR_NODE_CONT_C(m, b, s, N->as<qxir::Cont>());
      break;
    }

    case QIR_NODE_IF: {
      R = QIR_NODE_IF_C(m, b, s, N->as<qxir::If>());
      break;
    }

    case QIR_NODE_WHILE: {
      R = QIR_NODE_WHILE_C(m, b, s, N->as<qxir::While>());
      break;
    }

    case QIR_NODE_FOR: {
      R = QIR_NODE_FOR_C(m, b, s, N->as<qxir::For>());
      break;
    }

    case QIR_NODE_FORM: {
      R = QIR_NODE_FORM_C(m, b, s, N->as<qxir::Form>());
      break;
    }

    case QIR_NODE_CASE: {
      R = QIR_NODE_CASE_C(m, b, s, N->as<qxir::Case>());
      break;
    }

    case QIR_NODE_SWITCH: {
      R = QIR_NODE_SWITCH_C(m, b, s, N->as<qxir::Switch>());
      break;
    }

    case QIR_NODE_FN: {
      R = QIR_NODE_FN_C(m, b, s, N->as<qxir::Fn>());
      break;
    }

    case QIR_NODE_ASM: {
      R = QIR_NODE_ASM_C(m, b, s, N->as<qxir::Asm>());
      break;
    }

    case QIR_NODE_TMP: {
      qcore_panic("unexpected temporary node");
    }

    default: {
      throw ConvError("Expected an expression, but got: " + std::string(N->getKindName()));
    }
  }

  return R;
}

auto T(ctx_t &m, craft_t &b, const Mode &s, qxir::Expr *N) -> ty_t {
  ty_t R;

  switch (N->getKind()) {
    case QIR_NODE_U1_TY: {
      R = QIR_NODE_U1_TY_C(m, b, s, N->as<qxir::U1Ty>());
      break;
    }

    case QIR_NODE_U8_TY: {
      R = QIR_NODE_U8_TY_C(m, b, s, N->as<qxir::U8Ty>());
      break;
    }

    case QIR_NODE_U16_TY: {
      R = QIR_NODE_U16_TY_C(m, b, s, N->as<qxir::U16Ty>());
      break;
    }

    case QIR_NODE_U32_TY: {
      R = QIR_NODE_U32_TY_C(m, b, s, N->as<qxir::U32Ty>());
      break;
    }

    case QIR_NODE_U64_TY: {
      R = QIR_NODE_U64_TY_C(m, b, s, N->as<qxir::U64Ty>());
      break;
    }

    case QIR_NODE_U128_TY: {
      R = QIR_NODE_U128_TY_C(m, b, s, N->as<qxir::U128Ty>());
      break;
    }

    case QIR_NODE_I8_TY: {
      R = QIR_NODE_I8_TY_C(m, b, s, N->as<qxir::I8Ty>());
      break;
    }

    case QIR_NODE_I16_TY: {
      R = QIR_NODE_I16_TY_C(m, b, s, N->as<qxir::I16Ty>());
      break;
    }

    case QIR_NODE_I32_TY: {
      R = QIR_NODE_I32_TY_C(m, b, s, N->as<qxir::I32Ty>());
      break;
    }

    case QIR_NODE_I64_TY: {
      R = QIR_NODE_I64_TY_C(m, b, s, N->as<qxir::I64Ty>());
      break;
    }

    case QIR_NODE_I128_TY: {
      R = QIR_NODE_I128_TY_C(m, b, s, N->as<qxir::I128Ty>());
      break;
    }

    case QIR_NODE_F16_TY: {
      R = QIR_NODE_F16_TY_C(m, b, s, N->as<qxir::F16Ty>());
      break;
    }

    case QIR_NODE_F32_TY: {
      R = QIR_NODE_F32_TY_C(m, b, s, N->as<qxir::F32Ty>());
      break;
    }

    case QIR_NODE_F64_TY: {
      R = QIR_NODE_F64_TY_C(m, b, s, N->as<qxir::F64Ty>());
      break;
    }

    case QIR_NODE_F128_TY: {
      R = QIR_NODE_F128_TY_C(m, b, s, N->as<qxir::F128Ty>());
      break;
    }

    case QIR_NODE_VOID_TY: {
      R = QIR_NODE_VOID_TY_C(m, b, s, N->as<qxir::VoidTy>());
      break;
    }

    case QIR_NODE_PTR_TY: {
      R = QIR_NODE_PTR_TY_C(m, b, s, N->as<qxir::PtrTy>());
      break;
    }

    case QIR_NODE_OPAQUE_TY: {
      R = QIR_NODE_OPAQUE_TY_C(m, b, s, N->as<qxir::OpaqueTy>());
      break;
    }

    case QIR_NODE_STRING_TY: {
      R = QIR_NODE_STRING_TY_C(m, b, s, N->as<qxir::StringTy>());
      break;
    }

    case QIR_NODE_STRUCT_TY: {
      R = QIR_NODE_STRUCT_TY_C(m, b, s, N->as<qxir::StructTy>());
      break;
    }

    case QIR_NODE_UNION_TY: {
      R = QIR_NODE_UNION_TY_C(m, b, s, N->as<qxir::UnionTy>());
      break;
    }

    case QIR_NODE_ARRAY_TY: {
      R = QIR_NODE_ARRAY_TY_C(m, b, s, N->as<qxir::ArrayTy>());
      break;
    }

    case QIR_NODE_FN_TY: {
      R = QIR_NODE_FN_TY_C(m, b, s, N->as<qxir::FnTy>());
      break;
    }

    case QIR_NODE_TMP: {
      qcore_panic("unexpected temporary node");
    }

    default: {
      throw ConvError("Expected a type, but got: " + std::string(N->getKindName()));
    }
  }

  return R;
}

static std::optional<std::unique_ptr<llvm::Module>> fabricate_llvmir(qmodule_t *src,
                                                                     qcode_conf_t *c,
                                                                     std::ostream &e,
                                                                     llvm::raw_ostream &out) {
  static thread_local std::unique_ptr<llvm::LLVMContext> context;

  qxir::Expr *root = src->getRoot();
  if (!root) {
    e << "error: missing root node" << std::endl;
    return std::nullopt;
  }

  if (root->getKind() != QIR_NODE_SEQ) {
    e << "error: expected sequence node as root" << std::endl;
    return std::nullopt;
  }

  qxir::Seq *seq = root->as<qxir::Seq>();

  context = std::make_unique<llvm::LLVMContext>();
  craft_t b = std::make_unique<llvm::IRBuilder<>>(*context);
  ctx_t m = std::make_unique<llvm::Module>(src->getName(), *context);

  Mode s; /* For readonly config settings */

  for (auto &node : seq->getItems()) {
    val_t R = V(m, b, s, node);
    if (!R) {
      e << "error: failed to lower code" << std::endl;
      return std::nullopt;
    }
  }

  return m;
}

static val_t QIR_NODE_BINEXPR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::BinExpr *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::UnExpr *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_POST_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::PostUnExpr *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_INT_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Int *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_FLOAT_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Float *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_STRING_C(ctx_t &m, craft_t &b, const Mode &s, qxir::String *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_LIST_C(ctx_t &m, craft_t &b, const Mode &s, qxir::List *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_CALL_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Call *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_SEQ_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Seq *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_INDEX_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Index *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_EXTERN_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Extern *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_LOCAL_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Local *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_RET_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Ret *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_BRK_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Brk *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_CONT_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Cont *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_IF_C(ctx_t &m, craft_t &b, const Mode &s, qxir::If *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_WHILE_C(ctx_t &m, craft_t &b, const Mode &s, qxir::While *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_FOR_C(ctx_t &m, craft_t &b, const Mode &s, qxir::For *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_FORM_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Form *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_CASE_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Case *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_SWITCH_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Switch *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_FN_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Fn *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_ASM_C(ctx_t &m, craft_t &b, const Mode &s, qxir::Asm *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_U1_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U1Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_U8_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U8Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_U16_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U16Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_U32_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U32Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_U64_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U64Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_U128_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::U128Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_I8_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I8Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_I16_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I16Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_I32_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I32Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_I64_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I64Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_I128_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::I128Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_F16_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F16Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_F32_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F32Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_F64_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F64Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_F128_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::F128Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_VOID_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::VoidTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_PTR_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::PtrTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_OPAQUE_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::OpaqueTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_STRING_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::StringTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_STRUCT_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::StructTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_UNION_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::UnionTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_ARRAY_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::ArrayTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static ty_t QIR_NODE_FN_TY_C(ctx_t &m, craft_t &b, const Mode &s, qxir::FnTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}
