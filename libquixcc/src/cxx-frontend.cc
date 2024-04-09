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
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <quixcc.hpp>
#include <LibMacro.h>
#include <thread>
#include <fstream>
#include <filesystem>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>

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

LIB_CXX_EXPORT quixcc::CPU::CPU(const char *cpu)
{
    if (!cpu)
        throw CpuException("Invalid or unsupported LLVM CPU: (null)");

    if (cpu[0] == '\0')
    {
        m_cpu = llvm::sys::getHostCPUName().str();
        if (m_cpu.empty())
            m_cpu = "generic";
    }
    else
    {
        m_cpu = cpu; // cpu is not validated. idk how
    }
}

LIB_CXX_EXPORT quixcc::Compiler::Compiler(std::vector<quixcc_job_t *> jobs, std::set<FILE *> to_close)
{
    this->m_jobs = jobs;
    this->m_to_close = to_close;
    this->m_ok = false;
}

LIB_CXX_EXPORT quixcc::Compiler::~Compiler()
{
    for (auto job : this->m_jobs)
        quixcc_dispose(job);

    for (auto file : this->m_to_close)
        fclose(file);
}

LIB_CXX_EXPORT quixcc::Compiler &quixcc::Compiler::run(size_t max_threads)
{
    size_t i, tcount;
    std::vector<std::thread> threads;

    if (m_jobs.empty())
        return *this;

    if ((tcount = std::min(max_threads, this->m_jobs.size())) == 1)
    {
        quixcc_run(this->m_jobs[0]);
    }
    else
    {
        // Allocate jobs into threads
        for (i = 0; i < tcount; i++)
        {
            threads.push_back(std::thread([this, i, tcount]()
                                          {
                    for (size_t j = i; j < this->m_jobs.size(); j += tcount)
                        quixcc_run(this->m_jobs[j]); }));
        }

        for (auto &thread : threads)
            thread.join();
    }

    m_ok = true;

    // Collect messages
    for (auto job : this->m_jobs)
    {
        for (i = 0; i < job->m_result.m_count; i++)
        {
            auto feedback = job->m_result.m_messages[i];
            this->m_messages.push_back(std::make_pair(feedback->message, feedback->m_level));
        }

        if (!job->m_result.m_success)
            this->m_ok = false;
    }

    return *this;
}

LIB_CXX_EXPORT quixcc::Compiler &quixcc::Compiler::puts(std::ostream &normal, std::ostream &error)
{
    size_t i;

    for (auto job : this->m_jobs)
    {
        for (i = 0; i < job->m_result.m_count; i++)
        {
            auto feedback = job->m_result.m_messages[i];
            if (feedback->m_level < QUIXCC_ERROR)
                normal << feedback->message << std::endl;
            else
                error << feedback->message << std::endl;
        }
    }

    return *this;
}

///=============================================================================

LIB_CXX_EXPORT quixcc::CompilerBuilder::CompilerBuilder()
{
    m_verbose = Verbosity::NORMAL;
    m_target = ""; // Host target
    m_input = m_output = nullptr;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::add_source(const std::string &file)
{
    FILE *input;
    if ((input = fopen(file.c_str(), "r")) == nullptr)
        throw std::runtime_error("Failed to open input file: " + file);

    m_to_close.insert(input);
    m_files.push_back(std::make_pair(input, file));
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::add_code(const char *code, size_t size)
{
    m_input = fmemopen((void *)code, size, "r");
    if (!m_input)
        throw std::runtime_error("Failed to open input stream.");

    m_to_close.insert(m_input);
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::add_source(FILE *input)
{
    m_input = input;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::set_output(const std::string &file)
{
    if ((m_output = fopen(file.c_str(), "wb")) == nullptr)
        throw std::runtime_error("Failed to open output file: " + file);

    m_to_close.insert(m_output);
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::set_output(FILE *output)
{
    m_output = output;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::add_include(const std::string &dir)
{
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
        throw std::runtime_error("failed to access include directory: " + dir);

    m_flags.push_back("-I" + dir);

    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::add_library(const std::string &dir)
{
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
        throw std::runtime_error("failed to access library directory: " + dir);

    m_flags.push_back("-L" + dir);

    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::link_library(const std::string &lib)
{
    if (!std::filesystem::exists(lib) || !std::filesystem::is_directory(lib))
        throw std::runtime_error("failed to access library: " + lib);

    m_flags.push_back("-l" + lib);

    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::define(const std::string &name, const std::string &value)
{
    m_flags.push_back("-D" + name + "=" + value);
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::undefine(const std::string &name)
{
    m_flags.push_back("-U" + name);
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::set_flag(const std::string &flag)
{
    m_flags.push_back(flag);
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::set_verbosity(quixcc::Verbosity verbose)
{
    m_verbose = verbose;
    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::set_optimization(quixcc::OptimizationLevel optimize)
{
    switch (optimize)
    {
    case quixcc::OptimizationLevel::NONE:
        m_flags.push_back("-O0");
        break;
    case quixcc::OptimizationLevel::SPEED_1:
        m_flags.push_back("-O1");
        break;
    case quixcc::OptimizationLevel::SPEED_2:
        m_flags.push_back("-O2");
        break;
    case quixcc::OptimizationLevel::SPEED_3:
        m_flags.push_back("-O3");
        break;
    case quixcc::OptimizationLevel::SPEED_4:
        m_flags.push_back("-O3");
        m_flags.push_back("-flto");
        break;
    case quixcc::OptimizationLevel::SIZE:
        m_flags.push_back("-Os");
        break;
    }

    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::set_debug(bool debug)
{
    if (debug)
        m_flags.push_back("-g");

    return *this;
}

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::opt(const std::string &flag)
{
    m_flags.push_back(flag);
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

LIB_CXX_EXPORT quixcc::CompilerBuilder &quixcc::CompilerBuilder::cpu(quixcc::CPU cpu)
{
    m_cpu = cpu;
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

        switch (m_verbose)
        {
        case Verbosity::NORMAL:
            break;
        case Verbosity::VERBOSE:
            quixcc_add_option(job, "-v", true);
            break;
        case Verbosity::VERY_VERBOSE:
            job->m_debug = true;
            quixcc_add_option(job, "-v", true);
            break;
        }

        for (auto &flag : m_flags)
            quixcc_add_option(job, flag.c_str(), true);

        quixcc_set_input(job, entry.first, entry.second.c_str());

        if (!quixcc_set_triple(job, m_target.triple().c_str()))
            throw TargetTripleException("Invalid or unsupported LLVM target triple: " + m_target.triple());

        if (!quixcc_set_cpu(job, m_cpu.cpu().c_str()))
            throw CpuException("Invalid or unsupported LLVM CPU: " + m_cpu.cpu());

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

    return Compiler(jobs, m_to_close);
}
