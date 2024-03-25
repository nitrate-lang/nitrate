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
    {"-fPIE", "-fPIE"},
    {"-nostdlib", "-nostdlib"},
    {"-nostartfiles", "-nostartfiles"}};

bool libquixcc::write_IR(quixcc_job_t &ctx, const std::shared_ptr<libquixcc::AST> ast, FILE *out)
{
    int fd;
    // Get the file descriptor
    if ((fd = fileno(out)) == -1)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to get file descriptor");
        return false;
    }

    // Create a llvm::raw_fd_ostream
    std::error_code ec;
    llvm::raw_fd_ostream os(fd, false);
    if (os.has_error())
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to create llvm::raw_fd_ostream");
        return false;
    }

    // Add root nodes to the LLVMContext
    if (!ast->codegen(CodegenVisitor(ctx.m_inner.get())))
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate LLVM IR");
        return false;
    }

    // Verify the module
    message(ctx, libquixcc::Err::DEBUG, "Verifying LLVM module");
    std::string err;
    llvm::raw_string_ostream err_stream(err);

    if (llvm::verifyModule(*ctx.m_inner->m_module, &err_stream))
    {
        throw std::runtime_error("LLVM IR generation failed. The AST must have been semantically incorrect: " + err_stream.str());
    }

    // Generate the IR
    message(ctx, libquixcc::Err::DEBUG, "Generating LLVM IR");
    ctx.m_inner->m_module->print(os, nullptr, ctx.m_argset->contains("-g"));

    message(ctx, libquixcc::Err::DEBUG, "Finished generating LLVM IR");

    return true;
}

bool libquixcc::write_asm(quixcc_job_t &ctx, const std::shared_ptr<libquixcc::AST> ast, FILE *out)
{
    // LLVM createTargetMachine is bugging and not working
    // therefore, we will use clang binary to generate the assembly

#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
    message(ctx, libquixcc::Err::FATAL, "Unsupported operating system");
    throw std::runtime_error("Unsupported operating system");
#else
    std::string flags = "-Wno-override-module -ffunction-sections -fdata-sections ";

    for (const auto &e : *ctx.m_argset)
        if (acceptable_asmgen_flags.contains(e.first))
            flags += acceptable_asmgen_flags[e.first] + " ";

    std::string filename = std::tmpnam(nullptr) + std::string(".ll");
    FILE *tmp;
    if ((tmp = fopen(filename.c_str(), "w")) == nullptr)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to create temporary file");
        return false;
    }

    if (!write_IR(ctx, ast, tmp))
    {
        fclose(tmp);
        std::filesystem::remove(filename);
        message(ctx, libquixcc::Err::ERROR, "Failed to generate LLVM IR");
        return false;
    }
    fclose(tmp);

    if (system(("clang -S " + filename + " -o " + filename + ".S " + flags).c_str()) != 0)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate assembly file");
        return false;
    }

    std::ifstream ifs(filename + ".S");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    std::fputs(content.c_str(), out);

    std::filesystem::remove(filename);
    return true;
#endif
}

bool libquixcc::write_obj(quixcc_job_t &ctx, const std::shared_ptr<libquixcc::AST> ast, FILE *out)
{
#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
    message(ctx, libquixcc::Err::FATAL, "Unsupported operating system");
    throw std::runtime_error("Unsupported operating system");
#else
    std::string flags = "-Wno-override-module ";
    for (const auto &e : *ctx.m_argset)
        if (acceptable_objgen_flags.contains(e.first))
            flags += acceptable_objgen_flags[e.first] + " ";

    std::string filename = std::tmpnam(nullptr) + std::string(".ll");
    FILE *tmp;
    if ((tmp = fopen(filename.c_str(), "w")) == nullptr)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to create temporary file");
        return false;
    }

    if (!write_IR(ctx, ast, tmp))
    {
        fclose(tmp);
        std::filesystem::remove(filename);
        message(ctx, libquixcc::Err::ERROR, "Failed to generate LLVM IR");
        return false;
    }
    fclose(tmp);

    if (system(("clang -c " + filename + " -o " + filename + ".o " + flags).c_str()) != 0)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate object file");
        return false;
    }

    std::ifstream ifs(filename + ".o");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    std::fwrite(content.c_str(), 1, content.size(), out);
    std::filesystem::remove(filename);

    return true;
#endif
}

bool libquixcc::write_bin(quixcc_job_t &ctx, const std::shared_ptr<libquixcc::AST> ast, FILE *out)
{
#if !defined(__linux__) && !defined(__APPLE__) && !defined(__unix__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
    message(ctx, libquixcc::Err::FATAL, "Unsupported operating system");
    throw std::runtime_error("Unsupported operating system");
#else
    if (ctx.m_argset->contains("-staticlib"))
    {
        // message(ctx, libquixcc::Err::DEBUG, "Static library output requested");
        // std::string os_cmd = "ar rcs " + bin_filename + " " + obj_filename;
        // message(ctx, libquixcc::Err::DEBUG, "Running command: " + os_cmd);
        // if (system(os_cmd.c_str()) != 0)
        // {
        //     message(ctx, libquixcc::Err::ERROR, "Failed to generate static library");
        //     return false;
        // }
        // return true;

        /// TODO: implement static library generation
        return false;
    }

    // if not building a static library, do the following
    std::string flags = "-Wno-override-module ";
    for (const auto &e : *ctx.m_argset)
    {
        if (acceptable_bingen_flags.contains(e.first))
            flags += acceptable_bingen_flags[e.first] + " ";
        else if (e.first.starts_with("-l"))
        {
            message(ctx, libquixcc::Err::DEBUG, "Linking library: " + e.first);
            flags += e.first + " ";
        }
    }

    std::string filename = std::tmpnam(nullptr) + std::string(".ll");
    FILE *tmp;
    if ((tmp = fopen(filename.c_str(), "w")) == nullptr)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to create temporary file");
        return false;
    }

    if (!write_IR(ctx, ast, tmp))
    {
        fclose(tmp);
        std::filesystem::remove(filename);
        message(ctx, libquixcc::Err::ERROR, "Failed to generate LLVM IR");
        return false;
    }
    fclose(tmp);

    std::string cmd = "clang " + filename + " -o " + filename + ".out " + flags;
    message(ctx, libquixcc::Err::DEBUG, "Clang: " + cmd);
    if (system(cmd.c_str()) != 0)
    {
        message(ctx, libquixcc::Err::ERROR, "Failed to generate executable");
        return false;
    }

    std::ifstream ifs(filename + ".out");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    std::fwrite(content.c_str(), 1, content.size(), out);
    std::filesystem::remove(filename);

    return true;
#endif
}

bool libquixcc::generate(quixcc_job_t &job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    if (job.m_argset->contains("-emit-ir"))
        return write_IR(job, ast, job.m_out);
    else if (job.m_argset->contains("-S"))
        return write_asm(job, ast, job.m_out);
    else if (job.m_argset->contains("-c"))
        return write_obj(job, ast, job.m_out);
    else
        return write_bin(job, ast, job.m_out);
}