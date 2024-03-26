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
#include <thread>
#include <fstream>

#include <sys/mman.h>

quixcc::Compiler::Compiler(std::vector<quixcc_job_t *> jobs)
{
    this->m_jobs = jobs;
}

quixcc::Compiler::~Compiler()
{
    for (auto job : this->m_jobs)
        quixcc_dispose(job);
}

quixcc::Compiler &quixcc::Compiler::run(size_t max_threads)
{
    std::vector<std::thread> threads;

    size_t thread_count = std::min(max_threads, this->m_jobs.size());

    // Allocate jobs into threads
    for (size_t i = 0; i < thread_count; i++)
    {
        threads.push_back(std::thread([this, i, thread_count]()
                                      {
            for (size_t j = i; j < this->m_jobs.size(); j += thread_count)
            {
                quixcc_run(this->m_jobs[j]);
            } }));
    }

    // Wait for all threads to finish
    for (auto &thread : threads)
        thread.join();

    // Collect messages
    for (auto job : this->m_jobs)
    {
        for (size_t i = 0; i < job->m_result->m_feedback.m_count; i++)
        {
            auto feedback = job->m_result->m_feedback.m_messages[i];
            this->m_messages.push_back(std::make_pair(feedback->message, feedback->m_level));
        }
    }

    return *this;
}

const std::vector<std::pair<std::string, enum quixcc_msg_level_t>> &quixcc::Compiler::messages() const
{
    return this->m_messages;
}

quixcc::Compiler &quixcc::Compiler::puts(std::ostream &normal, std::ostream &error)
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

bool quixcc::Compiler::ok() const
{
    for (auto job : this->m_jobs)
        if (!job->m_result->m_success)
            return false;

    return true;
}

///=============================================================================

quixcc::CompilerBuilder::CompilerBuilder()
{
    m_verbose = false;
    m_target = Target::EXECUTABLE;
}

quixcc::CompilerBuilder::~CompilerBuilder()
{
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::in(const std::string &file)
{
    FILE *input;
    if ((input = fopen(file.c_str(), "r")) == nullptr)
        throw std::runtime_error("Failed to open input file: " + file);

    m_files.push_back(std::make_pair(input, file));
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::in(const std::vector<std::string> &files)
{
    for (auto &file : files)
        in(file);

    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::src(const char *code, size_t size)
{
    m_input = fmemopen((void *)code, size, "r");
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::src(FILE *input)
{
    m_input = std::move(input);
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::out(const std::string &file)
{
    if ((m_output = fopen(file.c_str(), "w")) == nullptr)
        throw std::runtime_error("Failed to open output file: " + file);

    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::out(FILE *output)
{
    m_output = std::move(output);
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::opt(const std::string &flag)
{
    m_flags.push_back(flag);
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::opt(const std::string &flag, const std::string &value)
{
    m_flags.push_back(flag + "=" + value);
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::opt(const std::vector<std::string> &flags)
{
    m_flags.insert(m_flags.end(), flags.begin(), flags.end());
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::opt(const std::vector<std::pair<std::string, std::string>> &flags)
{
    for (auto &flag : flags)
        m_flags.push_back(flag.first + "=" + flag.second);

    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::target(quixcc::Target target)
{
    m_target = target;
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::verbose(bool verbose)
{
    m_verbose = verbose;
    return *this;
}

quixcc::CompilerBuilder &quixcc::CompilerBuilder::debug(bool debug)
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

quixcc::Compiler quixcc::CompilerBuilder::build()
{
    std::vector<quixcc_job_t *> jobs;
    quixcc_job_t *job;

    for (auto &[_FILE, name] : m_files)
    {
        if ((job = quixcc_new()) == nullptr)
            throw std::runtime_error("Failed to create new job.");
        if (m_debug)
            job->m_debug = true;

        if (m_verbose)
            quixcc_add_option(job, "-v", true);

        for (auto &flag : m_flags)
            quixcc_add_option(job, flag.c_str(), true);

        quixcc_set_input(job, _FILE, name.c_str());

        switch (m_target)
        {
            /// TODO: Implement other targets
        default:
            break;

        /*
            Handle this:
                src_0 -> job_0 -> out_0   ->
                src_1 -> job_1 -> out_1      ->
                                                 -> Custom linker command
                src_2 -> job_2 -> out_2      ->  
                src_n -> job_n -> out_n   ->  
        */
        }
    }

    return Compiler(jobs);
}

///=============================================================================

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createLinter()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createIR()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createIRBitcode()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createAssembly()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createObject()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createExecutable()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createUpxCompressedExecutable()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createShellcode()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createSharedLibrary()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createStaticLibrary()
{
    return CompilerBuilder();
}

quixcc::CompilerBuilder quixcc::CompilerBuilderFactory::createDocumentation()
{
    return CompilerBuilder();
}

///=============================================================================

bool quixcc::Execute::exec(const std::string &code, std::ostream &messages)
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
