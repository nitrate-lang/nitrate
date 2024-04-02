////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <generate/Generate.h>
#include <generate/CodeGen.h>
#include <stdlib.h>
#include "llvm/LLVMWrapper.h"
#include <error/Message.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <llvm/IR/Verifier.h>
#include <llvm/Support/Host.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <stdlib.h>

static std::map<std::string, std::string> acceptable_objgen_flags = {
    {"-O0", "-O0"},
    {"-O1", "-O1"},
    {"-O2", "-O2"},
    {"-O3", "-O3"},
    {"-g", "-g"},
    {"-v", "-v"},
    {"-flto", "-flto"},
    {"-fPIC", "-fPIC"},
    {"-fPIE", "-fPIE"}};

class CFILE_raw_pwrite_ostream : public llvm::raw_pwrite_stream
{
public:
    explicit CFILE_raw_pwrite_ostream(FILE *file)
        : llvm::raw_pwrite_stream(false)
    {
        m_file = file;
    }

protected:
    FILE *m_file;

    void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override
    {
        if (fseek(m_file, Offset, SEEK_SET) == -1)
            llvm::report_fatal_error("Failed to seek in file");

        if (fwrite(Ptr, 1, Size, m_file) != Size)
            llvm::report_fatal_error("Failed to write to file");
    }

    void write_impl(const char *Ptr, size_t Size) override
    {
        if (fwrite(Ptr, 1, Size, m_file) != Size)
            llvm::report_fatal_error("Failed to write to file");
    }

    uint64_t current_pos() const override
    {
        long pos = ftell(m_file);
        if (pos == -1)
            llvm::report_fatal_error("Failed to get current position in file");

        return pos;
    }
};

bool libquixcc::write_IR(quixcc_job_t &ctx, const std::shared_ptr<libquixcc::AST> ast, FILE *out, bool generate_bitcode)
{
    std::error_code ec;
    CFILE_raw_pwrite_ostream os(out);

    // Generate code for AST
    if (!ast->codegen(CodegenVisitor(ctx.m_inner.get())))
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate LLVM IR");
        return false;
    }

    message(ctx, libquixcc::Err::DEBUG, "Verifying LLVM module");
    std::string err;
    llvm::raw_string_ostream err_stream(err);

    // Verify the module
    if (llvm::verifyModule(*ctx.m_inner->m_module, &err_stream))
        throw std::runtime_error("LLVM IR generation failed. The AST must have been semantically incorrect: " + err_stream.str());

    message(ctx, libquixcc::Err::DEBUG, "Generating LLVM IR");

    if (generate_bitcode)
    {
        message(ctx, libquixcc::Err::DEBUG, "Generating LLVM Bitcode");
        throw std::runtime_error("LLVM Bitcode generation is not yet implemented");
        message(ctx, libquixcc::Err::DEBUG, "Finished generating LLVM Bitcode");
    }
    else
    {
        message(ctx, libquixcc::Err::DEBUG, "Generating LLVM IR");
        ctx.m_inner->m_module->print(os, nullptr, ctx.m_argset.contains("-g"));
    }

    message(ctx, libquixcc::Err::DEBUG, "Finished generating LLVM IR");

    return true;
}

bool libquixcc::write_llvm(quixcc_job_t &ctx, std::shared_ptr<libquixcc::BlockNode> ast, FILE *out, llvm::CodeGenFileType mode)
{
#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
    message(ctx, libquixcc::Err::FATAL, "Unsupported operating system");
    throw std::runtime_error("Unsupported operating system");
#else
    auto &TargetTriple = ctx.m_triple;

    message(ctx, libquixcc::Err::DEBUG, "Generating code for target: %s", TargetTriple.c_str());

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target)
    {
        llvm::errs() << Error;
        return false;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);

    ctx.m_inner->m_module->setDataLayout(TargetMachine->createDataLayout());
    ctx.m_inner->m_module->setTargetTriple(TargetTriple);

    std::error_code ec;
    CFILE_raw_pwrite_ostream os(out);

    // Generate code for AST
    if (!ast->codegen(CodegenVisitor(ctx.m_inner.get())))
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate LLVM Code");
        return false;
    }

    message(ctx, libquixcc::Err::DEBUG, "Verifying LLVM module");
    std::string err;
    llvm::raw_string_ostream err_stream(err);

    // Verify the module
    if (llvm::verifyModule(*ctx.m_inner->m_module, &err_stream))
        throw std::runtime_error("LLVM Code generation failed. The AST must have been semantically incorrect: " + err_stream.str());

    llvm::legacy::PassManager pass;

    /// TODO: Add optimization levels

    if (TargetMachine->addPassesToEmitFile(pass, os, nullptr, mode))
    {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return false;
    }

    pass.run(*ctx.m_inner->m_module);
    os.flush();

    return true;
#endif
}

bool libquixcc::generate(quixcc_job_t &job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    if (job.m_argset.contains("-emit-ir"))
        return write_IR(job, ast, job.m_out, false);

    if (job.m_argset.contains("-emit-bc"))
        return write_IR(job, ast, job.m_out, true);

    if (job.m_argset.contains("-S"))
        return write_llvm(job, ast, job.m_out, llvm::CGFT_AssemblyFile);

    if (job.m_argset.contains("-c"))
        return write_llvm(job, ast, job.m_out, llvm::CGFT_ObjectFile);

    message(job, libquixcc::Err::FATAL, "Output format was not specified. Expected: [-emit-ir, -emit-bc, -S, -c]");
    return false;
}