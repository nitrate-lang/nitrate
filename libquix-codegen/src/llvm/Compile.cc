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

#include <llvm-14/llvm/Support/CodeGen.h>
#include <llvm-14/llvm/Support/ManagedStatic.h>

#include <cstdint>
#include <streambuf>
#define __QUIX_IMPL__

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/Sema.h>
#include <core/LibMacro.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/InitializePasses.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Host.h>
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

#include <exception>
#include <iostream>
#include <memory>
#include <sstream>

class OStreamWriter : public std::streambuf {
  FILE* m_file;

public:
  OStreamWriter(FILE* file) : m_file(file) {}

  virtual std::streamsize xsputn(const char* s, std::streamsize n) override {
    return fwrite(s, 1, n, m_file);
  }

  virtual int overflow(int c) override { return fputc(c, m_file); }
};

class OStreamDiscard : public std::streambuf {
public:
  virtual std::streamsize xsputn(const char* s, std::streamsize n) override { return n; }
  virtual int overflow(int c) override { return c; }
};

class my_pwrite_ostream : public llvm::raw_pwrite_stream {
  std::ostream& m_os;

public:
  my_pwrite_ostream(std::ostream& os) : llvm::raw_pwrite_stream(true), m_os(os) {}

  void write_impl(const char* ptr, size_t size) override { m_os.write(ptr, size); }

  void pwrite_impl(const char* ptr, size_t size, uint64_t offset) override {
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

static bool compile_to_llvm_ir(std::string_view cxx_filename, std::ostream& err,
                               std::ostream& code) {
  /**
   * @ref
   * https://web.archive.org/web/20201023120232/https://blog.audio-tk.com/2018/09/18/compiling-c-code-in-memory-with-clang/
   */

  using namespace clang;

  try {
    DiagnosticOptions* DiagOpts = new DiagnosticOptions();
    llvm::raw_os_ostream err_os(err);
    TextDiagnosticPrinter* textDiagPrinter = new TextDiagnosticPrinter(err_os, DiagOpts);
    DiagnosticIDs* pDiagIDs = new DiagnosticIDs();

    /* diagnosticsEngine takes ownership of:
     * - pDiagIDs
     * - DiagOpts
     * - textDiagPrinter
     */
    auto diagnosticsEngine = std::make_unique<DiagnosticsEngine>(
        std::move(pDiagIDs), std::move(DiagOpts), std::move(textDiagPrinter));

    CompilerInstance compilerInstance;
    auto& compilerInvocation = compilerInstance.getInvocation();

    ///==========================================================================

    std::stringstream ss;
    ss << "-triple=" << llvm::sys::getDefaultTargetTriple();

    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::istream_iterator<std::string> i = begin;
    std::vector<const char*> itemcstrs;
    std::vector<std::string> itemstrs;
    while (i != end) {
      itemstrs.push_back(*i);
      ++i;
    }

    for (unsigned idx = 0; idx < itemstrs.size(); idx++) {
      // note: if itemstrs is modified after this, itemcstrs will be full
      // of invalid pointers! Could make copies, but would have to clean up then...
      itemcstrs.push_back(itemstrs[idx].c_str());
    }

    llvm::ArrayRef<const char*> itemcstrsRef(itemcstrs);
    CompilerInvocation::CreateFromArgs(compilerInvocation, itemcstrsRef, *diagnosticsEngine);

    ///==========================================================================

    auto& targetOptions = compilerInvocation.getTargetOpts();
    auto& frontEndOptions = compilerInvocation.getFrontendOpts();
#ifdef DEBUG
    frontEndOptions.ShowStats = true;
#endif
    auto& headerSearchOptions = compilerInvocation.getHeaderSearchOpts();
    (void)headerSearchOptions;
#ifdef DEBUG
    headerSearchOptions.Verbose = true;
#endif

    ///==========================================================================

    frontEndOptions.Inputs.clear();

    InputKind inputKind(Language::CXX);
    frontEndOptions.Inputs.push_back(FrontendInputFile(cxx_filename, inputKind));

    targetOptions.Triple = llvm::sys::getDefaultTargetTriple();
    compilerInstance.createDiagnostics(textDiagPrinter, false);

    llvm::LLVMContext context;
    std::unique_ptr<CodeGenAction> action = std::make_unique<EmitLLVMOnlyAction>(&context);

    if (!compilerInstance.ExecuteAction(*action)) {
      err << "error: failed to compile C++ code" << std::endl;
      return false;
    }

    ///==========================================================================

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
  } catch (std::exception& e) {
    err << e.what();
    return false;
  }
}

static bool compile_to_asm(std::string_view cxx_filename, std::ostream& err, std::ostream& code) {
  /**
   * @ref
   * https://web.archive.org/web/20201023120232/https://blog.audio-tk.com/2018/09/18/compiling-c-code-in-memory-with-clang/
   */

  using namespace clang;

  try {
    /// TODO: Get the target triple from somewhere else
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string CPU = "generic";
    std::string Features = "";

    DiagnosticOptions* DiagOpts = new DiagnosticOptions();
    llvm::raw_os_ostream err_os(err);
    TextDiagnosticPrinter* textDiagPrinter = new TextDiagnosticPrinter(err_os, DiagOpts);
    DiagnosticIDs* pDiagIDs = new DiagnosticIDs();

    /* diagnosticsEngine takes ownership of:
     * - pDiagIDs
     * - DiagOpts
     * - textDiagPrinter
     */
    auto diagnosticsEngine = std::make_unique<DiagnosticsEngine>(
        std::move(pDiagIDs), std::move(DiagOpts), std::move(textDiagPrinter));

    CompilerInstance compilerInstance;
    auto& compilerInvocation = compilerInstance.getInvocation();

    ///==========================================================================

    std::stringstream ss;
    ss << "-triple=" << targetTriple;

    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::istream_iterator<std::string> i = begin;
    std::vector<const char*> itemcstrs;
    std::vector<std::string> itemstrs;
    while (i != end) {
      itemstrs.push_back(*i);
      ++i;
    }

    for (unsigned idx = 0; idx < itemstrs.size(); idx++) {
      // note: if itemstrs is modified after this, itemcstrs will be full
      // of invalid pointers! Could make copies, but would have to clean up then...
      itemcstrs.push_back(itemstrs[idx].c_str());
    }

    llvm::ArrayRef<const char*> itemcstrsRef(itemcstrs);
    CompilerInvocation::CreateFromArgs(compilerInvocation, itemcstrsRef, *diagnosticsEngine);

    ///==========================================================================

    auto& targetOptions = compilerInvocation.getTargetOpts();
    auto& frontEndOptions = compilerInvocation.getFrontendOpts();
#ifdef DEBUG
    frontEndOptions.ShowStats = true;
#endif
    auto& headerSearchOptions = compilerInvocation.getHeaderSearchOpts();
    (void)headerSearchOptions;
#ifdef DEBUG
    headerSearchOptions.Verbose = true;
#endif

    ///==========================================================================

    frontEndOptions.Inputs.clear();

    InputKind inputKind(Language::CXX);
    frontEndOptions.Inputs.push_back(FrontendInputFile(cxx_filename, inputKind));

    targetOptions.Triple = targetTriple;
    compilerInstance.createDiagnostics(textDiagPrinter, false);

    llvm::LLVMContext context;
    std::unique_ptr<CodeGenAction> action = std::make_unique<EmitLLVMOnlyAction>(&context);

    if (!compilerInstance.ExecuteAction(*action)) {
      err << "error: failed to compile C++ code" << std::endl;
      return false;
    }

    ///==========================================================================

    std::string lookupTarget_err;
    auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, lookupTarget_err);
    if (!Target) {
      err << "error: failed to lookup target: " << lookupTarget_err << std::endl;
      return false;
    }
    llvm::TargetOptions opt;
    auto TargetMachine =
        Target->createTargetMachine(targetTriple, CPU, Features, opt, llvm::Reloc::PIC_);

    ///==========================================================================

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

    // Create the pass manager.
    // This one corresponds to a typical -O2 optimization pipeline.
    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

    // Optimize the IR!
    MPM.run(*module, MAM);

    ///==========================================================================

    /// FIXME: Figure out how to do this probably with streams
    std::error_code ec;
    llvm::SmallVector<char> output_buffer;
    llvm::raw_svector_ostream code_os(output_buffer);

    llvm::legacy::PassManager pass;
    TargetMachine->addPassesToEmitFile(pass, code_os, nullptr, llvm::CGFT_AssemblyFile);
    if (!pass.run(*module)) {
      err << "error: failed to emit assembly" << std::endl;
      return false;
    }

    code.write(output_buffer.data(), output_buffer.size());

    ///==========================================================================

    return true;
  } catch (std::exception& e) {
    err << e.what();
    return false;
  }
}

static bool compile_to_obj(std::string_view cxx_filename, std::ostream& err, std::ostream& code) {
  /**
   * @ref
   * https://web.archive.org/web/20201023120232/https://blog.audio-tk.com/2018/09/18/compiling-c-code-in-memory-with-clang/
   */

  using namespace clang;

  try {
    /// TODO: Get the target triple from somewhere else
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string CPU = "generic";
    std::string Features = "";

    DiagnosticOptions* DiagOpts = new DiagnosticOptions();
    llvm::raw_os_ostream err_os(err);
    TextDiagnosticPrinter* textDiagPrinter = new TextDiagnosticPrinter(err_os, DiagOpts);
    DiagnosticIDs* pDiagIDs = new DiagnosticIDs();

    /* diagnosticsEngine takes ownership of:
     * - pDiagIDs
     * - DiagOpts
     * - textDiagPrinter
     */
    auto diagnosticsEngine = std::make_unique<DiagnosticsEngine>(
        std::move(pDiagIDs), std::move(DiagOpts), std::move(textDiagPrinter));

    CompilerInstance compilerInstance;
    auto& compilerInvocation = compilerInstance.getInvocation();

    ///==========================================================================

    std::stringstream ss;
    ss << "-triple=" << targetTriple;

    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::istream_iterator<std::string> i = begin;
    std::vector<const char*> itemcstrs;
    std::vector<std::string> itemstrs;
    while (i != end) {
      itemstrs.push_back(*i);
      ++i;
    }

    for (unsigned idx = 0; idx < itemstrs.size(); idx++) {
      // note: if itemstrs is modified after this, itemcstrs will be full
      // of invalid pointers! Could make copies, but would have to clean up then...
      itemcstrs.push_back(itemstrs[idx].c_str());
    }

    llvm::ArrayRef<const char*> itemcstrsRef(itemcstrs);
    CompilerInvocation::CreateFromArgs(compilerInvocation, itemcstrsRef, *diagnosticsEngine);

    ///==========================================================================

    auto& targetOptions = compilerInvocation.getTargetOpts();
    auto& frontEndOptions = compilerInvocation.getFrontendOpts();
#ifdef DEBUG
    frontEndOptions.ShowStats = true;
#endif
    auto& headerSearchOptions = compilerInvocation.getHeaderSearchOpts();
    (void)headerSearchOptions;
#ifdef DEBUG
    headerSearchOptions.Verbose = true;
#endif

    ///==========================================================================

    frontEndOptions.Inputs.clear();

    InputKind inputKind(Language::CXX);
    frontEndOptions.Inputs.push_back(FrontendInputFile(cxx_filename, inputKind));

    targetOptions.Triple = targetTriple;
    compilerInstance.createDiagnostics(textDiagPrinter, false);

    llvm::LLVMContext context;
    std::unique_ptr<CodeGenAction> action = std::make_unique<EmitLLVMOnlyAction>(&context);

    if (!compilerInstance.ExecuteAction(*action)) {
      err << "error: failed to compile C++ code" << std::endl;
      return false;
    }

    ///==========================================================================

    std::string lookupTarget_err;
    auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, lookupTarget_err);
    if (!Target) {
      err << "error: failed to lookup target: " << lookupTarget_err << std::endl;
      return false;
    }
    llvm::TargetOptions opt;
    auto TargetMachine =
        Target->createTargetMachine(targetTriple, CPU, Features, opt, llvm::Reloc::PIC_);

    ///==========================================================================

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

    ///==========================================================================

    return true;
  } catch (std::exception& e) {
    err << e.what();
    return false;
  }
}

struct qmodule_t;

LIB_EXPORT bool qcode_ir(qmodule_t* module, qcode_conf_t* conf, FILE* err, FILE* out) {
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

    char cxx_filename[] = "/tmp/quix-XXXXXX";
    int cxx_fd = mkstemp(cxx_filename);
    if (cxx_fd == -1) {
      err_stream << "error: mkstemp: failed to create temporary file" << std::endl;
      return false;
    }
    FILE* cxx_memfile = fdopen(cxx_fd, "wb+");
    if (!cxx_memfile) {
      err_stream << "error: fdopen: failed to create temporary file" << std::endl;
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      return false;
    }

    if (!qcode_transcode(module, conf, QCODE_CXX11, QCODE_MINIFY, err, cxx_memfile)) {
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      err_stream << "error: failed to transcode module" << std::endl;
      return false;
    }
    rewind(cxx_memfile);

    if (!compile_to_llvm_ir(cxx_filename, err_stream, out_stream)) {
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      err_stream.flush();
      out_stream.flush();
      err&& fflush(err);
      out&& fflush(out);
      return false;
    }

    fclose(cxx_memfile);
    std::remove(cxx_filename);
    err_stream.flush();
    out_stream.flush();
    err&& fflush(err);
    out&& fflush(out);

    return true;
  } catch (std::exception& e) {
    fprintf(err, "%s", e.what());
    err&& fflush(err);
    out&& fflush(out);
    return false;
  }
}

LIB_EXPORT bool qcode_asm(qmodule_t* module, qcode_conf_t* conf, FILE* err, FILE* out) {
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

    char cxx_filename[] = "/tmp/quix-XXXXXX";
    int cxx_fd = mkstemp(cxx_filename);
    if (cxx_fd == -1) {
      err_stream << "error: mkstemp: failed to create temporary file" << std::endl;
      return false;
    }
    FILE* cxx_memfile = fdopen(cxx_fd, "wb+");
    if (!cxx_memfile) {
      err_stream << "error: fdopen: failed to create temporary file" << std::endl;
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      return false;
    }

    if (!qcode_transcode(module, conf, QCODE_CXX11, QCODE_MINIFY, err, cxx_memfile)) {
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      err_stream << "error: failed to transcode module" << std::endl;
      return false;
    }
    rewind(cxx_memfile);

    if (!compile_to_asm(cxx_filename, err_stream, out_stream)) {
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      err_stream.flush();
      out_stream.flush();
      err&& fflush(err);
      out&& fflush(out);
      return false;
    }

    fclose(cxx_memfile);
    std::remove(cxx_filename);
    err_stream.flush();
    out_stream.flush();
    err&& fflush(err);
    out&& fflush(out);

    return true;
  } catch (std::exception& e) {
    fprintf(err, "%s", e.what());
    err&& fflush(err);
    out&& fflush(out);
    return false;
  }
}

LIB_EXPORT bool qcode_obj(qmodule_t* module, qcode_conf_t* conf, FILE* err, FILE* out) {
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

    char cxx_filename[] = "/tmp/quix-XXXXXX";
    int cxx_fd = mkstemp(cxx_filename);
    if (cxx_fd == -1) {
      err_stream << "error: mkstemp: failed to create temporary file" << std::endl;
      return false;
    }
    FILE* cxx_memfile = fdopen(cxx_fd, "wb+");
    if (!cxx_memfile) {
      err_stream << "error: fdopen: failed to create temporary file" << std::endl;
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      return false;
    }

    if (!qcode_transcode(module, conf, QCODE_CXX11, QCODE_MINIFY, err, cxx_memfile)) {
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      err_stream << "error: failed to transcode module" << std::endl;
      return false;
    }
    rewind(cxx_memfile);

    if (!compile_to_obj(cxx_filename, err_stream, out_stream)) {
      fclose(cxx_memfile);
      std::remove(cxx_filename);
      err_stream.flush();
      out_stream.flush();
      err&& fflush(err);
      out&& fflush(out);
      return false;
    }

    fclose(cxx_memfile);
    std::remove(cxx_filename);
    err_stream.flush();
    out_stream.flush();
    err&& fflush(err);
    out&& fflush(out);

    return true;
  } catch (std::exception& e) {
    fprintf(err, "%s", e.what());
    err&& fflush(err);
    out&& fflush(out);
    return false;
  }
}
