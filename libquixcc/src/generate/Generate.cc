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

/// TODO: make this code less horrible

#include <generate/Generate.h>
#include <generate/CodeGen.h>
#include <stdlib.h>
#include "llvm/LLVMWrapper.h"
#include <error/Message.h>
#include <filesystem>
#include <fstream>

#include <llvm/IR/Verifier.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include "llvm/Target/TargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include <stdlib.h>

static std::map<std::string, std::string> acceptable_asmgen_flags = {
    {"-O0", "-O0"},
    {"-O1", "-O1"},
    {"-O2", "-O2"},
    {"-O3", "-O3"},
    {"-g", "-g"},
    {"-v", "-v"},
    {"-fPIC", "-fPIC"},
    {"-fPIE", "-fPIE"}};

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

static std::map<std::string, std::string> acceptable_bingen_flags = {
    {"-O0", "-O0"},
    {"-O1", "-O1"},
    {"-O2", "-O2"},
    {"-O3", "-O3"},
    {"-g", "-g"},
    {"-v", "-v"},
    {"-flto", "-flto"},
    {"-static", "-static"},
    {"-shared", "-shared"},
    {"-fPIC", "-fPIC"},
    {"-fPIE", "-fPIE"}};

bool libquixcc::write_IR(quixcc_job_t &ctx, std::shared_ptr<libquixcc::BlockNode> ast, const std::string &ir_filename)
{
    // Check if the quixcc_job_t is valid
    if (!ctx.m_inner)
    {
        message(ctx, libquixcc::Err::FATAL, "Invalid quixcc_job_t");
        return false;
    }

    // Create a llvm::raw_fd_ostream
    std::error_code ec;
    llvm::raw_fd_ostream os(ir_filename, ec);
    if (os.has_error())
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to create llvm::raw_fd_ostream");
        return false;
    }

    // Get the LLVMContext from the quixcc_job_t
    LLVMContext &llvm_ctx = *(LLVMContext *)ctx.m_inner;

    // Add root nodes to the LLVMContext
    for (auto &node : ast->m_stmts)
    {
        llvm::Value *val = node->codegen(CodegenVisitor(&llvm_ctx));
        if (!val)
        {
            message(ctx, libquixcc::Err::ERROR, "Failed to generate code for node");
            return false;
        }
    }

    // Verify the module
    message(ctx, libquixcc::Err::DEBUG, "Verifying LLVM module");
    std::string err;
    llvm::raw_string_ostream err_stream(err);

    if (llvm::verifyModule(*llvm_ctx.m_module, &err_stream))
    {
        throw std::runtime_error("LLVM IR generation failed. The AST must have been semantically incorrect: " + err_stream.str());
    }

    // Generate the IR
    message(ctx, libquixcc::Err::DEBUG, "Generating LLVM IR");
    llvm_ctx.m_module->print(os, nullptr, ctx.m_argset->contains("-g"));

    message(ctx, libquixcc::Err::DEBUG, "Finished generating LLVM IR");

    return true;
}

bool libquixcc::write_asm(quixcc_job_t &ctx, const std::string &ir_filename, const std::string &asm_filename)
{
    // LLVM createTargetMachine is bugging and not working
    // therefore, we will use clang binary to generate the assembly

#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
    message(ctx, libquixcc::Err::FATAL, "Unsupported operating system");
    throw std::runtime_error("Unsupported operating system");
#else
    std::string flags = "-ffunction-sections -fdata-sections ";

    for (const auto &e : *ctx.m_argset)
        if (acceptable_asmgen_flags.contains(e.first))
            flags += acceptable_asmgen_flags[e.first] + " ";

    // Its ugly, but it works
    std::string os_cmd = "clang -Wno-override-module -S " + ir_filename + " -o " + asm_filename + " " + flags;

    message(ctx, libquixcc::Err::DEBUG, "Running command: " + os_cmd);
    if (system(os_cmd.c_str()) != 0)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate assembly");
        return false;
    }

    return true;
#endif
}

bool libquixcc::write_obj(quixcc_job_t &ctx, const std::string &asm_filename, const std::string &obj_filename)
{
#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
    message(ctx, libquixcc::Err::FATAL, "Unsupported operating system");
    throw std::runtime_error("Unsupported operating system");
#else
    std::string flags;
    for (const auto &e : *ctx.m_argset)
        if (acceptable_objgen_flags.contains(e.first))
            flags += acceptable_objgen_flags[e.first] + " ";

    // Its ugly, but it works
    std::string os_cmd = "clang -c " + asm_filename + " -o " + obj_filename + " " + flags;

    message(ctx, libquixcc::Err::DEBUG, "Running command: " + os_cmd);
    if (system(os_cmd.c_str()) != 0)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate object file");
        return false;
    }

    return true;
#endif
}

bool libquixcc::write_bin(quixcc_job_t &ctx, const std::string &obj_filename, const std::string &bin_filename)
{
#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
    message(ctx, libquixcc::Err::FATAL, "Unsupported operating system");
    throw std::runtime_error("Unsupported operating system");
#else
    if (ctx.m_argset->contains("-staticlib"))
    {
        message(ctx, libquixcc::Err::DEBUG, "Static library output requested");
        std::string os_cmd = "ar rcs " + bin_filename + " " + obj_filename;
        message(ctx, libquixcc::Err::DEBUG, "Running command: " + os_cmd);
        if (system(os_cmd.c_str()) != 0)
        {
            message(ctx, libquixcc::Err::ERROR, "Failed to generate static library");
            return false;
        }
        return true;
    }

    // if not building a static library, do the following
    std::string flags = "-nostdlib -nostartfiles -nodefaultlibs ";
    for (const auto &e : *ctx.m_argset)
        if (acceptable_bingen_flags.contains(e.first))
            flags += acceptable_bingen_flags[e.first] + " ";

    // Its ugly, but it works
    std::string os_cmd = "clang " + flags + " " + obj_filename + " -o " + bin_filename;

    message(ctx, libquixcc::Err::DEBUG, "Running command: " + os_cmd);
    bool ok = system(os_cmd.c_str()) == 0;

    if (!ok)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate executable");
        return false;
    }

    return true;
#endif
}

bool libquixcc::generate(quixcc_job_t &job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    std::string ir_filename = std::tmpnam(nullptr) + std::string(".ll");
    std::string asm_filename = std::tmpnam(nullptr) + std::string(".s");
    std::string obj_filename = std::tmpnam(nullptr) + std::string(".o");
    std::string bin_filename = std::tmpnam(nullptr) + std::string(".out");
    char buf[4096];
    size_t n = 0;
    FILE *ir_out, *asm_out, *obj_out, *bin_out;
    ir_out = asm_out = obj_out = bin_out = nullptr;

    if (!libquixcc::write_IR(job, ast, ir_filename))
    {
        libquixcc::message(job, libquixcc::Err::ERROR, "failed to generate output");
        goto remove_irfile_false;
    }

    if (job.m_argset->contains("-IR"))
    {
        libquixcc::message(job, libquixcc::Err::DEBUG, "IR output requested");

        if ((ir_out = fopen(ir_filename.c_str(), "r")) == nullptr)
        {
            libquixcc::message(job, libquixcc::Err::ERROR, "failed to open temporary file to write IR");
            goto remove_irfile_false;
        }

        while ((n = fread(buf, 1, sizeof(buf), ir_out)) > 0)
        {
            if (fwrite(buf, 1, n, job.m_out) != n)
            {
                libquixcc::message(job, libquixcc::Err::ERROR, "failed to write IR to output");
                fclose(ir_out);
                goto remove_irfile_false;
            }
        }

        if (ferror(ir_out) != 0)
        {
            libquixcc::message(job, libquixcc::Err::ERROR, "failed to read IR from temporary file");
            fclose(ir_out);
            goto remove_irfile_false;
        }

        fclose(ir_out);
        return true;
    }

    if (!libquixcc::write_asm(job, ir_filename, asm_filename))
    {
        libquixcc::message(job, libquixcc::Err::ERROR, "failed to generate assembly");
        goto remove_irfile_asm_false;
    }

    message(job, libquixcc::Err::DEBUG, "Generated assembly");

    if (job.m_argset->contains("-S"))
    {
        libquixcc::message(job, libquixcc::Err::DEBUG, "Assembly output requested");

        if ((asm_out = fopen(asm_filename.c_str(), "r")) == nullptr)
        {
            libquixcc::message(job, libquixcc::Err::ERROR, "failed to open temporary file to write assembly");
            goto remove_irfile_asm_false;
        }

        while ((n = fread(buf, 1, sizeof(buf), asm_out)) > 0)
        {
            if (fwrite(buf, 1, n, job.m_out) != n)
            {
                libquixcc::message(job, libquixcc::Err::ERROR, "failed to write assembly to output");
                fclose(asm_out);
                goto remove_irfile_asm_false;
            }
        }

        if (ferror(asm_out) != 0)
        {
            libquixcc::message(job, libquixcc::Err::ERROR, "failed to read assembly from temporary file");
            fclose(asm_out);
            goto remove_irfile_asm_false;
        }

        fclose(asm_out);
        return true;
    }

    if (!libquixcc::write_obj(job, asm_filename, obj_filename))
    {
        libquixcc::message(job, libquixcc::Err::ERROR, "failed to generate object file");
        goto remove_irfile_asm_obj_false;
    }

    message(job, libquixcc::Err::DEBUG, "Generated object file");

    if (job.m_argset->contains("-c"))
    {
        libquixcc::message(job, libquixcc::Err::DEBUG, "Object file output requested");

        if ((obj_out = fopen(obj_filename.c_str(), "r")) == nullptr)
        {
            libquixcc::message(job, libquixcc::Err::ERROR, "failed to open temporary file to write object file");
            goto remove_irfile_asm_obj_false;
        }

        while ((n = fread(buf, 1, sizeof(buf), obj_out)) > 0)
        {
            if (fwrite(buf, 1, n, job.m_out) != n)
            {
                libquixcc::message(job, libquixcc::Err::ERROR, "failed to write object file to output");
                fclose(obj_out);
                goto remove_irfile_asm_obj_false;
            }
        }

        if (ferror(obj_out) != 0)
        {
            libquixcc::message(job, libquixcc::Err::ERROR, "failed to read object file from temporary file");
            fclose(obj_out);
            goto remove_irfile_asm_obj_false;
        }

        fclose(obj_out);
        return true;
    }

    if (!libquixcc::write_bin(job, obj_filename, bin_filename))
    {
        libquixcc::message(job, libquixcc::Err::ERROR, "failed to generate executable");
        goto remove_irfile_asm_obj_false;
    }

    message(job, libquixcc::Err::DEBUG, "Generated executable");

    if ((bin_out = fopen(bin_filename.c_str(), "r")) == nullptr)
    {
        libquixcc::message(job, libquixcc::Err::ERROR, "failed to open temporary file to write executable");
        fclose(bin_out);
        goto remove_irfile_asm_obj_false;
    }

    while ((n = fread(buf, 1, sizeof(buf), bin_out)) > 0)
    {
        if (fwrite(buf, 1, n, job.m_out) != n)
        {
            libquixcc::message(job, libquixcc::Err::ERROR, "failed to write object file to output");
            fclose(bin_out);
            goto remove_irfile_asm_obj_false;
        }
    }

    if (ferror(bin_out) != 0)
    {
        libquixcc::message(job, libquixcc::Err::ERROR, "failed to read object file from temporary file");
        fclose(bin_out);
        goto remove_irfile_asm_obj_false;
    }

    fflush(job.m_out);
    fclose(bin_out);

    std::filesystem::remove(ir_filename);
    std::filesystem::remove(asm_filename);
    std::filesystem::remove(obj_filename);
    std::filesystem::remove(bin_filename);
    return true; // TODO: fixme

remove_irfile_false:
    std::filesystem::remove(ir_filename);
    return false;

remove_irfile_asm_false:
    std::filesystem::remove(ir_filename);
    std::filesystem::remove(asm_filename);
    return false;

remove_irfile_asm_obj_false:
    std::filesystem::remove(ir_filename);
    std::filesystem::remove(asm_filename);
    std::filesystem::remove(obj_filename);
    return false;
}