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

#ifndef __QUIXCC_HPP__
#define __QUIXCC_HPP__

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <iostream>
#include <quixcc.h>

namespace quixcc
{
    class TargetTripleException : public std::runtime_error
    {
    public:
        TargetTripleException(const std::string &msg) : std::runtime_error(msg) {}
    };

    class TargetTriple
    {
        std::string m_triple;

    public:
        TargetTriple(const std::string &triple = "") : TargetTriple(triple.c_str()) {}

        TargetTriple(const char *triple);

        const std::string &triple() const { return m_triple; }
    };

    enum class Verbosity
    {
        NORMAL = 0,
        VERBOSE = 1,
        VERY_VERBOSE = 2
    };

    enum class OptimizationLevel
    {
        NONE = 0,
        SIZE = 10,
        SPEED_1 = 30,
        SPEED_2 = 40,
        SPEED_3 = 50,
        SPEED_4 = 60,
    };

    class Compiler
    {
        std::vector<quixcc_job_t *> m_jobs;
        std::set<FILE *> m_to_close;
        std::vector<std::pair<std::string, enum quixcc_msg_level_t>> m_messages;
        bool m_ok;

        Compiler(const Compiler &) = delete;
        Compiler(Compiler &&) = delete;
        Compiler &operator=(const Compiler &) = delete;

    public:
        Compiler(std::vector<quixcc_job_t *> jobs, std::set<FILE *> to_close);

        ~Compiler();

        Compiler &run(size_t max_threads = 4);

        const std::vector<std::pair<std::string, enum quixcc_msg_level_t>> &messages() const { return m_messages; }

        Compiler &puts(std::ostream &normal = std::cout, std::ostream &error = std::cerr);

        bool ok() const { return m_ok; }
    };

    class CompilerBuilder
    {
        std::vector<std::pair<FILE *, std::string>> m_files;
        std::vector<std::string> m_flags;
        std::set<FILE *> m_to_close;
        FILE *m_input;
        FILE *m_output;
        Verbosity m_verbose;
        TargetTriple m_target;
        bool m_disregard;

    public:
        CompilerBuilder();
        ~CompilerBuilder() = default;

        CompilerBuilder &add_source(const std::string &file);
        CompilerBuilder &add_code(const char *code, size_t size);
        CompilerBuilder &add_source(FILE *input);

        CompilerBuilder &set_output(const std::string &file);
        CompilerBuilder &set_output(FILE *output);

        CompilerBuilder &add_include(const std::string &dir);
        CompilerBuilder &add_library(const std::string &dir);
        CompilerBuilder &link_library(const std::string &lib);
        CompilerBuilder &define(const std::string &name, const std::string &value);
        CompilerBuilder &undefine(const std::string &name);
        CompilerBuilder &set_flag(const std::string &flag);
        CompilerBuilder &set_verbosity(Verbosity verbose = Verbosity::NORMAL);
        CompilerBuilder &set_optimization(OptimizationLevel optimize = OptimizationLevel::SPEED_1);
        CompilerBuilder &set_debug(bool debug = true);

        CompilerBuilder &opt(const std::string &flag);
        CompilerBuilder &disregard(bool disregard = true);
        CompilerBuilder &target(TargetTriple target);

        bool verify();

        Compiler build();
    };
}

#endif // __QUIXCC_HPP__
