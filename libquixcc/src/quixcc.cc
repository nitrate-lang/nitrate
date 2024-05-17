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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
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

#include <quixcc.h>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstring>
#include <iostream>
#include <regex>
#include <filesystem>
#include <random>
#include <thread>
#include <signal.h>
#include <execinfo.h>

#include <LibMacro.h>
#include <llvm/LLVMWrapper.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <generate/Generate.h>
#include <lexer/Lex.h>
#include <preprocessor/Preprocesser.h>
#include <core/Logger.h>
#include <core/Exception.h>
#include <parse/Parser.h>
#include <libquixcc.h>
#include <mutate/Routine.h>

#include <IR/Q/QIR.h>
#include <IR/delta/DeltaIR.h>

#include <optimizer/Optimizer.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

#define PROJECT_REPO_URL "https://github.com/Kracken256/quixcc"

#define PRUNE_DEBUG_MESSAGES 1

using namespace libquixcc;

static std::atomic<bool> g_is_initialized = false;
std::atomic<uint64_t> g_num_of_contexts = 0;
std::mutex g_library_lock;

static void print_stacktrace();

[[noreturn]] static void quixcc_panic(std::string msg) noexcept
{
    msg = "LIBQUIXCC LIBRARY PANIC: " + msg;
    // Split msg into lines of `max` characters
    std::vector<std::string> lines;
    std::string line;
    size_t pos = 0, len = 0;
    const size_t max = 80;
    while (pos < msg.size())
    {
        len = std::min<size_t>(max - 4, msg.size() - pos);
        line = msg.substr(pos, len);

        if (line.size() < max - 4)
            line += std::string(max - 4 - line.size(), ' ');
        lines.push_back(line);
        pos += len;
    }

    std::string sep;
    for (size_t i = 0; i < max - 2; i++)
        sep += "━";

    std::cerr << "\x1b[31;1m┏" << sep << "┓\x1b[0m" << std::endl;
    for (auto &str : lines)
        std::cerr << "\x1b[31;1m┃\x1b[0m " << str << " \x1b[31;1m┃\x1b[0m" << std::endl;
    std::cerr << "\x1b[31;1m┗" << sep << "\x1b[31;1m┛\x1b[0m\n"
              << std::endl;

    print_stacktrace();

    abort();

    while (true)
        std::this_thread::yield();
}

static void *safe_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr)
        quixcc_panic("out of memory");

    return new_ptr;
}

static char *safe_strdup(const char *str)
{
    char *new_str = strdup(str);
    if (!new_str)
        quixcc_panic("out of memory");
    return new_str;
}

static quixcc_uuid_t quixcc_uuid()
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    static_assert((sizeof(quixcc_uuid_t::data) | sizeof(uuid.data)) == 16, "UUID type size mismatch");

    quixcc_uuid_t id;
    memcpy(id.data, uuid.data, sizeof(quixcc_uuid_t));

    return id;
}

LIB_EXPORT bool quixcc_init()
{
    /* We don't need to initialize more than once */
    if (g_is_initialized)
        return true;

    static std::mutex g_mutex;
    std::lock_guard<std::mutex> lock(g_mutex);

#ifdef LLVM_SUUPORT_ALL_TARGETS
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    /* Check if LLVM is initialized */
    if (llvm::TargetRegistry::targets().empty())
    {
        std::cerr << "error: LLVM initialization failed" << std::endl;
        return false;
    }
#else
#warning "Building LIBQUIXCC without support for ANY LLVM targets!!"
#endif

    g_is_initialized = true;
    return true;
}

LIB_EXPORT quixcc_job_t *quixcc_new()
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_new(). quitcc_new() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* Acquire a lock on the library state. This is for MT-safe cache management */
    std::lock_guard<std::mutex> lock(g_library_lock);

    /* Allocate a new job using raw pointers to be friendly with C */
    quixcc_job_t *job = new quixcc_job_t();
    job->m_id = quixcc_uuid();

    /* Clear structures */
    memset(&job->m_options, 0, sizeof(quixcc_options_t));
    memset(&job->m_result, 0, sizeof(quixcc_status_t));

    /* Clear all pointers & values */
    job->m_in = job->m_out = nullptr;
    job->m_filename = nullptr;
    job->m_priority = 0;
    job->m_debug = job->m_tainted = job->m_running = false;

    /* Set magic structure field */
    job->m_magic = JOB_MAGIC;

    g_num_of_contexts++;

    return job;
}

LIB_EXPORT bool quixcc_dispose(quixcc_job_t *job)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_dispose(). quixcc_dispose() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job)
        return false;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_dispose().");

    bool lockable = job->m_lock.try_lock();
    if (!lockable)
        return false;

    /* Free Options array */
    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        free((void *)job->m_options.m_options[i]);
        job->m_options.m_options[i] = nullptr;
    }
    if (job->m_options.m_options)
        free(job->m_options.m_options);
    memset(&job->m_options, 0, sizeof(quixcc_options_t));

    /* Free messages array */
    for (uint32_t i = 0; i < job->m_result.m_count; i++)
    {
        quixcc_msg_t *msg = job->m_result.m_messages[i];
        assert(msg);
        assert(msg->message);

        free((void *)msg->message);
        memset(msg, 0, sizeof(quixcc_msg_t));
        free(msg);
    }

    if (job->m_result.m_messages)
        free(job->m_result.m_messages);
    memset(&job->m_result, 0, sizeof(quixcc_status_t));

    if (job->m_filename)
    {
        free((void *)job->m_filename);
        job->m_filename = nullptr;
    }

    /* Clear all pointers & values */
    /* The FILE handles are owned by the caller; we don't close them */
    job->m_in = job->m_out = nullptr;
    job->m_priority = 0;
    job->m_debug = job->m_tainted = false;

    job->m_lock.unlock();

    /* Hopefully, this will cache library usage errors */
    job->m_magic = 0;

    delete job; // Destruct C++ object members implicitly

    g_num_of_contexts--;

    return true;
}

LIB_EXPORT void quixcc_option(quixcc_job_t *job, const char *opt, bool enable)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_option(). quixcc_option() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job || !opt)
        return;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_option().");

    std::lock_guard<std::mutex> lock(job->m_lock);

    /* Remove the option if it already exists */
    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        const char *option = job->m_options.m_options[i];
        if (strcmp(option, opt) == 0)
        {
            free((void *)option);

            job->m_options.m_options[i] = job->m_options.m_options[job->m_options.m_count - 1];
            job->m_options.m_options[job->m_options.m_count - 1] = nullptr;
            job->m_options.m_count--;
            break;
        }
    }

    if (enable)
    {
        /* Enable it */
        job->m_options.m_options = (const char **)safe_realloc(job->m_options.m_options, (job->m_options.m_count + 1) * sizeof(const char *));
        job->m_options.m_options[job->m_options.m_count++] = safe_strdup(opt);
    }
}

LIB_EXPORT void quixcc_source(quixcc_job_t *job, FILE *in, const char *filename)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_source(). quixcc_source() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job || !in || !filename)
        return;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_source().");

    std::lock_guard<std::mutex> lock(job->m_lock);

    /* Its the callers responsibility to make sure this is a valid file */
    if (fseek(in, 0, SEEK_SET) != 0)
        abort();

    job->m_in = in;
    job->m_filename = safe_strdup(filename);
}

LIB_EXPORT bool quixcc_target(quixcc_job_t *job, const char *_llvm_triple)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_target(). quixcc_target() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job || !_llvm_triple)
        return false;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_target().");

    std::lock_guard<std::mutex> lock(job->m_lock);

    std::string new_triple, err;

    try
    {
        /* An empty string means use the default target */
        if (_llvm_triple[0] == '\0')
            new_triple = llvm::sys::getDefaultTargetTriple();
        else
            new_triple = _llvm_triple;
    }
    catch (std::exception &)
    {
        return false;
    }

    if (llvm::TargetRegistry::lookupTarget(new_triple, err) == nullptr)
    {
        LOG(ERROR) << "invalid target triple: " << new_triple << std::endl;
        return false;
    }

    job->m_triple = new_triple;

    return true;
}

LIB_EXPORT bool quixcc_cpu(quixcc_job_t *job, const char *cpu)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_cpu(). quixcc_cpu() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job || !cpu)
        return false;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_cpu().");

    std::lock_guard<std::mutex> lock(job->m_lock);

    /// TODO: find a way to validate the CPU

    job->m_cpu = cpu;

    return true;
}

LIB_EXPORT void quixcc_output(quixcc_job_t *job, FILE *out, FILE **old_out)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_output(). quixcc_output() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job || !out)
        return;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_output().");

    std::lock_guard<std::mutex> lock(job->m_lock);

    if (old_out)
        *old_out = job->m_out;

    job->m_out = out;
}

static std::string get_datetime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

std::string base64_encode(const std::string &in)
{
    std::string out;

    int val = 0, valb = -6;
    for (unsigned char c : in)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

static bool quixcc_mutate_ast(quixcc_job_t *job, std::shared_ptr<AST> ast)
{
    Mutation mutator;
    mutator.add_routine(mutate::StripUnderscoreNames);    ///> Remove all constructs named '_'
    mutator.add_routine(mutate::MethodToFunc);            ///> Convert method calls to function calls
    mutator.add_routine(mutate::DiscoverNamedConstructs); ///> Map named constructs to their respective AST nodes
    mutator.add_routine(mutate::ResolveNamedConstructs);  ///> Resolve named constructs to their respective AST nodes
    mutator.add_routine(mutate::ConvertTypes);            ///> Perform implicit type conversions
    mutator.add_routine(mutate::FoldConstExpr);           ///> Fold constant expressions
    mutator.add_routine(mutate::ExtrapolateEnumFields);   ///> Derive enum field values
    mutator.add_routine(mutate::InferTypes);              ///> Type inference
    mutator.add_routine(mutate::ObjectConstruction);      ///> Object construction
    mutator.add_routine(mutate::ObjectDestruction);       ///> Object destruction
    mutator.add_routine(mutate::ImplicitReturn);          ///> Implicit return statements
    mutator.add_routine(mutate::SubsystemCollapse);       ///> Flatten AST by collapsing subsystems
    mutator.run(job, ast);

    return true;
}

static bool quixcc_qualify(quixcc_job_t *job, std::shared_ptr<AST> ast)
{
    /// TODO: implement semantic analysis
    return true;
}

static bool get_include_directories(quixcc_job_t *job, std::set<std::string> &dirs)
{
    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        std::string option = job->m_options.m_options[i];

        if (option.size() < 3)
            continue;

        if (option.starts_with("-I"))
        {
            dirs.insert(option.substr(2));
            continue;
        }
    }

    return true;
}

static bool verify_user_constant_string(const std::string &s)
{
    int state = 0;

    for (size_t i = 1; i < s.size() - 1; i++)
    {
        char c = s[i];

        switch (state)
        {
        case 0:
            if (c == '\\')
                state = 1;
            if (c == '"')
                return false;
            if (!std::isprint(c))
                return false;
            break;
        case 1:
            if (c == '"')
                state = 0;
            else if (c == '\\')
                state = 0;
            else if (c == 'n' || c == 'r' || c == 't' || c == '0' || c == 'x')
                state = 0;
            else
                return false;
            break;
        }
    }

    return state == 0;
}

static bool verify_user_constant(const std::string &key, const std::string &value)
{
    if (key.empty())
        return false;

    bool key_valid = std::isalpha(key[0]) || key[0] == '_';

    for (char c : key)
    {
        if (!std::isalnum(c) && c != '_')
        {
            key_valid = false;
            break;
        }
    }

    if (!key_valid)
        return false;

    if (value.empty())
        return true;

    if (value == "true" || value == "false")
        return true;

    bool is_int = true;
    for (char c : value)
    {
        if (!std::isdigit(c))
        {
            is_int = false;
            break;
        }
    }

    if (is_int)
        return true;

    return verify_user_constant_string(value);
}

static bool get_compile_time_user_constants(quixcc_job_t *job, std::map<std::string, std::string> &constants)
{
    auto argmap = job->m_argset;

    for (auto &arg : argmap)
    {
        if (arg.first.starts_with("-D"))
        {
            std::string key = arg.first.substr(2);
            std::string value = arg.second;

            if (!verify_user_constant(key, value))
            {
                LOG(ERROR) << "invalid user constant: " << key << std::endl;
                return false;
            }
            constants[key] = value;
        }
    }

    return true;
}

static bool get_env_constants(quixcc_job_t *job, std::map<std::string, std::string> &constants)
{
    for (char **env = environ; *env; env++)
    {
        std::string var = *env;
        if (var.find("QUIXCC_VAR_") == 0)
        {
            size_t pos = var.find('=');
            if (pos != std::string::npos)
            {
                std::string key = var.substr(4, pos - 4);
                std::string value = var.substr(pos + 1);

                if (!verify_user_constant(key, value))
                {
                    LOG(ERROR) << "invalid user constant: " << key << std::endl;
                    return false;
                }
                constants[key] = value;
            }
        }
    }

    return true;
}

static bool preprocess_phase(quixcc_job_t *job, std::shared_ptr<PrepEngine> prep)
{
    std::set<std::string> dirs;
    std::map<std::string, std::string> constants;

    if (!get_include_directories(job, dirs))
    {
        LOG(ERROR) << "failed to get include directories" << std::endl;
        return false;
    }
    for (auto &dir : dirs)
        prep->addpath(dir);
    if (!get_compile_time_user_constants(job, constants))
    {
        LOG(ERROR) << "failed to get compile time user constants" << std::endl;
        return false;
    }
    for (auto &constant : constants)
        prep->set_static(constant.first, constant.second);

    if (!get_env_constants(job, constants))
    {
        LOG(ERROR) << "failed to get environment constants" << std::endl;
        return false;
    }
    for (auto &constant : constants)
        prep->set_static(constant.first, constant.second);

    if (!prep->set_source(job->m_in, job->m_filename))
    {
        LOG(ERROR) << "failed to set source" << std::endl;
        return false;
    }

    return true;
}

static bool compile(quixcc_job_t *job)
{
    auto ast = std::make_shared<AST>();

    if (job->m_argset.contains("-PREP"))
    {
        LOG(DEBUG) << "Preprocessing only" << std::endl;
        std::shared_ptr<PrepEngine> prep = std::make_shared<PrepEngine>(*job);
        prep->setup();
        if (!preprocess_phase(job, prep))
            return false;

        // Generate output
        std::string tmp;
        std::optional<Token> tok;
        while ((tok = prep->next())->type() != TT::Eof)
        {
            tmp = tok->serialize();
            fwrite(tmp.c_str(), 1, tmp.size(), job->m_out);
            fputc('\n', job->m_out);
        }
        fflush(job->m_out);
        return true;
    }

    if (job->m_argset.contains("-emit-tokens"))
    {
        LOG(DEBUG) << "Lexing only" << std::endl;
        StreamLexer lexer;

        if (!lexer.set_source(job->m_in, job->m_filename))
        {
            LOG(ERROR) << "failed to set source" << std::endl;
            return false;
        }

        // Generate output
        std::string tmp;
        Token tok;
        while ((tok = lexer.next()).type() != TT::Eof)
        {
            tmp = tok.serialize();
            fwrite(tmp.c_str(), 1, tmp.size(), job->m_out);
            fputc('\n', job->m_out);
        }
        fflush(job->m_out);
        return true;
    }

    ///=========================================
    /// BEGIN: PREPROCESSOR/LEXER
    auto prep = std::make_shared<PrepEngine>(*job);
    LOG(DEBUG) << "Preprocessing source" << std::endl;
    prep->setup();
    if (!preprocess_phase(job, prep))
        return false;
    LOG(DEBUG) << "Finished preprocessing source" << std::endl;
    /// END:   PREPROCESSOR/LEXER
    ///=========================================

    ///=========================================
    /// BEGIN: PARSER
    LOG(DEBUG) << "Building AST 1" << std::endl;
    if (!parse(*job, prep, ast, false))
        return false;
    LOG(DEBUG) << "Finished building AST 1" << std::endl;
    if (job->m_debug)
        LOG(DEBUG) << "Dumping AST 1 (JSON): " << base64_encode(ast->to_json()) << std::endl;
    /// END:   PARSER
    ///=========================================

    ///=========================================
    /// BEGIN: INTERMEDIATE PROCESSING
    if (!quixcc_mutate_ast(job, ast) || job->m_tainted)
        return false;
    if (job->m_debug)
        LOG(DEBUG) << "Dumping AST 2 (JSON): " << base64_encode(ast->to_json()) << std::endl;
    /// END:   INTERMEDIATE PROCESSING
    ///=========================================

    if (!job->m_argset.contains("-fno-check")) // -fno-check disables semantic analysis
    {
        ///=========================================
        /// BEGIN: SEMANTIC ANALYSIS
        LOG(DEBUG) << "Performing semantic analysis" << std::endl;
        if (!quixcc_qualify(job, ast))
        {
            LOG(ERROR) << "failed to verify AST" << std::endl;
            return false;
        }
        LOG(DEBUG) << "Finished semantic analysis" << std::endl;
        /// END:   SEMANTIC ANALYSIS
        ///=========================================
    }

    ///=========================================
    /// BEGIN: OPTIMIZATION PIPELINE
    auto QIR = std::make_unique<ir::q::QModule>(job->m_filename);
    if (!QIR->from_ast(std::move(ast))) /* This will modify the AST */
        return false;

    if (!job->m_argset.contains("-O0"))
    {
        /* Apply architecural optimizations
         * - Transform single threaded code into multi-threaded code
         * - Recognize common structural patterns and replace them with optimized equivalents (remove polymorphism, etc.)
         * - Break large functions into smaller functions
         * - Move large code segments into dynamic library bundles
         */
        if (job->m_argset.contains("-OA") && !optimizer::optimize_architecture(QIR))
            return false;

        /* Apply behavioral optimizations
         * - Replace math with SIMD instructions
         * - Algebraic reduction
         * - Remove redundant locks
         * - Auto-generate comparison methods for user defined types
         * - Remove redundant threads
         * - Remove redundant memory allocations
         * - Replace allocators with flyweight patterns
         * - Replace heap with stack whenever possible
         * - Replace stack with static memory whenever possible
         * - Replace static memory with compile time constants whenever possible
         * - If allowed, install huristics into opaque algorithms
         * - Replace opaque algorithms with specialized algorithms
         * - Replace out-of-band messaging with direct functional messaging
         * - Replace exceptions with composite return values
         * - Eliminate dynamic runtime deallocations (garbage collection) whenever possible
         */
        if (job->m_argset.contains("-OB") && !optimizer::optimize_behavior(QIR))
            return false;

        /* Apply general optimizations
         * - Replace stringy code with enums
         * - Native TypeSize optimizations
         * - Generator unrolling
         * - Cache the results of complex pure functions
         * - Replace pure functions with lookup tables
         * - Remove trivially constructable unused allocations/deallocations
         * - Replace non-trivially constructable unused allocations with direct constructor calls
         * - ML-smart branch prediction / rearangement
         * - Don't construct fields that are unused
         */
        if (job->m_argset.contains("-OG") && !optimizer::optimize_general(QIR))
            return false;
    }

    auto DIR = std::make_unique<ir::delta::IRDelta>(job->m_filename);
    if (!DIR->from_qir(QIR))
        return false;
    /// END:   OPTIMIZATION PIPELINE
    ///=========================================

    ///=========================================
    /// BEGIN: GENERATOR
    LOG(DEBUG) << "Generating output" << std::endl;
    if (!generate(*job, DIR)) /* Apply LLVM optimizations */
    {
        LOG(ERROR) << "failed to generate output" << std::endl;
        return false;
    }
    /// END: GENERATOR
    ///=========================================

    fflush(job->m_out);
    return true;
}

static bool verify_build_option(const std::string &option, const std::string &value)
{
    const static std::set<std::string> static_options = {
        "-S",           // assembly output
        "-PREP",        // preprocessor/Lexer output
        "-emit-tokens", // lexer output (no preprocessing)
        "-emit-ir",     // IR output
        "-emit-c11",    // C11 output
        "-emit-bc",     // bitcode output
        "-c",           // compile only
        "-O0",          // optimization levels
        "-O1",          // optimization levels
        "-O2",          // optimization levels
        "-O3",          // optimization levels
        "-Os",          // optimization levels
        "-g",           // debug information
        "-flto",        // link time optimization
        "-fPIC",        // position independent code
        "-fPIE",        // position independent executable
        "-v",           // verbose
    };
    const static std::vector<std::pair<std::regex, std::regex>> static_regexes = {
        {std::regex("-D[a-zA-Z_][a-zA-Z0-9_]*"), std::regex("[a-zA-Z0-9_ ]*")},
        {std::regex("-I.+"), std::regex("")},
    };

    if (static_options.contains(option))
        return true;

    for (auto &regex : static_regexes)
    {
        if (std::regex_match(option, regex.first) && std::regex_match(value, regex.second))
            return true;
    }

    return false;
}

static bool verify_build_option_conflicts(quixcc_job_t *job)
{
    (void)job;
    return true;
}

static bool build_argmap(quixcc_job_t *job)
{
    const static std::set<char> okay_prefixes = {'f', 'O', 'P', 'I', 'e', 'D', 'W', 'm', 'c', 'S', 'g', 's', 'v'};

    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        std::string option = job->m_options.m_options[i];

        if (option.size() == 2 && option[0] == '-' && !okay_prefixes.contains(option[1]))
        {
            LOG(ERROR) << "invalid build option: " << option << std::endl;
            return false;
        }

        size_t pos = option.find('=');
        std::string key = option.substr(0, pos);
        std::string value;

        if (pos != std::string::npos)
            value = option.substr(pos + 1);

        if (!verify_build_option(key, value))
        {
            LOG(ERROR) << "invalid build option: " << key << std::endl;
            return false;
        }

        if (key == "-v" && value.empty())
            job->m_debug = true;

        job->m_argset.insert({key, value});

        if (!value.empty())
            LOG(DEBUG) << "Added switch entry: " << key << "=" << value << std::endl;
        else
            LOG(DEBUG) << "Added switch entry: " << key << std::endl;
    }

    return verify_build_option_conflicts(job);
}

static void print_stacktrace()
{
    // UTF-8 support
    setlocale(LC_ALL, "");

    std::cerr << "\x1b[31;1m┏━━━━━━┫ INTERNAL COMPILER ERROR ┣━━\x1b[0m\n";
    std::cerr << "\x1b[31;1m┃\x1b[0m\n";

    void *array[48];
    size_t size = backtrace(array, 48);
    char **strings = backtrace_symbols(array, size);

    for (size_t i = 0; i < size && strings[i]; i++)
        std::cerr << "\x1b[31;1m┣╸╸\x1b[0m \x1b[37;1m" << strings[i] << "\x1b[0m\n";

    free(strings);

    std::cerr << "\x1b[31;1m┃\x1b[0m\n";
    std::cerr << "\x1b[31;1m┗━━━━━━┫ END STACK TRACE ┣━━\x1b[0m" << std::endl;
}

static std::string escape_json_string(const std::string &s)
{
    std::string out;
    for (char c : s)
    {
        switch (c)
        {
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\"':
            out += "\\\"";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

static std::string geterror_report_string()
{
    std::vector<std::string> trace;

    void *array[48];
    size_t size = backtrace(array, 48);
    char **strings = backtrace_symbols(array, size);

    for (size_t i = 0; i < size && strings[i]; i++)
        trace.push_back(strings[i]);

    free(strings);

    std::string report = "{\"version\":\"1.0\",\"quixcc_run\":\"";

    char buf[17];
    snprintf(buf, sizeof(buf), "%p", (void *)quixcc_run);
    report += buf;

    report += "\",\"trace\":[";
    for (size_t i = 0; i < trace.size(); i++)
    {
        report += "\"" + escape_json_string(trace[i]) + "\"";
        if (i + 1 < trace.size())
            report += ",";
    }

    report += "]}";

    return "QUIXCC_TRACE-{" + base64_encode(report) + "}";
}

static void print_general_fault_message()
{
    std::cerr << "The compiler (libquixcc backend) encountered a fatal internal error.\n";
    std::cerr << "Please report this error to the QuixCC developers at " PROJECT_REPO_URL ".\n\n";
    std::cerr << "Please include the following report code: \n  " << geterror_report_string() << std::endl;
}

void quixcc_fault_handler(int sig)
{
    /*
        Lock all threads to prevent multiple error messages
    */
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    signal(SIGINT, SIG_IGN);
    signal(SIGILL, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGABRT, SIG_IGN);

    switch (sig)
    {
    case SIGINT:
        std::cerr << "SIGINT: libquixcc was interrupted. compilation aborted." << std::endl;
        break;
    case SIGILL:
        std::cerr << "SIGILL: libquixcc tried to execute an illegal instruction. compilation aborted." << std::endl;
        break;
    case SIGFPE:
        std::cerr << "SIGFPE: libquixcc tried to execute an illegal floating point operation. compilation aborted." << std::endl;
        break;
    case SIGSEGV:
        std::cerr << "SIGSEGV: libquixcc tried to access an invalid memory location leading to a segmentation fault. compilation aborted." << std::endl;
        break;
    case SIGTERM:
        std::cerr << "SIGTERM: libquixcc was terminated. compilation aborted." << std::endl;
        break;
    case SIGABRT:
        std::cerr << "SIGABRT: libquixcc encountered an internal error. compilation aborted." << std::endl;
        break;
    default:
        std::cerr << "libquixcc encountered an unexpected signal. compilation aborted." << std::endl;
        break;
    }

    std::cerr << '\n';
    print_general_fault_message();
    std::cerr << "\n";
    print_stacktrace();
    std::cerr << "\n";
    std::cerr << "The compiler will now abort." << std::endl;

    abort();
}

static bool execute_job(quixcc_job_t *job)
{
    if (!job->m_in || !job->m_out || !job->m_filename)
        return false;

    LoggerConfigure(*job);

    job->m_inner.setup(job->m_filename);

    LOG(DEBUG) << "Starting quixcc run @ " << get_datetime() << std::endl;

    if (!build_argmap(job))
    {
        LOG(ERROR) << "failed to build argmap" << std::endl;
        return false;
    }

    bool success = false;
    try
    {
        if (compile(job))
        {
            LOG(DEBUG) << "Compilation successful" << std::endl;
            success = true;
        }
        else
        {
            LOG(ERROR) << "Compilation failed" << std::endl;
        }
    }
    catch (ProgrammaticPreprocessorException &)
    {
        LOG(FAILED) << "Compilation was programmatically aborted while preprocessing source" << std::endl;
    }
    catch (PreprocessorException &)
    {
        LOG(FAILED) << "Compilation was aborted while preprocessing source" << std::endl;
    }
    catch (ParseException &)
    {
        LOG(FAILED) << "Compilation was aborted while parsing source" << std::endl;
    }
    catch (Exception &)
    {
        LOG(FAILED) << "Compilation failed" << std::endl;
    }

    LOG(DEBUG) << "Finished quixcc run @ " << get_datetime() << std::endl;

    return job->m_result.m_success = success;
}

LIB_EXPORT bool quixcc_run(quixcc_job_t *job)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_run(). quixcc_run() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job)
        return false;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_run().");

    std::lock_guard<std::mutex> lock(job->m_lock);

    /* Install signal handlers to catch fatal memory errors */
    sighandler_t old_handlers[6];
    old_handlers[0] = signal(SIGINT, quixcc_fault_handler);
    old_handlers[1] = signal(SIGILL, quixcc_fault_handler);
    old_handlers[2] = signal(SIGFPE, quixcc_fault_handler);
    old_handlers[3] = signal(SIGSEGV, quixcc_fault_handler);
    old_handlers[4] = signal(SIGTERM, quixcc_fault_handler);
    old_handlers[5] = signal(SIGABRT, quixcc_fault_handler);

    /* Every compiler job must have its own thread-local storage */
    bool success = false;
    std::thread t([&]
                  { success = execute_job(job); });
    t.join();

    /* Restore signal handlers */
    signal(SIGINT, old_handlers[0]);
    signal(SIGILL, old_handlers[1]);
    signal(SIGFPE, old_handlers[2]);
    signal(SIGSEGV, old_handlers[3]);
    signal(SIGTERM, old_handlers[4]);
    signal(SIGABRT, old_handlers[5]);

    return success;
}

LIB_EXPORT const quixcc_status_t *quixcc_status(quixcc_job_t *job)
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_status(). quixcc_status() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!job)
        return nullptr;

    /* User may have passed an invalid job pointer */
    if (job->m_magic != JOB_MAGIC) /* We can't handle this, so panic */
        quixcc_panic("A libquixcc library contract violation occurred: An invalid job pointer was passed to quixcc_status().");

    bool lockable = job->m_lock.try_lock();
    if (!lockable)
        return nullptr;

    quixcc_status_t *result = &job->m_result;

#if PRUNE_DEBUG_MESSAGES
    // Remove debug messages
    uint32_t i = 0;
    while (i < result->m_count)
    {
        quixcc_msg_t *msg = result->m_messages[i];
        if (msg->m_level != QUIXCC_DEBUG)
        {
            i++;
            continue;
        }

        if (msg->message)
            free((void *)msg->message);
        msg->message = nullptr;
        free(msg);

        memmove(&result->m_messages[i], &result->m_messages[i + 1], (result->m_count - i - 1) * sizeof(quixcc_msg_t *));
        result->m_count--;
    }
#endif

    job->m_lock.unlock();

    return result;
}

LIB_EXPORT char **quixcc_compile(FILE *in, FILE *out, const char *options[])
{
    if (!g_is_initialized && !quixcc_init())
    {
        quixcc_panic("A libquixcc library contract violation occurred: A successful call to quixcc_init() is required before calling quixcc_compile(). quixcc_compile() attempted to compensate for this error, but quitcc_init() failed to initialize.");
    }

    /* no-op */
    if (!in || !out)
        return nullptr;

    quixcc_job_t *job = quixcc_new();

    /* No need for locks here */
    quixcc_source(job, in, "stdin");
    quixcc_output(job, out, nullptr);

    /* Set options */
    if (options)
    {
        for (uint32_t i = 0; options[i]; i++)
            quixcc_option(job, options[i], true);
    }

    /* Run the job */
    if (quixcc_run(job))
    {
        quixcc_dispose(job);
        return nullptr; // success
    }

    /* Copy messages */
    char **messages = (char **)malloc((job->m_result.m_count + 1) * sizeof(char *));
    for (uint32_t i = 0; i < job->m_result.m_count; i++)
        messages[i] = safe_strdup(job->m_result.m_messages[i]->message);
    messages[job->m_result.m_count] = nullptr;

    quixcc_dispose(job);
    return messages;
}