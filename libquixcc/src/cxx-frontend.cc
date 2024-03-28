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

#include <quixcc.hpp>
#include <LibMacro.h>
#include <thread>
#include <fstream>
#include <llvm/Support/Host.h>
#include "llvm/MC/TargetRegistry.h"

#include <sys/mman.h>

LIB_CXX_EXPORT quixcc::TargetTriple::TargetTriple(const char *_triple)
{
    if (!_triple)
        throw TargetTripleException("Invalid or unsupported LLVM target triple: (null)");

    std::string triple = _triple;

    if (triple[0] == '\0')
    {
        m_triple = llvm::sys::getDefaultTargetTriple();
    }
    else
    {
        std::string err;
        if (!llvm::TargetRegistry::lookupTarget(triple, err))
            throw TargetTripleException("Invalid or unsupported LLVM target triple: " + triple);

        m_triple = triple;
    }
}

LIB_CXX_EXPORT quixcc::Compiler::Compiler(std::vector<quixcc_job_t *> jobs, std::vector<std::string> oscmds)
{
    this->m_jobs = jobs;
    this->m_oscmds = oscmds;
    this->m_ok = false;
}

LIB_CXX_EXPORT quixcc::Compiler::~Compiler()
{
    for (auto job : this->m_jobs)
        quixcc_dispose(job);
}

LIB_CXX_EXPORT quixcc::Compiler &quixcc::Compiler::run(size_t max_threads)
{
    if (m_jobs.empty())
        return *this;

    std::vector<std::thread> threads;

    size_t thread_count = std::min(max_threads, this->m_jobs.size());

    if (thread_count == 1)
    {
        quixcc_run(this->m_jobs[0]);
    }
    else
    {
        // Allocate jobs into threads
        for (size_t i = 0; i < thread_count; i++)
        {
            threads.push_back(std::thread([this, i, thread_count]()
                                          {
                    for (size_t j = i; j < this->m_jobs.size(); j += thread_count)
                        quixcc_run(this->m_jobs[j]); }));
        }

        // Wait for all threads to finish
        for (auto &thread : threads)
            thread.join();
    }

    m_ok = true;

    // Collect messages
    for (auto job : this->m_jobs)
    {
        if (!job->m_result)
            throw std::runtime_error("Job result is null.");

        for (size_t i = 0; i < job->m_result->m_feedback.m_count; i++)
        {
            auto feedback = job->m_result->m_feedback.m_messages[i];
            this->m_messages.push_back(std::make_pair(feedback->message, feedback->m_level));
        }

        if (!job->m_result->m_success)
            this->m_ok = false;
    }

    return *this;
}

LIB_CXX_EXPORT const std::vector<std::pair<std::string, enum quixcc_msg_level_t>> &quixcc::Compiler::messages() const
{
    return this->m_messages;
}

LIB_CXX_EXPORT quixcc::Compiler &quixcc::Compiler::puts(std::ostream &normal, std::ostream &error)
{
    for (auto job : this->m_jobs)
    {
        size_t i;
        for (i = 0; i < job->m_result->m_feedback.m_count; i++)
        {
            auto feedback = job->m_result->m_feedback.m_messages[i];
            if (feedback->m_level < QUIXCC_ERROR)
                normal << feedback->message << std::endl;
            else
                error << feedback->message << std::endl;
        }
    }

    return *this;
}

LIB_CXX_EXPORT bool quixcc::Compiler::ok() const
{
    return m_ok;
}

///=============================================================================

LIB_CXX_EXPORT quixcc::CompilerBuilder::CompilerBuilder()
{
    m_verbose = m_debug = false;
    m_target = ""; // Host target
    m_input = m_output = nullptr;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder::~CompilerBuilder()
{
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::in(const std::string &file)
{
    FILE *input;
    if ((input = fopen(file.c_str(), "r")) == nullptr)
        throw std::runtime_error("Failed to open input file: " + file);

    m_files.push_back(std::make_pair(input, file));
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::in(const std::vector<std::string> &files)
{
    for (auto &file : files)
        in(file);

    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::src(const char *code, size_t size)
{
    m_input = fmemopen((void *)code, size, "r");
    if (!m_input)
        throw std::runtime_error("Failed to open input stream.");
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::src(FILE *input)
{
    m_input = input;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::out(const std::string &file)
{
    if ((m_output = fopen(file.c_str(), "w+")) == nullptr)
        throw std::runtime_error("Failed to open output file: " + file);

    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::out(FILE *output)
{
    m_output = output;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::opt(const std::string &flag)
{
    m_flags.push_back(flag);
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::opt(const std::vector<std::string> &flags)
{
    m_flags.insert(m_flags.end(), flags.begin(), flags.end());
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::post(const std::string &cmd, const std::vector<std::string> &args)
{
    std::string pre;
    for (auto &arg : args)
        pre += "export " + arg + ";";

    m_oscmds.push_back(pre + cmd);
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::disregard(bool disregard)
{
    m_disregard = disregard;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::target(quixcc::TargetTriple target)
{
    m_target = target;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::verbose(bool verbose)
{
    m_verbose = verbose;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::debug(bool debug)
{
    m_debug = debug;
    return *this;
}

bool quixcc::CompilerBuilder::verify()
{
    if (m_files.empty() && !m_input)
        return false;

    if (!m_output)
        return false;

    return true;
}

LIB_CXX_EXPORT quixcc::Compiler quixcc::CompilerBuilder::build()
{
    if (!verify())
        throw std::runtime_error("Invalid compiler configuration.");

    std::vector<quixcc_job_t *> jobs;
    quixcc_job_t *job;

    if (m_input)
        m_files.push_back(std::make_pair(m_input, "stdin"));

    for (auto &entry : m_files)
    {
        if ((job = quixcc_new()) == nullptr)
            throw std::runtime_error("Failed to create new job.");
        if (m_debug)
            job->m_debug = true;

        if (m_verbose)
            quixcc_add_option(job, "-v", true);

        for (auto &flag : m_flags)
            quixcc_add_option(job, flag.c_str(), true);

        quixcc_set_input(job, entry.first, entry.second.c_str());

        if (!quixcc_set_triple(job, m_target.triple().c_str()))
            throw std::runtime_error("Failed to set target triple.");

        /// TODO: Implement other targets

        /*
            Handle this:
                src_0 -> job_0 -> out_0   ->
                src_1 -> job_1 -> out_1      ->
                                                 -> Custom linker command
                src_2 -> job_2 -> out_2      ->
                src_n -> job_n -> out_n   ->
        */

        quixcc_set_output(job, m_output);

        jobs.push_back(job);
    }

    return Compiler(jobs, m_oscmds);
}

///=============================================================================

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createLinter()
{
    return CompilerBuilder().opt("-emit-bc").disregard();
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createIR()
{
    return CompilerBuilder().opt("-emit-ir");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createIRBitcode()
{
    return CompilerBuilder().opt("-emit-bc");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createAssembly()
{
    return CompilerBuilder().opt("-S");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createObject()
{
    return CompilerBuilder().opt("-c");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createExecutable()
{
    return CompilerBuilder().opt("-O5");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createUpxCompressedExecutable()
{
    /// TODO: Implement UPX compression
    return CompilerBuilder().opt("-O5").opt("-fpack=upx");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createShellcode()
{
    /// TODO: Implement shellcode builder
    throw std::runtime_error("Shellcode builder not implemented.");
    return CompilerBuilder();
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createSharedLibrary()
{
    /// TODO: Implement shared library builder
    return CompilerBuilder().opt("-shared").opt("-fPIC").opt("-O5").post("ld -shared -o $QCC_OUTPUT $QCC_LAST");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createStaticLibrary()
{
    /// TODO: Implement static library builder
    return CompilerBuilder().opt("-c").opt("-O5").post("ar rcs $QCC_OUTPUT $QCC_LAST");
}

LIB_CXX_EXPORT quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createDocumentation()
{
    /// TODO: Implement documentation builder
    throw std::runtime_error("Documentation builder not implemented.");
    return CompilerBuilder();
}

///=============================================================================

LIB_CXX_EXPORT bool quixcc::Execute::exec(const std::string &code, std::ostream &messages)
{
    FILE *output = tmpfile();
    if (!output)
        return false;

    auto shellcode = quixcc::CompilerBuilderFactory::createShellcode()
                         .src(code.c_str(), code.size())
                         .out(output)
                         .build();

    shellcode.run(std::thread::hardware_concurrency())
        .puts(messages, messages);

    if (!shellcode.ok())
        return false;

    if (fseek(output, 0, SEEK_END) != 0)
        return false;
    size_t size = ftell(output);
    if (fseek(output, 0, SEEK_SET) != 0)
        return false;

    // Execute shellcode
    void *executable_memory = mmap(nullptr, size, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (executable_memory == MAP_FAILED)
        return false;

    if (fread(executable_memory, 1, size, output) != size)
    {
        munmap(executable_memory, size);
        return false;
    }

    ((void (*)())executable_memory)();

    munmap(executable_memory, size);
    fclose(output);

    return true;
}
