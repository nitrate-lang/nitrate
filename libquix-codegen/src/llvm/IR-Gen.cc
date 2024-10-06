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

#include <llvm-14/llvm/IR/Function.h>
#define __QUIX_IMPL__
#define QXIR_USE_CPP_API

#include <core/LibMacro.h>
#include <llvm-14/llvm/IR/BasicBlock.h>
#include <llvm-14/llvm/IR/DerivedTypes.h>
#include <llvm-14/llvm/IR/GlobalValue.h>
#include <llvm-14/llvm/IR/Instructions.h>
#include <llvm-14/llvm/IR/Type.h>
#include <llvm-14/llvm/IR/Value.h>
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
#include <sys/types.h>

#include <charconv>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <unordered_map>

#include "quix-qxir/TypeDecl.h"

#define debug(...) std::cerr << "[debug]: ln " << __LINE__ << ": " << __VA_ARGS__ << std::endl

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
          /// FIXME: Fail here
          // return false;
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

typedef llvm::Module ctx_t;
typedef llvm::IRBuilder<> craft_t;
typedef std::optional<llvm::Value *> val_t;
typedef std::optional<llvm::Type *> ty_t;

struct Mode {
  bool unused;
};

struct State {
  std::stack<std::pair<llvm::AllocaInst *, llvm::BasicBlock *>> return_val;
  std::stack<llvm::GlobalValue::LinkageTypes> linkage;
  std::stack<std::pair<llvm::Function *, std::unordered_map<std::string_view, llvm::AllocaInst *>>>
      locals;
  std::unordered_map<std::string_view, llvm::Function *> functions;
  struct LoopBlock {
    llvm::BasicBlock *step;
    llvm::BasicBlock *exit;
  };
  std::stack<LoopBlock> loops;

  bool in_fn;
  bool did_ret;
  bool did_brk;
  bool did_cont;

  static State defaults() {
    State s;
    s.linkage.push(llvm::GlobalValue::LinkageTypes::InternalLinkage);
    s.in_fn = false;
    s.did_ret = false;
    s.did_brk = false;
    s.did_cont = false;
    return s;
  }

  std::optional<llvm::Value *> find_named_value(std::string_view name) {
    for (const auto &[cur_name, inst] : locals.top().second) {
      if (cur_name == name) {
        return inst;
      }
    }

    if (auto it = functions.find(name); it != functions.end()) {
      return it->second;
    }

    /// TODO: Global variables.

    debug("Failed to find named value: " << name);
    return std::nullopt;
  }
};

static val_t QIR_NODE_BINEXPR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::BinExpr *N);
static val_t QIR_NODE_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::UnExpr *N);
static val_t QIR_NODE_POST_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s,
                                    qxir::PostUnExpr *N);
static val_t QIR_NODE_INT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Int *N);
static val_t QIR_NODE_FLOAT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Float *N);
static val_t QIR_NODE_STRING_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::String *N);
static val_t QIR_NODE_LIST_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::List *N);
static val_t QIR_NODE_CALL_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Call *N);
static val_t QIR_NODE_SEQ_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Seq *N);
static val_t QIR_NODE_INDEX_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Index *N);
static val_t QIR_NODE_IDENT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Ident *N);
static val_t QIR_NODE_EXTERN_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Extern *N);
static val_t QIR_NODE_LOCAL_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Local *N);
static val_t QIR_NODE_RET_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Ret *N);
static val_t QIR_NODE_BRK_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Brk *N);
static val_t QIR_NODE_CONT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Cont *N);
static val_t QIR_NODE_IF_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::If *N);
static val_t QIR_NODE_WHILE_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::While *N);
static val_t QIR_NODE_FOR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::For *N);
static val_t QIR_NODE_FORM_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Form *N);
static val_t QIR_NODE_CASE_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Case *N);
static val_t QIR_NODE_SWITCH_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Switch *N);
static val_t QIR_NODE_FN_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Fn *N);
static val_t QIR_NODE_ASM_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Asm *N);
static ty_t QIR_NODE_U1_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U1Ty *N);
static ty_t QIR_NODE_U8_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U8Ty *N);
static ty_t QIR_NODE_U16_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U16Ty *N);
static ty_t QIR_NODE_U32_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U32Ty *N);
static ty_t QIR_NODE_U64_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U64Ty *N);
static ty_t QIR_NODE_U128_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U128Ty *N);
static ty_t QIR_NODE_I8_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I8Ty *N);
static ty_t QIR_NODE_I16_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I16Ty *N);
static ty_t QIR_NODE_I32_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I32Ty *N);
static ty_t QIR_NODE_I64_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I64Ty *N);
static ty_t QIR_NODE_I128_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I128Ty *N);
static ty_t QIR_NODE_F16_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F16Ty *N);
static ty_t QIR_NODE_F32_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F32Ty *N);
static ty_t QIR_NODE_F64_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F64Ty *N);
static ty_t QIR_NODE_F128_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F128Ty *N);
static ty_t QIR_NODE_VOID_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::VoidTy *N);
static ty_t QIR_NODE_PTR_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::PtrTy *N);
static ty_t QIR_NODE_OPAQUE_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::OpaqueTy *N);
static ty_t QIR_NODE_STRING_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::StringTy *N);
static ty_t QIR_NODE_STRUCT_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::StructTy *N);
static ty_t QIR_NODE_UNION_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::UnionTy *N);
static ty_t QIR_NODE_ARRAY_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::ArrayTy *N);
static ty_t QIR_NODE_FN_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::FnTy *N);

auto V(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Expr *N) -> val_t {
  val_t R;

  switch (N->getKind()) {
    case QIR_NODE_BINEXPR: {
      R = QIR_NODE_BINEXPR_C(m, b, cf, s, N->as<qxir::BinExpr>());
      break;
    }

    case QIR_NODE_UNEXPR: {
      R = QIR_NODE_UNEXPR_C(m, b, cf, s, N->as<qxir::UnExpr>());
      break;
    }

    case QIR_NODE_POST_UNEXPR: {
      R = QIR_NODE_POST_UNEXPR_C(m, b, cf, s, N->as<qxir::PostUnExpr>());
      break;
    }

    case QIR_NODE_INT: {
      R = QIR_NODE_INT_C(m, b, cf, s, N->as<qxir::Int>());
      break;
    }

    case QIR_NODE_FLOAT: {
      R = QIR_NODE_FLOAT_C(m, b, cf, s, N->as<qxir::Float>());
      break;
    }

    case QIR_NODE_STRING: {
      R = QIR_NODE_STRING_C(m, b, cf, s, N->as<qxir::String>());
      break;
    }

    case QIR_NODE_LIST: {
      R = QIR_NODE_LIST_C(m, b, cf, s, N->as<qxir::List>());
      break;
    }

    case QIR_NODE_CALL: {
      R = QIR_NODE_CALL_C(m, b, cf, s, N->as<qxir::Call>());
      break;
    }

    case QIR_NODE_SEQ: {
      R = QIR_NODE_SEQ_C(m, b, cf, s, N->as<qxir::Seq>());
      break;
    }

    case QIR_NODE_INDEX: {
      R = QIR_NODE_INDEX_C(m, b, cf, s, N->as<qxir::Index>());
      break;
    }

    case QIR_NODE_IDENT: {
      R = QIR_NODE_IDENT_C(m, b, cf, s, N->as<qxir::Ident>());
      break;
    }

    case QIR_NODE_EXTERN: {
      R = QIR_NODE_EXTERN_C(m, b, cf, s, N->as<qxir::Extern>());
      break;
    }

    case QIR_NODE_LOCAL: {
      R = QIR_NODE_LOCAL_C(m, b, cf, s, N->as<qxir::Local>());
      break;
    }

    case QIR_NODE_RET: {
      R = QIR_NODE_RET_C(m, b, cf, s, N->as<qxir::Ret>());
      break;
    }

    case QIR_NODE_BRK: {
      R = QIR_NODE_BRK_C(m, b, cf, s, N->as<qxir::Brk>());
      break;
    }

    case QIR_NODE_CONT: {
      R = QIR_NODE_CONT_C(m, b, cf, s, N->as<qxir::Cont>());
      break;
    }

    case QIR_NODE_IF: {
      R = QIR_NODE_IF_C(m, b, cf, s, N->as<qxir::If>());
      break;
    }

    case QIR_NODE_WHILE: {
      R = QIR_NODE_WHILE_C(m, b, cf, s, N->as<qxir::While>());
      break;
    }

    case QIR_NODE_FOR: {
      R = QIR_NODE_FOR_C(m, b, cf, s, N->as<qxir::For>());
      break;
    }

    case QIR_NODE_FORM: {
      R = QIR_NODE_FORM_C(m, b, cf, s, N->as<qxir::Form>());
      break;
    }

    case QIR_NODE_CASE: {
      R = QIR_NODE_CASE_C(m, b, cf, s, N->as<qxir::Case>());
      break;
    }

    case QIR_NODE_SWITCH: {
      R = QIR_NODE_SWITCH_C(m, b, cf, s, N->as<qxir::Switch>());
      break;
    }

    case QIR_NODE_FN: {
      R = QIR_NODE_FN_C(m, b, cf, s, N->as<qxir::Fn>());
      break;
    }

    case QIR_NODE_ASM: {
      R = QIR_NODE_ASM_C(m, b, cf, s, N->as<qxir::Asm>());
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

auto T(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Expr *N) -> ty_t {
  ty_t R;

  switch (N->getKind()) {
    case QIR_NODE_U1_TY: {
      R = QIR_NODE_U1_TY_C(m, b, cf, s, N->as<qxir::U1Ty>());
      break;
    }

    case QIR_NODE_U8_TY: {
      R = QIR_NODE_U8_TY_C(m, b, cf, s, N->as<qxir::U8Ty>());
      break;
    }

    case QIR_NODE_U16_TY: {
      R = QIR_NODE_U16_TY_C(m, b, cf, s, N->as<qxir::U16Ty>());
      break;
    }

    case QIR_NODE_U32_TY: {
      R = QIR_NODE_U32_TY_C(m, b, cf, s, N->as<qxir::U32Ty>());
      break;
    }

    case QIR_NODE_U64_TY: {
      R = QIR_NODE_U64_TY_C(m, b, cf, s, N->as<qxir::U64Ty>());
      break;
    }

    case QIR_NODE_U128_TY: {
      R = QIR_NODE_U128_TY_C(m, b, cf, s, N->as<qxir::U128Ty>());
      break;
    }

    case QIR_NODE_I8_TY: {
      R = QIR_NODE_I8_TY_C(m, b, cf, s, N->as<qxir::I8Ty>());
      break;
    }

    case QIR_NODE_I16_TY: {
      R = QIR_NODE_I16_TY_C(m, b, cf, s, N->as<qxir::I16Ty>());
      break;
    }

    case QIR_NODE_I32_TY: {
      R = QIR_NODE_I32_TY_C(m, b, cf, s, N->as<qxir::I32Ty>());
      break;
    }

    case QIR_NODE_I64_TY: {
      R = QIR_NODE_I64_TY_C(m, b, cf, s, N->as<qxir::I64Ty>());
      break;
    }

    case QIR_NODE_I128_TY: {
      R = QIR_NODE_I128_TY_C(m, b, cf, s, N->as<qxir::I128Ty>());
      break;
    }

    case QIR_NODE_F16_TY: {
      R = QIR_NODE_F16_TY_C(m, b, cf, s, N->as<qxir::F16Ty>());
      break;
    }

    case QIR_NODE_F32_TY: {
      R = QIR_NODE_F32_TY_C(m, b, cf, s, N->as<qxir::F32Ty>());
      break;
    }

    case QIR_NODE_F64_TY: {
      R = QIR_NODE_F64_TY_C(m, b, cf, s, N->as<qxir::F64Ty>());
      break;
    }

    case QIR_NODE_F128_TY: {
      R = QIR_NODE_F128_TY_C(m, b, cf, s, N->as<qxir::F128Ty>());
      break;
    }

    case QIR_NODE_VOID_TY: {
      R = QIR_NODE_VOID_TY_C(m, b, cf, s, N->as<qxir::VoidTy>());
      break;
    }

    case QIR_NODE_PTR_TY: {
      R = QIR_NODE_PTR_TY_C(m, b, cf, s, N->as<qxir::PtrTy>());
      break;
    }

    case QIR_NODE_OPAQUE_TY: {
      R = QIR_NODE_OPAQUE_TY_C(m, b, cf, s, N->as<qxir::OpaqueTy>());
      break;
    }

    case QIR_NODE_STRING_TY: {
      R = QIR_NODE_STRING_TY_C(m, b, cf, s, N->as<qxir::StringTy>());
      break;
    }

    case QIR_NODE_STRUCT_TY: {
      R = QIR_NODE_STRUCT_TY_C(m, b, cf, s, N->as<qxir::StructTy>());
      break;
    }

    case QIR_NODE_UNION_TY: {
      R = QIR_NODE_UNION_TY_C(m, b, cf, s, N->as<qxir::UnionTy>());
      break;
    }

    case QIR_NODE_ARRAY_TY: {
      R = QIR_NODE_ARRAY_TY_C(m, b, cf, s, N->as<qxir::ArrayTy>());
      break;
    }

    case QIR_NODE_FN_TY: {
      R = QIR_NODE_FN_TY_C(m, b, cf, s, N->as<qxir::FnTy>());
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
  std::unique_ptr<llvm::IRBuilder<>> b = std::make_unique<llvm::IRBuilder<>>(*context);
  std::unique_ptr<llvm::Module> m = std::make_unique<llvm::Module>(src->getName(), *context);

  Mode cf; /* For readonly config settings */
  State s = State::defaults();

  for (auto &node : seq->getItems()) {
    val_t R = V(*m, *b, cf, s, node);
    if (!R) {
      e << "error: failed to lower code" << std::endl;
      return std::nullopt;
    }
  }

  return m;
}

static val_t binexpr_do_cast(ctx_t &m, craft_t &b, const Mode &cf, State &s, llvm::Value *L,
                             qxir::Op O, llvm::Type *R, qxir::Type *LT, qxir::Type *RT) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  val_t E;

  if (LT->cmp_eq(RT)) {
    return L;
  }

  switch (O) {
    case qxir::Op::BitcastAs: {
      if (LT->is_pointer() && RT->is_integral()) {
        E = b.CreatePtrToInt(L, R);
      } else if (LT->is_integral() && RT->is_pointer()) {
        E = b.CreateIntToPtr(L, R);
      } else {
        E = b.CreateBitCast(L, R);
      }
      break;
    }

    case qxir::Op::CastAs: {
      llvm::Type *IR_LT = L->getType();
      /* Handle floating point */
      if (IR_LT->isFloatingPointTy() && RT->is_signed()) {
        E = b.CreateFPToSI(L, R);
      } else if (IR_LT->isFloatingPointTy() && RT->is_unsigned()) {
        E = b.CreateFPToUI(L, R);
      } else if (LT->is_signed() && RT->is_floating_point()) {
        E = b.CreateSIToFP(L, R);
      } else if (LT->is_unsigned() && RT->is_floating_point()) {
        E = b.CreateUIToFP(L, R);
      }

      /* Integer stuff */
      else if (LT->is_signed() && RT->is_signed()) {
        E = b.CreateSExtOrTrunc(L, R);
      } else if (LT->is_unsigned() && RT->is_signed()) {
        E = b.CreateZExtOrTrunc(L, R);
      } else if (LT->is_signed() && RT->is_unsigned()) {
        E = b.CreateSExtOrTrunc(L, R);
      } else if (LT->is_unsigned() && RT->is_unsigned()) {
        E = b.CreateZExtOrTrunc(L, R);
      }

      /* Composite casting */
      else if (LT->is_array() && RT->getKind() == QIR_NODE_STRUCT_TY) {
        qxir::ArrayTy *base = LT->as<qxir::ArrayTy>();
        qxir::StructTy *ST = RT->as<qxir::StructTy>();

        if (base->getCount() == ST->getFields().size()) {
          llvm::StructType *new_st_ty = llvm::cast<llvm::StructType>(R);
          llvm::AllocaInst *new_st = b.CreateAlloca(new_st_ty);

          for (size_t i = 0; i < ST->getFields().size(); i++) {
            std::cout << "Casting element " << i << std::endl;
            val_t F = binexpr_do_cast(m, b, cf, s, b.CreateExtractValue(L, i), qxir::Op::CastAs,
                                      new_st_ty->getElementType(i), base->getElement()->getType(),
                                      ST->getFields()[i]);
            if (!F) {
              debug("Failed to cast element " << i);
              return std::nullopt;
            }

            b.CreateStore(F.value(), b.CreateStructGEP(new_st_ty, new_st, i));
          }

          E = b.CreateLoad(new_st->getType()->getPointerElementType(), new_st);
        }
      } else if (LT->is_array() && RT->is_array()) {
        qxir::ArrayTy *FROM = LT->as<qxir::ArrayTy>();
        qxir::ArrayTy *TO = RT->as<qxir::ArrayTy>();

        if (FROM->getCount() == TO->getCount()) {
          llvm::ArrayType *new_arr_ty = llvm::cast<llvm::ArrayType>(R);
          llvm::AllocaInst *new_arr = b.CreateAlloca(new_arr_ty);

          for (size_t i = 0; i < FROM->getCount(); i++) {
            val_t F = binexpr_do_cast(m, b, cf, s, b.CreateExtractValue(L, i), qxir::Op::CastAs,
                                      new_arr_ty->getElementType(), FROM->getElement()->getType(),
                                      TO->getElement()->getType());
            if (!F) {
              debug("Failed to cast element " << i);
              return std::nullopt;
            }

            b.CreateStore(F.value(), b.CreateStructGEP(new_arr_ty, new_arr, i));
          }

          E = b.CreateLoad(new_arr->getType()->getPointerElementType(), new_arr);
        }
      } else if (LT->is_string() && RT->is_ptr_to(qxir::create<qxir::I8Ty>())) {
        // Get pointer to start of L
        llvm::Type *ty = llvm::cast<llvm::PointerType>(L->getType())->getPointerElementType();
        llvm::Value *ptr = b.CreateGEP(ty, L, {b.getInt32(0), b.getInt32(0)});

        // Cast to i8*
        E = b.CreateBitCast(ptr, R);
      } else {
        std::cout << "Failed to cast from " << LT->getKindName() << " to " << RT->getKindName()
                  << std::endl;
      }
      break;
    }

    default: {
      qcore_panic("unexpected binary operator");
    }
  }

  return E;
}

static val_t QIR_NODE_BINEXPR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::BinExpr *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

#define PROD_LHS()                       \
  val_t L = V(m, b, cf, s, N->getLHS()); \
  if (!L) {                              \
    debug("Failed to get LHS");          \
    return std::nullopt;                 \
  }

  qxir::Op O = N->getOp();

  if (N->getRHS()->isType()) { /* Do casting */
    PROD_LHS()

    ty_t TY = T(m, b, cf, s, N->getRHS()->asType());
    if (!TY) {
      debug("Failed to get RHS type");
      return std::nullopt;
    }

    qxir::Type *L_T = N->getLHS()->getType();
    if (!L_T) {
      debug("Failed to get LHS type");
      return std::nullopt;
    }

    qxir::Type *R_T = N->getRHS()->getType();
    if (!R_T) {
      debug("Failed to get RHS type");
      return std::nullopt;
    }

    return binexpr_do_cast(m, b, cf, s, L.value(), O, TY.value(), L_T, R_T);
  }

  val_t R = V(m, b, cf, s, N->getRHS());

  val_t E;

  switch (O) {
    case qxir::Op::Plus: { /* '+': Addition operator */
      PROD_LHS()
      E = b.CreateAdd(L.value(), R.value());
      break;
    }

    case qxir::Op::Minus: { /* '-': Subtraction operator */
      PROD_LHS()
      E = b.CreateSub(L.value(), R.value());
      break;
    }

    case qxir::Op::Times: { /* '*': Multiplication operator */
      PROD_LHS()
      E = b.CreateMul(L.value(), R.value());
      break;
    }

    case qxir::Op::Slash: { /* '/': Division operator */
      PROD_LHS()
      if (N->getLHS()->getType()->is_signed()) {
        E = b.CreateSDiv(L.value(), R.value());
      } else if (N->getLHS()->getType()->is_unsigned()) {
        E = b.CreateUDiv(L.value(), R.value());
      } else {
        qcore_panic("unexpected type for division");
      }
      break;
    }

    case qxir::Op::Percent: { /* '%': Modulus operator */
      PROD_LHS()
      if (N->getLHS()->getType()->is_signed()) {
        E = b.CreateSRem(L.value(), R.value());
      } else if (N->getLHS()->getType()->is_unsigned()) {
        E = b.CreateURem(L.value(), R.value());
      } else {
        qcore_panic("unexpected type for modulus");
      }
      break;
    }

    case qxir::Op::BitAnd: { /* '&': Bitwise AND operator */
      PROD_LHS()
      E = b.CreateAnd(L.value(), R.value());
      break;
    }

    case qxir::Op::BitOr: { /* '|': Bitwise OR operator */
      PROD_LHS()
      E = b.CreateOr(L.value(), R.value());
      break;
    }

    case qxir::Op::BitXor: { /* '^': Bitwise XOR operator */
      PROD_LHS()
      E = b.CreateXor(L.value(), R.value());
      break;
    }

    case qxir::Op::LogicAnd: { /* '&&': Logical AND operator */
      PROD_LHS()
      E = b.CreateLogicalAnd(L.value(), R.value());
      break;
    }

    case qxir::Op::LogicOr: { /* '||': Logical OR operator */
      PROD_LHS()
      E = b.CreateLogicalOr(L.value(), R.value());
      break;
    }

    case qxir::Op::LShift: { /* '<<': Left shift operator */
      PROD_LHS()
      E = b.CreateShl(L.value(), R.value());
      break;
    }

    case qxir::Op::RShift: { /* '>>': Right shift operator */
      PROD_LHS()
      if (N->getLHS()->getType()->is_signed()) {
        E = b.CreateAShr(L.value(), R.value());
      } else if (N->getLHS()->getType()->is_unsigned()) {
        E = b.CreateLShr(L.value(), R.value());
      } else {
        qcore_panic("unexpected type for shift");
      }
      break;
    }

    case qxir::Op::ROTR: { /* '>>>': Rotate right operator */
      PROD_LHS()
      // Formula: (L >> (R % num_bits)) | (L << ((num_bits - R) % num_bits))

      size_t num_bits = N->getLHS()->getType()->getSizeBits();
      llvm::ConstantInt *num_bits_c =
          llvm::ConstantInt::get(m.getContext(), llvm::APInt(num_bits, num_bits));

      llvm::Value *n = b.CreateURem(R.value(), num_bits_c);
      llvm::Value *lshift = b.CreateLShr(L.value(), n);
      llvm::Value *sub = b.CreateSub(num_bits_c, R.value());
      llvm::Value *rshift = b.CreateShl(L.value(), b.CreateURem(sub, num_bits_c));

      E = b.CreateOr(lshift, rshift);
      break;
    }

    case qxir::Op::ROTL: { /* '<<<': Rotate left operator */
      PROD_LHS()
      // Formula: (L << (R % num_bits)) | (L >> ((num_bits - R) % num_bits))

      size_t num_bits = N->getLHS()->getType()->getSizeBits();
      llvm::ConstantInt *num_bits_c =
          llvm::ConstantInt::get(m.getContext(), llvm::APInt(num_bits, num_bits));

      llvm::Value *n = b.CreateURem(R.value(), num_bits_c);
      llvm::Value *lshift = b.CreateShl(L.value(), n);
      llvm::Value *sub = b.CreateSub(num_bits_c, R.value());
      llvm::Value *rshift = b.CreateLShr(L.value(), b.CreateURem(sub, num_bits_c));

      E = b.CreateOr(lshift, rshift);
      break;
    }

    case qxir::Op::Set: { /* '=': Assignment operator */
      if (N->getLHS()->getKind() != QIR_NODE_IDENT) {
        qcore_panic("expected identifier for assignment");
      }

      qxir::Ident *I = N->getLHS()->as<qxir::Ident>();
      auto find = s.find_named_value(I->getName());
      if (!find) {
        qcore_panic("failed to find named value");
      }

      b.CreateStore(R.value(), find.value());

      E = R;
      break;
    }

    case qxir::Op::LT: { /* '<': Less than operator */
      PROD_LHS()
      if (N->getLHS()->getType()->is_signed()) {
        E = b.CreateICmpSLT(L.value(), R.value());
      } else if (N->getLHS()->getType()->is_unsigned()) {
        E = b.CreateICmpULT(L.value(), R.value());
      } else {
        qcore_panic("unexpected type for comparison");
      }
      break;
    }

    case qxir::Op::GT: { /* '>': Greater than operator */
      PROD_LHS()
      if (N->getLHS()->getType()->is_signed()) {
        E = b.CreateICmpSGT(L.value(), R.value());
      } else if (N->getLHS()->getType()->is_unsigned()) {
        E = b.CreateICmpUGT(L.value(), R.value());
      } else {
        qcore_panic("unexpected type for comparison");
      }
      break;
    }

    case qxir::Op::LE: { /* '<=': Less than or equal to operator */
      PROD_LHS()
      if (N->getLHS()->getType()->is_signed()) {
        E = b.CreateICmpSLE(L.value(), R.value());
      } else if (N->getLHS()->getType()->is_unsigned()) {
        E = b.CreateICmpULE(L.value(), R.value());
      } else {
        qcore_panic("unexpected type for comparison");
      }
      break;
    }

    case qxir::Op::GE: { /* '>=': Greater than or equal to operator */
      PROD_LHS()
      if (N->getLHS()->getType()->is_signed()) {
        E = b.CreateICmpSGE(L.value(), R.value());
      } else if (N->getLHS()->getType()->is_unsigned()) {
        E = b.CreateICmpUGE(L.value(), R.value());
      } else {
        qcore_panic("unexpected type for comparison");
      }
      break;
    }

    case qxir::Op::Eq: { /* '==': Equal to operator */
      PROD_LHS()
      E = b.CreateICmpEQ(L.value(), R.value());
      break;
    }

    case qxir::Op::NE: { /* '!=': Not equal to operator */
      PROD_LHS()
      E = b.CreateICmpNE(L.value(), R.value());
      break;
    }

    default: {
      qcore_panic("unexpected binary operator");
    }
  }

  return E;
}

/*
case qOpPlus: {
      return STD_UNOP(Plus);
    }
    case qOpMinus: {
      return STD_UNOP(Minus);
    }
    case qOpTimes: {
      return STD_UNOP(Times);
    }
    case qOpBitAnd: {
      return STD_UNOP(BitAnd);
    }
    case qOpBitNot: {
      return STD_UNOP(BitNot);
    }

    case qOpLogicNot: {
      return STD_UNOP(LogicNot);
    }
    case qOpInc: {
      return STD_UNOP(Inc);
    }
    case qOpDec: {
      return STD_UNOP(Dec);
    }
    case qOpSizeof: {
      auto bits = qxir::create<qxir::UnExpr>(rhs, qxir::Op::Bitsizeof);
      auto arg = qxir::create<qxir::BinExpr>(bits, qxir::create<qxir::Float>(8), qxir::Op::Slash);
      return create_simple_call(s, "std::ceil", {{"0", arg}});
    }
    case qOpAlignof: {
      return STD_UNOP(Alignof);
    }
    case qOpTypeof: {
      return create_simple_call(s, "__detail::type_of", {{"0", rhs}});
    }
    case qOpOffsetof: {
      return STD_UNOP(Offsetof);
    }
    case qOpBitsizeof: {
      return STD_UNOP(Bitsizeof);
    }
*/
static val_t QIR_NODE_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::UnExpr *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

#define PROD_SUB()                        \
  val_t E = V(m, b, cf, s, N->getExpr()); \
  if (!E) {                               \
    debug("Failed to get expression");    \
    return std::nullopt;                  \
  }

  val_t R;

  switch (N->getOp()) {
    case qxir::Op::Plus: {
      PROD_SUB();
      R = E;
      break;
    }
    case qxir::Op::Minus: {
      PROD_SUB();
      R = b.CreateNeg(E.value());
      break;
    }
    case qxir::Op::Times: {
      /// TODO: Dereference
      break;
    }
    case qxir::Op::BitAnd: {
      if (N->getExpr()->getKind() != QIR_NODE_IDENT) {
        qcore_panic("expected identifier for address_of");
      }

      qxir::Ident *I = N->getExpr()->as<qxir::Ident>();
      auto find = s.find_named_value(I->getName());
      if (!find) {
        qcore_panic("failed to find identifier for address_of");
      }

      llvm::Value *V = find.value();
      if (!V->getType()->isPointerTy()) {
        qcore_panic("expected pointer type for address_of");
      }

      R = V;
      break;
    }
    case qxir::Op::BitNot: {
      PROD_SUB();
      R = b.CreateNot(E.value());
      break;
    }
    case qxir::Op::LogicNot: {
      PROD_SUB();
      R = b.CreateICmpEQ(E.value(), llvm::ConstantInt::get(m.getContext(), llvm::APInt(1, 0)));
      break;
    }
    case qxir::Op::Inc: {
      if (N->getExpr()->getKind() != QIR_NODE_IDENT) {
        qcore_panic("expected identifier for increment");
      }

      qxir::Ident *I = N->getExpr()->as<qxir::Ident>();
      auto find = s.find_named_value(I->getName());
      if (!find) {
        qcore_panic("failed to find identifier for increment");
      }

      llvm::Value *V = find.value();

      if (!V->getType()->isPointerTy()) {
        qcore_panic("expected pointer type for increment");
      }

      llvm::Value *current = b.CreateLoad(V->getType()->getPointerElementType(), V);
      llvm::Value *new_val;

      if (current->getType()->isIntegerTy()) {
        new_val = b.CreateAdd(
            current, llvm::ConstantInt::get(
                         m.getContext(), llvm::APInt(N->getExpr()->getType()->getSizeBits(), 1)));
      } else if (current->getType()->isFloatingPointTy()) {
        new_val = b.CreateFAdd(current, llvm::ConstantFP::get(m.getContext(), llvm::APFloat(1.0)));
      } else {
        qcore_panic("unexpected type for increment");
      }

      b.CreateStore(new_val, V);

      R = new_val;
      break;
    }
    case qxir::Op::Dec: {
      if (N->getExpr()->getKind() != QIR_NODE_IDENT) {
        qcore_panic("expected identifier for decrement");
      }

      qxir::Ident *I = N->getExpr()->as<qxir::Ident>();
      auto find = s.find_named_value(I->getName());
      if (!find) {
        qcore_panic("failed to find identifier for decrement");
      }

      llvm::Value *V = find.value();

      if (!V->getType()->isPointerTy()) {
        qcore_panic("expected pointer type for decrement");
      }

      llvm::Value *current = b.CreateLoad(V->getType()->getPointerElementType(), V);
      llvm::Value *new_val;

      if (current->getType()->isIntegerTy()) {
        new_val = b.CreateSub(
            current, llvm::ConstantInt::get(
                         m.getContext(), llvm::APInt(N->getExpr()->getType()->getSizeBits(), 1)));
      } else if (current->getType()->isFloatingPointTy()) {
        new_val = b.CreateFSub(current, llvm::ConstantFP::get(m.getContext(), llvm::APFloat(1.0)));
      } else {
        qcore_panic("unexpected type for decrement");
      }

      b.CreateStore(new_val, V);

      R = new_val;
      break;
    }
    case qxir::Op::Alignof: {
      R = llvm::ConstantInt::get(m.getContext(),
                                 llvm::APInt(64, N->getExpr()->getType()->getAlignBytes()));
      break;
    }
    case qxir::Op::Typeof: {
      /// TODO: Typeof
      break;
    }
    case qxir::Op::Offsetof: {
      /// TODO: Offsetof
      break;
    }
    case qxir::Op::Bitsizeof: {
      R = llvm::ConstantInt::get(m.getContext(),
                                 llvm::APInt(64, N->getExpr()->getType()->getSizeBits()));
      break;
    }

    default: {
      qcore_panic("unexpected unary operator");
    }
  }

#undef PROD_SUB

  return R;
}

static val_t QIR_NODE_POST_UNEXPR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s,
                                    qxir::PostUnExpr *N) {
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

static val_t QIR_NODE_INT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Int *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  llvm::ConstantInt *R;
  uint64_t lit;

  if (N->isNativeRepresentation()) {
    static_assert(sizeof(void *) == sizeof(uint64_t), "This code expects 64-bit architecture");

    lit = N->getNativeRepresentation();
    goto fold_lit;
  } else {
    std::string_view lit_istr = N->getStringRepresentation();

    if (lit_istr.size() < 20) { /* Fits in 64-bit */
      lit = 0;
      std::from_chars(lit_istr.data(), lit_istr.data() + lit_istr.size(), lit);
      goto fold_lit;
    } else {
      unsigned __int128 lit128 = 0;

      while (!lit_istr.empty()) {
        lit128 = lit128 * 10 + (lit_istr[0] - '0');
        lit_istr.remove_prefix(1);
      }

      R = llvm::ConstantInt::get(m.getContext(), llvm::APInt(128, lit128));
    }
  }

  return R;

fold_lit:
  if (lit == 0) {
    return llvm::ConstantInt::get(m.getContext(), llvm::APInt(32, 0));
  }

  uint8_t l2 = std::log2(lit);

  if (l2 <= 32) {
    R = llvm::ConstantInt::get(m.getContext(), llvm::APInt(32, lit));
  } else {
    R = llvm::ConstantInt::get(m.getContext(), llvm::APInt(64, lit));
  }

  return R;
}

static val_t QIR_NODE_FLOAT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Float *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  llvm::ConstantFP *R;

  if (N->isNativeRepresentation()) {
    double lit = N->getNativeRepresentation();
    R = llvm::ConstantFP::get(m.getContext(), llvm::APFloat(lit));
  } else {
    std::string_view lit_fstr = N->getStringRepresentation();
    double lit;

    std::from_chars(lit_fstr.data(), lit_fstr.data() + lit_fstr.size(), lit);
    R = llvm::ConstantFP::get(m.getContext(), llvm::APFloat(lit));
  }

  return R;
}

static val_t QIR_NODE_STRING_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::String *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return b.CreateGlobalString(N->getValue(), "", 0, &m);
}

static val_t QIR_NODE_LIST_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::List *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  if (N->getItems().empty()) {
    debug("Empty list");
    return std::nullopt;
  }

  std::vector<llvm::Value *> items;
  items.reserve(N->getItems().size());

  for (auto &node : N->getItems()) {
    val_t R = V(m, b, cf, s, node);
    if (!R) {
      debug("Failed to get item");
      return std::nullopt;
    }

    items.push_back(R.value());
  }

  bool is_homogeneous = std::all_of(items.begin(), items.end(), [&](llvm::Value *V) {
    return V->getType() == items[0]->getType();
  });

  if (is_homogeneous) {  // It's a Basic Array
    if (N->isConstExpr()) {
      llvm::ArrayType *AT = llvm::ArrayType::get(items[0]->getType(), items.size());
      llvm::ArrayRef<llvm::Constant *> items_ref(reinterpret_cast<llvm::Constant **>(items.data()),
                                                 items.size());

      return llvm::ConstantArray::get(AT, items_ref);
    } else {
      qcore_implement("non-const array");
    }
  } else {  // It's an implicit struct value
    if (N->isConstExpr()) {
      qcore_implement("const struct");
    } else {
      qcore_implement("non-const struct");
    }
  }

  /// TODO: Implement conversion for node
  qcore_implement(__func__);
}

static val_t QIR_NODE_CALL_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Call *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  /* Direct call */
  if (!N->getTarget()->getName().empty()) {
    std::string_view fn_name = N->getTarget()->getName();
    auto find = s.find_named_value(fn_name);
    if (!find) {
      debug("Failed to find function " << fn_name);
      return std::nullopt;
    }

    llvm::Function *func_def = llvm::cast<llvm::Function>(find.value());
    llvm::FunctionType *FT = func_def->getFunctionType();

    std::vector<llvm::Value *> args;

    { /* Arguments */
      args.reserve(N->getArgs().size());

      for (auto &node : N->getArgs()) {
        val_t R = V(m, b, cf, s, node);
        if (!R) {
          debug("Failed to get argument");
          return std::nullopt;
        }

        args.push_back(R.value());
      }
    }

    { /* Verify call */
      if (!(FT->getNumParams() == args.size() ||
            (FT->isVarArg() && FT->getNumParams() <= args.size()))) {
        debug("Expected " << FT->getNumParams() << " arguments, but got " << args.size());
        return std::nullopt;
      }
    }

    return b.CreateCall(func_def, args);
  }
  /* Indirect call */
  else {
    /// TODO: Implement conversion for node
    qcore_implement(__func__);
  }
}

static val_t QIR_NODE_SEQ_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Seq *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  if (N->getItems().empty()) {
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(m.getContext()));
  }

  val_t R;

  for (auto &node : N->getItems()) {
    R = V(m, b, cf, s, node);
    if (!R) {
      debug("Failed to get item");
      return std::nullopt;
    }
  }

  return R;
}

static val_t QIR_NODE_INDEX_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Index *N) {
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

static val_t QIR_NODE_IDENT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Ident *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  if (s.in_fn) {
    auto &locals = s.locals.top().second;
    auto it = locals.find(N->getName());
    if (it == locals.end()) {
      debug("Failed to find local " << N->getName());
      return std::nullopt;
    }

    /// FIXME: ???
    return b.CreateLoad(it->second->getAllocatedType(), it->second);
  } else {
    auto it = m.getNamedValue(N->getName());
    if (!it) {
      debug("Failed to find global " << N->getName());
      return std::nullopt;
    }

    return it;
  }
}

static val_t QIR_NODE_EXTERN_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Extern *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  s.linkage.push(llvm::GlobalValue::ExternalLinkage);

  val_t R = V(m, b, cf, s, N->getValue());
  if (!R) {
    s.linkage.pop();
    debug("Failed to get value");
    return std::nullopt;
  }

  s.linkage.pop();

  return R;
}

static val_t QIR_NODE_LOCAL_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Local *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  //  N->getN

  if (s.in_fn) {
    val_t R = V(m, b, cf, s, N->getValue());
    if (!R) {
      debug("Failed to get value");
      return std::nullopt;
    }

    ty_t R_T = T(m, b, cf, s, N->getValue()->getType());
    if (!R_T) {
      debug("Failed to get type");
      return std::nullopt;
    }

    llvm::AllocaInst *local = b.CreateAlloca(R_T.value(), nullptr, N->getName());
    b.CreateStore(R.value(), local);

    s.locals.top().second.emplace(N->getName(), local);

    return local;
  } else {
    if (N->getValue()->getKind() == QIR_NODE_FN_TY) {
      qxir::FnTy *FT = N->getValue()->as<qxir::FnTy>();
      llvm::FunctionType *F_T = llvm::cast<llvm::FunctionType>(T(m, b, cf, s, FT).value());

      llvm::Function *F = llvm::Function::Create(F_T, s.linkage.top(), N->getName(), &m);

      s.functions.emplace(N->getName(), F);
      return F;
    } else {
      qcore_implement("global variables");
    }
    // /*
    // GlobalVariable(Type *Ty, bool isConstExprant, LinkageTypes Linkage, Constant *Initializer =
    // nullptr, const Twine &Name = "", ThreadLocalMode = NotThreadLocal, unsigned int AddressSpace
    // = 0, bool isExternallyInitialized = false)*/
    //     llvm::GlobalVariable *global = new llvm::GlobalVariable(R_T.value(), false,
    //     s.linkage.top(), R.value(), N->getName());

    // return global;

    qcore_implement("global variables");
  }
}

static val_t QIR_NODE_RET_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Ret *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  val_t R;

  if (N->getExpr()->getKind() != QIR_NODE_VOID_TY) {
    R = V(m, b, cf, s, N->getExpr());
    if (!R) {
      debug("Failed to get return value");
      return std::nullopt;
    }

    b.CreateStore(R.value(), s.return_val.top().first);
  } else {
    R = llvm::Constant::getNullValue(llvm::Type::getInt32Ty(m.getContext()));
  }

  b.CreateBr(s.return_val.top().second);
  s.did_ret = true;

  return R;
}

static val_t QIR_NODE_BRK_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Brk *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  qcore_assert(!s.loops.empty(), "break statement outside of loop?");

  s.did_brk = true;
  return b.CreateBr(s.loops.top().exit);
}

static val_t QIR_NODE_CONT_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Cont *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  qcore_assert(!s.loops.empty(), "continue statement outside of loop?");

  s.did_cont = true;
  return b.CreateBr(s.loops.top().step);
}

static val_t QIR_NODE_IF_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::If *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  llvm::BasicBlock *then, *els, *end;

  then = llvm::BasicBlock::Create(m.getContext(), "then", s.locals.top().first);
  els = llvm::BasicBlock::Create(m.getContext(), "else", s.locals.top().first);
  end = llvm::BasicBlock::Create(m.getContext(), "end", s.locals.top().first);

  val_t R = V(m, b, cf, s, N->getCond());
  if (!R) {
    debug("Failed to get condition");
    return std::nullopt;
  }

  b.CreateCondBr(R.value(), then, els);
  b.SetInsertPoint(then);

  bool old_did_ret = s.did_ret;
  val_t R_T = V(m, b, cf, s, N->getThen());
  if (!R_T) {
    debug("Failed to get then");
    return std::nullopt;
  }

  if (!s.did_ret) {
    b.CreateBr(end);
  }
  s.did_ret = old_did_ret;

  b.SetInsertPoint(els);

  s.did_ret = false;
  if (N->getElse()->getKind() != QIR_NODE_VOID_TY) {
    val_t R_E = V(m, b, cf, s, N->getElse());
    if (!R_E) {
      debug("Failed to get else");
      return std::nullopt;
    }
  }
  if (!s.did_ret) {
    b.CreateBr(end);
  }
  s.did_ret = old_did_ret;
  b.SetInsertPoint(end);

  return end;
}

static val_t QIR_NODE_WHILE_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::While *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  llvm::BasicBlock *begin, *body, *end;

  begin = llvm::BasicBlock::Create(m.getContext(), "begin", s.locals.top().first);
  body = llvm::BasicBlock::Create(m.getContext(), "body", s.locals.top().first);
  end = llvm::BasicBlock::Create(m.getContext(), "end", s.locals.top().first);

  b.CreateBr(begin);
  b.SetInsertPoint(begin);

  val_t R = V(m, b, cf, s, N->getCond());
  if (!R) {
    debug("Failed to get condition");
    return std::nullopt;
  }

  b.CreateCondBr(R.value(), body, end);
  b.SetInsertPoint(body);
  s.loops.push({body, end});

  bool did_brk = s.did_brk;
  bool did_cont = s.did_cont;
  bool old_ret = s.did_ret;

  val_t R_B = V(m, b, cf, s, N->getBody());
  if (!R_B) {
    debug("Failed to get body");
    return std::nullopt;
  }

  if (!s.did_brk && !s.did_cont && !s.did_ret) {
    b.CreateBr(begin);
  }
  s.did_brk = did_brk;
  s.did_cont = did_cont;
  s.did_ret = old_ret;

  s.loops.pop();

  b.SetInsertPoint(end);

  return end;
}

static val_t QIR_NODE_FOR_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::For *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  llvm::BasicBlock *begin, *body, *step, *end;

  begin = llvm::BasicBlock::Create(m.getContext(), "begin", s.locals.top().first);
  body = llvm::BasicBlock::Create(m.getContext(), "body", s.locals.top().first);
  step = llvm::BasicBlock::Create(m.getContext(), "step", s.locals.top().first);
  end = llvm::BasicBlock::Create(m.getContext(), "end", s.locals.top().first);

  val_t R = V(m, b, cf, s, N->getInit());
  if (!R) {
    debug("Failed to get init");
    return std::nullopt;
  }

  b.CreateBr(begin);
  b.SetInsertPoint(begin);

  val_t R_C = V(m, b, cf, s, N->getCond());
  if (!R_C) {
    debug("Failed to get condition");
    return std::nullopt;
  }

  b.CreateCondBr(R_C.value(), body, end);
  b.SetInsertPoint(body);
  s.loops.push({begin, end});

  bool did_brk = s.did_brk;
  bool did_cont = s.did_cont;
  bool old_ret = s.did_ret;

  val_t R_B = V(m, b, cf, s, N->getBody());
  if (!R_B) {
    debug("Failed to get body");
    return std::nullopt;
  }

  if (!s.did_brk && !s.did_cont && !s.did_ret) {
    b.CreateBr(step);
  }
  s.did_brk = did_brk;
  s.did_cont = did_cont;
  s.did_ret = old_ret;

  b.SetInsertPoint(step);

  val_t R_S = V(m, b, cf, s, N->getStep());
  if (!R_S) {
    debug("Failed to get step");
    return std::nullopt;
  }

  b.CreateBr(begin);

  s.loops.pop();

  b.SetInsertPoint(end);

  return end;
}

static val_t QIR_NODE_FORM_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Form *N) {
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

static val_t QIR_NODE_CASE_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Case *N) {
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

static val_t QIR_NODE_SWITCH_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Switch *N) {
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

static val_t QIR_NODE_FN_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Fn *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  bool in_fn_old = s.in_fn;
  s.in_fn = true;

  std::vector<llvm::Type *> params;

  { /* Lower parameter types */
    params.reserve(N->getParams().size());

    for (auto &param : N->getParams()) {
      ty_t R = T(m, b, cf, s, param.first);
      if (!R) {
        s.in_fn = in_fn_old;
        debug("Failed to get parameter type");
        return std::nullopt;
      }

      params.push_back(R.value());
    }
  }

  llvm::Type *ret_ty;

  { /* Lower return type */
    // Use type inference to get return type
    ty_t R = T(m, b, cf, s, N->getType()->as<qxir::FnTy>()->getReturn());
    if (!R) {
      s.in_fn = in_fn_old;
      debug("Failed to get return type");
      return std::nullopt;
    }

    ret_ty = R.value();
  }

  llvm::FunctionType *fn_ty = llvm::FunctionType::get(ret_ty, params, N->isVariadic());

  llvm::Function *fn = llvm::Function::Create(fn_ty, s.linkage.top(), N->getName(), &m);
  s.locals.push({fn, {}});
  s.functions[N->getName()] = fn;

  { /* Lower function body */
    llvm::BasicBlock *entry, *exit;

    entry = llvm::BasicBlock::Create(m.getContext(), "entry", fn);
    exit = llvm::BasicBlock::Create(m.getContext(), "end", fn);

    b.SetInsertPoint(entry);

    llvm::AllocaInst *ret_val_alloc = nullptr;
    if (!ret_ty->isVoidTy()) {
      ret_val_alloc = b.CreateAlloca(ret_ty, nullptr, "__ret");
    }
    s.return_val.push({ret_val_alloc, exit});

    {
      b.SetInsertPoint(exit);
      if (!ret_ty->isVoidTy()) {
        llvm::LoadInst *ret_val = b.CreateLoad(ret_ty, s.return_val.top().first);
        b.CreateRet(ret_val);
      } else {
        b.CreateRetVoid();
      }
    }

    b.SetInsertPoint(entry);

    for (size_t i = 0; i < N->getParams().size(); i++) {
      fn->getArg(i)->setName(N->getParams()[i].second);

      std::string ploc_name = "_Q" + std::string(N->getParams()[i].second);
      llvm::AllocaInst *param_alloc = b.CreateAlloca(fn->getArg(i)->getType(), nullptr, ploc_name);

      b.CreateStore(fn->getArg(i), param_alloc);
      s.locals.top().second.emplace(N->getParams()[i].second, param_alloc);
    }

    bool old_did_ret = s.did_ret;

    for (auto &node : N->getBody()->getItems()) {
      val_t R = V(m, b, cf, s, node);
      if (!R) {
        s.return_val.pop();
        s.in_fn = in_fn_old;
        s.locals.pop();
        debug("Failed to get body");
        return std::nullopt;
      }
    }

    if (!s.did_ret) {
      b.CreateBr(exit);
    }
    s.did_ret = old_did_ret;

    s.return_val.pop();
    s.in_fn = in_fn_old;
    s.locals.pop();
  }

  return fn;
}

static val_t QIR_NODE_ASM_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::Asm *N) {
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

static ty_t QIR_NODE_U1_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U1Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt1Ty(m.getContext());
}

static ty_t QIR_NODE_U8_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U8Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt8Ty(m.getContext());
}

static ty_t QIR_NODE_U16_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U16Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt16Ty(m.getContext());
}

static ty_t QIR_NODE_U32_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U32Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt32Ty(m.getContext());
}

static ty_t QIR_NODE_U64_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U64Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt64Ty(m.getContext());
}

static ty_t QIR_NODE_U128_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::U128Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt128Ty(m.getContext());
}

static ty_t QIR_NODE_I8_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I8Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt8Ty(m.getContext());
}

static ty_t QIR_NODE_I16_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I16Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt16Ty(m.getContext());
}

static ty_t QIR_NODE_I32_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I32Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt32Ty(m.getContext());
}

static ty_t QIR_NODE_I64_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I64Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt64Ty(m.getContext());
}

static ty_t QIR_NODE_I128_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::I128Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getInt128Ty(m.getContext());
}

static ty_t QIR_NODE_F16_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F16Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getHalfTy(m.getContext());
}

static ty_t QIR_NODE_F32_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F32Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getFloatTy(m.getContext());
}

static ty_t QIR_NODE_F64_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F64Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getDoubleTy(m.getContext());
}

static ty_t QIR_NODE_F128_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::F128Ty *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getFP128Ty(m.getContext());
}

static ty_t QIR_NODE_VOID_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::VoidTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  return llvm::Type::getVoidTy(m.getContext());
}

static ty_t QIR_NODE_PTR_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::PtrTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  ty_t R = T(m, b, cf, s, N->getPointee());
  if (!R) {
    debug("Failed to get pointee type");
    return std::nullopt;
  }

  return llvm::PointerType::get(R.value(), 0);
}

static ty_t QIR_NODE_OPAQUE_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s,
                                 qxir::OpaqueTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  qcore_assert(!N->getName().empty());

  return llvm::StructType::create(m.getContext(), N->getName());
}

static ty_t QIR_NODE_STRING_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s,
                                 qxir::StringTy *N) {
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

static ty_t QIR_NODE_STRUCT_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s,
                                 qxir::StructTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  std::vector<llvm::Type *> elements;
  elements.reserve(N->getFields().size());

  for (auto &field : N->getFields()) {
    ty_t R = T(m, b, cf, s, field);
    if (!R) {
      debug("Failed to get field type");
      return std::nullopt;
    }

    elements.push_back(R.value());
  }

  return llvm::StructType::get(m.getContext(), elements, true);
}

static ty_t QIR_NODE_UNION_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::UnionTy *N) {
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

static ty_t QIR_NODE_ARRAY_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::ArrayTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  ty_t R = T(m, b, cf, s, N->getElement());
  if (!R) {
    debug("Failed to get element type");
    return std::nullopt;
  }

  return llvm::ArrayType::get(R.value(), N->getCount());
}

static ty_t QIR_NODE_FN_TY_C(ctx_t &m, craft_t &b, const Mode &cf, State &s, qxir::FnTy *N) {
  /**
   * @brief [Write explanation here]
   *
   * @note [Write expected behavior here]
   *
   * @note [Write assumptions here]
   */

  std::vector<llvm::Type *> params;
  params.reserve(N->getParams().size());

  for (auto &param : N->getParams()) {
    ty_t R = T(m, b, cf, s, param);
    if (!R) {
      debug("Failed to get parameter type");
      return std::nullopt;
    }

    params.push_back(R.value());
  }

  ty_t R = T(m, b, cf, s, N->getReturn());
  if (!R) {
    debug("Failed to get return type");
    return std::nullopt;
  }

  bool is_vararg = N->getAttrs().contains(qxir::FnAttr::Variadic);

  return llvm::FunctionType::get(R.value(), params, is_vararg);
}
