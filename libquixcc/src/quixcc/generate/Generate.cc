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

#define QUIXCC_INTERNAL

#include <quixcc/core/Logger.h>
#include <quixcc/generate/CodeGen.h>
#include <quixcc/generate/Generate.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <quixcc/llvm/LLVMWrapper.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <stdlib.h>

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace libquixcc;
using namespace libquixcc::codegen;

static std::map<std::string, std::string> acceptable_objgen_flags = {
    {"-O0", "-O0"},     {"-O1", "-O1"},     {"-O2", "-O2"},
    {"-O3", "-O3"},     {"-g", "-g"},       {"-v", "-v"},
    {"-flto", "-flto"}, {"-fPIC", "-fPIC"}, {"-fPIE", "-fPIE"}};

bool libquixcc::codegen::write_IR(quixcc_job_t &ctx,
                                  std::unique_ptr<ir::delta::IRDelta> &ir,
                                  FILE *out, bool generate_bitcode) {
  std::error_code ec;
  std::string output_buffer;
  llvm::raw_string_ostream os(output_buffer);

  if (!LLVM14Codegen::codegen(ir, ctx.m_inner)) {
    LOG(ERROR) << log::raw << ctx.m_filename.top()
               << ": Failed to generate LLVM IR" << std::endl;
    return false;
  }

  LOG(DEBUG) << "Verifying LLVM module" << std::endl;
  std::string err;
  llvm::raw_string_ostream err_stream(err);

  // Verify the module
  if (llvm::verifyModule(*ctx.m_inner.m_module, &err_stream)) {
    if (ctx.m_debug)
      LOG(FAILED) << log::raw << ctx.m_filename.top()
                  << "LLVM IR generation failed. The Ptree must have been "
                     "semantically incorrect: "
                  << err_stream.str() << std::endl;
    else
      LOG(ERROR) << log::raw << ctx.m_filename.top()
                 << "LLVM IR generation failed. The Ptree must have been "
                    "semantically incorrect: "
                 << err_stream.str() << std::endl;
  }

  LOG(DEBUG) << "Generating LLVM IR" << std::endl;

  if (generate_bitcode) {
    LOG(DEBUG) << "Generating LLVM Bitcode" << std::endl;
    throw std::runtime_error("LLVM Bitcode generation is not yet implemented");
    LOG(DEBUG) << "Finished generating LLVM Bitcode" << std::endl;
  } else {
    LOG(DEBUG) << "Generating LLVM IR" << std::endl;
    ctx.m_inner.m_module->print(os, nullptr, ctx.has("-g"));
    if (!fwrite(output_buffer.c_str(), 1, output_buffer.size(), out)) {
      LOG(ERROR) << "Failed to write LLVM IR to file" << std::endl;
      return false;
    }
    fflush(out);
  }

  LOG(DEBUG) << "Finished generating LLVM IR" << std::endl;

  return true;
}

bool libquixcc::codegen::write_c11(quixcc_job_t &ctx,
                                   std::unique_ptr<ir::delta::IRDelta> &ir,
                                   FILE *out) {
  LOG(DEBUG) << "Generating C" << std::endl;

  std::stringstream stream;

  if (!C11Codegen::codegen(ir, stream)) {
    LOG(ERROR) << "Failed to generate C11 code" << std::endl;
    return false;
  }

  std::string code = stream.str();

  if (fwrite(code.c_str(), 1, code.size(), out) != code.size()) {
    LOG(ERROR) << "Failed to write C11 code to file" << std::endl;
    return false;
  }

  fflush(out);

  LOG(DEBUG) << "Finished generating C" << std::endl;

  return true;
}

bool libquixcc::codegen::write_llvm(quixcc_job_t &ctx,
                                    std::unique_ptr<ir::delta::IRDelta> &ir,
                                    FILE *out, llvm::CodeGenFileType mode) {
#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && \
    !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
  LOG(FATAL) << "Unsupported operating system" << std::endl;
  throw core::Exception();
#else
  auto &TargetTriple = ctx.m_triple;

  LOG(DEBUG) << log::raw << "Generating code for target: " << TargetTriple
             << std::endl;

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  if (!Target) {
    llvm::errs() << Error;
    return false;
  }

  auto Features = "";

  llvm::TargetOptions opt;
  auto TargetMachine = Target->createTargetMachine(
      TargetTriple, ctx.m_cpu, Features, opt, llvm::Reloc::PIC_);

  ctx.m_inner.m_module->setDataLayout(TargetMachine->createDataLayout());
  ctx.m_inner.m_module->setTargetTriple(TargetTriple);

  std::error_code ec;
  llvm::SmallVector<char> output_buffer;
  llvm::raw_svector_ostream os(output_buffer);

  if (!LLVM14Codegen::codegen(ir, ctx.m_inner)) {
    LOG(ERROR) << log::raw << "Failed to generate LLVM Code for file"
               << ctx.m_filename.top() << std::endl;
    return false;
  }

  LOG(DEBUG) << "Verifying LLVM module" << std::endl;
  std::string err;
  llvm::raw_string_ostream err_stream(err);

  // Verify the module
  if (llvm::verifyModule(*ctx.m_inner.m_module, &err_stream)) {
    if (ctx.m_debug)
      LOG(FAILED) << log::raw
                  << "LLVM Code generation failed. The Ptree must have been "
                     "semantically incorrect: "
                  << err_stream.str() << std::endl;
    else
      LOG(ERROR) << log::raw
                 << "LLVM Code generation failed. The Ptree must have been "
                    "semantically incorrect: "
                 << err_stream.str() << std::endl;
  }

  llvm::PassManagerBuilder builder;
  llvm::legacy::PassManager pass;

  if (ctx.has("-O0")) {
    builder.OptLevel = 0;

    builder.populateModulePassManager(pass);
  } else if (ctx.has("-Os")) {
    builder.SizeLevel = 1;
    builder.populateModulePassManager(pass);
  } else if (ctx.has("-Oz")) {
    builder.SizeLevel = 2;
    builder.populateModulePassManager(pass);
  } else {
    builder.SizeLevel = 0;

    if (ctx.has("-O1"))
      builder.OptLevel = 1;
    else if (ctx.has("-O2"))
      builder.OptLevel = 2;
    else if (ctx.has("-O3"))
      builder.OptLevel = 3;

    builder.Inliner = llvm::createFunctionInliningPass(
        builder.OptLevel, builder.SizeLevel, false);

    builder.populateModulePassManager(pass);
  }

  if (TargetMachine->addPassesToEmitFile(pass, os, nullptr, mode)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    return false;
  }

  if (!pass.run(*ctx.m_inner.m_module)) {
    LOG(ERROR) << "Failed to generate code for file: " << ctx.m_filename.top()
               << std::endl;
    return false;
  }

  fwrite(output_buffer.data(), 1, output_buffer.size(), out);
  fflush(out);

  return true;
#endif
}

bool libquixcc::codegen::generate(quixcc_job_t &job,
                                  std::unique_ptr<ir::delta::IRDelta> &ir) {
  if (job.has("-emit-ir")) return write_IR(job, ir, job.m_out, false);

  if (job.has("-emit-bc")) return write_IR(job, ir, job.m_out, true);

  if (job.has("-emit-c11")) return write_c11(job, ir, job.m_out);

  if (job.has("-S"))
    return write_llvm(job, ir, job.m_out, llvm::CGFT_AssemblyFile);

  if (job.has("-c"))
    return write_llvm(job, ir, job.m_out, llvm::CGFT_ObjectFile);

  LOG(ERROR) << "Output format was not specified. Expected: [-emit-ir, "
                "-emit-quix-ir, -emit-delta-ir, "
                "-emit-c11, -emit-bc, -S, -c]"
             << std::endl;

  return false;
}