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

#include <quixcc.h>
#include <memory>
#include <string>
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

#include <IR/IRModule.h>

#define PROJECT_REPO_URL "https://github.com/Kracken256/quixcc"

using namespace libquixcc;

static void *safe_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr)
    {
        std::cerr << "error: out of memory" << std::endl;
        exit(1);
    }
    return new_ptr;
}

static char *safe_strdup(const char *str)
{
    char *new_str = strdup(str);
    if (!new_str)
    {
        std::cerr << "error: out of memory" << std::endl;
        exit(1);
    }
    return new_str;
}

static quixcc_uuid_t quixcc_uuid()
{
    /*
        I'm not sure if std::random_device is thread safe, so I'm using a mutex to lock it.
    */
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    union
    {
        uint8_t bytes[16];
        struct
        {
            uint64_t m_low;
            uint64_t m_high;
        } __attribute__((packed)) m;
    } __attribute__((packed)) raw;

    // Generate non-deteministic random number
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);
    raw.m.m_high = dis(gen);
    raw.m.m_low = dis(gen);

    return {.m_low = raw.m.m_low, .m_high = raw.m.m_high};
}

LIB_EXPORT bool quixcc_init()
{
    static bool g_is_initialized = false;
    static std::mutex g_mutex;

    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_is_initialized)
        return true;

#ifdef LLVM_SUUPORT_ALL_TARGETS
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
#endif

    g_is_initialized = true;
    return true;
}

LIB_EXPORT quixcc_job_t *quixcc_new()
{
    quixcc_job_t *job = new quixcc_job_t();
    job->m_id = quixcc_uuid();

    job->m_options.m_count = 0;
    job->m_options.m_options = nullptr;
    memset(&job->m_result, 0, sizeof(quixcc_result_t));
    job->m_in = job->m_out = nullptr;
    job->m_filename = nullptr;
    job->m_priority = 0;
    job->m_debug = job->m_tainted = false;

    return job;
}

LIB_EXPORT bool quixcc_dispose(quixcc_job_t *job)
{
    if (!job)
        return false;

    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        free((void *)job->m_options.m_options[i]);
        job->m_options.m_options[i] = nullptr;
    }

    if (job->m_options.m_options)
        free(job->m_options.m_options);
    job->m_options.m_count = 0;
    job->m_options.m_options = nullptr;

    for (uint32_t i = 0; i < job->m_result.m_count; i++)
    {
        quixcc_msg_t *msg = job->m_result.m_messages[i];
        if (!msg)
            continue;

        if (msg->message)
            free((void *)msg->message);

        msg->message = nullptr;
        free(msg);

        job->m_result.m_messages[i] = nullptr;
    }

    if (job->m_result.m_messages)
        free(job->m_result.m_messages);
    job->m_result.m_count = 0;
    job->m_result.m_messages = nullptr;

    if (job->m_filename)
    {
        free((void *)job->m_filename);
        job->m_filename = nullptr;
    }

    delete job;

    return true;
}

LIB_EXPORT void quixcc_add_option(quixcc_job_t *job, const char *opt, bool enabled)
{
    if (!job || !opt || !enabled)
        return;

    job->m_options.m_options = (const char **)safe_realloc(job->m_options.m_options, (job->m_options.m_count + 1) * sizeof(const char *));
    job->m_options.m_options[job->m_options.m_count++] = safe_strdup(opt);
}

LIB_EXPORT void quixcc_remove_option(quixcc_job_t *job, const char *opt)
{
    if (!job || !opt)
        return;

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
}

LIB_EXPORT void quixcc_set_input(quixcc_job_t *job, FILE *in, const char *filename)
{
    if (!job || !in || !filename)
        return;

    job->m_in = in;
    job->m_filename = safe_strdup(filename);
}

LIB_EXPORT bool quixcc_set_triple(quixcc_job_t *job, const char *_triple)
{
    if (!job || !_triple)
        return false;

    if (!quixcc_triple(_triple))
        return false;

    job->m_triple = _triple;

    return true;
}

LIB_EXPORT bool quixcc_triple(const char *_triple)
{
    if (!_triple)
        return false;

    try
    {
        if (_triple[0] == '\0')
            return !llvm::sys::getDefaultTargetTriple().empty();

        std::string err;
        return llvm::TargetRegistry::lookupTarget(_triple, err) != nullptr;
    }
    catch (std::exception &)
    {
        return false;
    }
}

LIB_EXPORT bool quixcc_set_cpu(quixcc_job_t *job, const char *cpu)
{
    if (!job || !cpu)
        return false;

    /// TODO: find a way to validate the CPU
    /*
    if (!quixcc_cpu(cpu))
        return false;
    */

    job->m_cpu = cpu;

    return true;
}

LIB_EXPORT void quixcc_set_output(quixcc_job_t *job, FILE *out)
{
    if (!job || !out)
        return;

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

static std::string base64_encode(const std::string &in)
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
    mutator.add_routine(mutate::SubsystemCollapse);       ///> Flatten AST by collapsing subsystems
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
    mutator.run(job, ast);

    return true;
}

static bool quixcc_verify_semantics(quixcc_job_t *job, std::shared_ptr<AST> ast)
{
    (void)job;
    (void)ast;

    /*
     * TODO: Implement function argument type verification
     * TODO: Implement function return type verification
     */

    /* old stuff
        /// TODO: Verify that all identifiers are defined
        /// TODO: Verify that all types are defined
        /// TODO: Verify that all functions are defined
        /// TODO: Type checking
        /// TODO: Integer overflow/underflow checking
        /// TODO: Array bounds checking
        /// TODO: NULL value checking
        /// TODO: Veirfy struct/union members are non cyclic
        /// TODO: Verify mutability of variables
        /// TODO: Item alignment checking
        /// TODO: verify definitions match declarations
        /// TODO: identifiers don't conflict with reserved words
    */

    return true;
}

static bool quixcc_optimize_ast(quixcc_job_t *job, std::shared_ptr<AST> ast)
{
    (void)job;
    (void)ast;

    /// TODO: Argument by-value to const reference optimization
    /// TODO: Data flow analysis
    ///   - compile-time immutable blobs to const data
    ///   - function folding via effect engine
    ///   - ControlFlowGraph reduction via effect engine
    ///   - Smart dead code elimination via effect engine
    ///   - Standard library aware optimizations/eliminations
    ///      - Heap allocation elimination via metastructure generation
    ///      - String concatenation to constant string
    ///      - String concatenation to optimized metaclass builder
    ///   - Lookup table generation
    /// TODO: Detect common algorithms implementation and replace with standard library optimized version
    /// TODO: Try-catch block elimination via deep inspection of the AST
    /// TODO:

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

static void reduce_named_mappings(quixcc_job_t &ctx)
{
    auto it = ctx.m_inner.m_named_construsts.begin();

    while (it != ctx.m_inner.m_named_construsts.end())
    {
        switch (it->first.first)
        {
        case NodeType::GroupDefNode:
        {
            auto group = std::static_pointer_cast<GroupDefNode>(it->second);
            ctx.m_inner.m_named_construsts[std::make_pair(NodeType::StructDefNode, it->first.second)] = group->to_struct_def();
            it = ctx.m_inner.m_named_construsts.erase(it);
            break;
        }
        case NodeType::RegionDefNode:
        {
            auto region = std::static_pointer_cast<RegionDefNode>(it->second);
            ctx.m_inner.m_named_construsts[std::make_pair(NodeType::StructDefNode, it->first.second)] = region->to_struct_def();
            it = ctx.m_inner.m_named_construsts.erase(it);
            break;
        }
        default:
            it++;
            break;
        }
    }
}

static bool compile(quixcc_job_t *job)
{
    // Create an AST before goto statements
    // AST ast;
    std::shared_ptr<AST> ast = std::make_shared<AST>();

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
        while ((tok = prep->next())->type() != TokenType::Eof)
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
        while ((tok = lexer.next()).type() != TokenType::Eof)
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
    ///=========================================
    std::shared_ptr<PrepEngine> prep = std::make_shared<PrepEngine>(*job);
    LOG(DEBUG) << "Preprocessing source" << std::endl;
    prep->setup();
    if (!preprocess_phase(job, prep))
        return false;
    LOG(DEBUG) << "Finished preprocessing source" << std::endl;
    ///=========================================
    /// END: PREPROCESSOR/LEXER
    ///=========================================

    ///=========================================
    /// BEGIN: PARSER
    ///=========================================
    LOG(DEBUG) << "Building AST 1" << std::endl;
    if (!parse(*job, prep, ast, false))
        return false;
    LOG(DEBUG) << "Finished building AST 1" << std::endl;
    if (job->m_debug)
        LOG(DEBUG) << "Dumping AST 1 (JSON): " << base64_encode(ast->to_json()) << std::endl;
    ///=========================================
    /// END: PARSER
    ///=========================================

    ///=========================================
    /// BEGIN: INTERMEDIATE PROCESSING
    ///=========================================
    if (!quixcc_mutate_ast(job, ast) || job->m_tainted)
        return false;

    if (!quixcc_verify_semantics(job, ast) || job->m_tainted)
        return false;

    if (!quixcc_optimize_ast(job, ast) || job->m_tainted)
        return false;

    if (job->m_debug)
        LOG(DEBUG) << "Dumping AST 2 (JSON): " << base64_encode(ast->to_json()) << std::endl;

    ///=========================================
    /// END: INTERMEDIATE PROCESSING
    ///=========================================

    ///=========================================
    /// BEGIN: AST REDUCTION
    ///=========================================

    ReductionState state;
    auto ast_reduced = ast->reduce(state);
    if (!ast_reduced)
    {
        LOG(ERROR) << "failed to reduce AST" << std::endl;
        return false;
    }

    if (job->m_debug)
        LOG(DEBUG) << "Dumping AST 3 (JSON): " << base64_encode(ast_reduced->to_json()) << std::endl;

    reduce_named_mappings(*job);

    ///=========================================
    /// END: AST REDUCTION
    ///=========================================

    ///=========================================
    /// BEGIN: GENERATOR
    ///=========================================
    LOG(DEBUG) << "Generating output" << std::endl;
    if (!generate(*job, std::move(ast_reduced)))
    {
        LOG(ERROR) << "failed to generate output" << std::endl;
        return false;
    }

    ///=========================================
    /// END: GENERATOR
    ///=========================================

    fflush(job->m_out);
    return true;
}

static bool verify_build_option(const std::string &option, const std::string &value)
{
    const static std::set<std::string> static_options = {
        "-S",            // assembly output
        "-PREP",         // preprocessor/Lexer output
        "-emit-tokens",  // lexer output (no preprocessing)
        "-emit-ir",      // IR output
        "-emit-c11",     // C11 output
        "-emit-bc",      // bitcode output
        "-c",            // compile only
        "-O0",           // optimization levels
        "-O1",           // optimization levels
        "-O2",           // optimization levels
        "-O3",           // optimization levels
        "-Os",           // optimization levels
        "-g",            // debug information
        "-flto",         // link time optimization
        "-fPIC",         // position independent code
        "-fPIE",         // position independent executable
        "-v",            // verbose
        "-s",            // strip
        "-nostdlib",     // no standard library
        "-nostartfiles", // no standard startup files
    };
    const static std::vector<std::pair<std::regex, std::regex>> static_regexes = {
        // -D<name>[=<value>]
        {std::regex("-D[a-zA-Z_][a-zA-Z0-9_]*"), std::regex("[a-zA-Z0-9_ ]*")},
        {std::regex("-l[a-zA-Z0-9_]*"), std::regex("")},
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
    // -<p><key>[=<value>]
    const static std::set<char> okay_prefixes = {'f', 'O', 'l', 'P', 'n', 'L', 'I', 'e', 'D', 'W', 'm', 'c', 'S', 'g', 's', 'v'};

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

LIB_EXPORT void quixcc_fault_handler(int sig)
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
    /*
        Catch crashes and pretty print them
    */

    sighandler_t old_handlers[6];

    old_handlers[0] = signal(SIGINT, quixcc_fault_handler);
    old_handlers[1] = signal(SIGILL, quixcc_fault_handler);
    old_handlers[2] = signal(SIGFPE, quixcc_fault_handler);
    old_handlers[3] = signal(SIGSEGV, quixcc_fault_handler);
    old_handlers[4] = signal(SIGTERM, quixcc_fault_handler);
    old_handlers[5] = signal(SIGABRT, quixcc_fault_handler);

    /*
        This guarantees that every job will run in a seperate thread:
            1. Every compilation unit has its own thread local storage
            1. Every compilation unit has its own cache
    */

    bool success = false;
    std::thread t([&]
                  { success = execute_job(job); });

    t.join();

    signal(SIGINT, old_handlers[0]);
    signal(SIGILL, old_handlers[1]);
    signal(SIGFPE, old_handlers[2]);
    signal(SIGSEGV, old_handlers[3]);
    signal(SIGTERM, old_handlers[4]);
    signal(SIGABRT, old_handlers[5]);

    return success;
}

LIB_EXPORT const quixcc_result_t *quixcc_result(quixcc_job_t *job)
{
    quixcc_result_t *result = &job->m_result;

    if (job->m_debug)
        return result;

    // remove debug messages
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

    if (result->m_count == 0)
    {
        free(result->m_messages);
        result->m_messages = nullptr;
    }
    else
    {
        result->m_messages = (quixcc_msg_t **)safe_realloc(result->m_messages, result->m_count * sizeof(quixcc_msg_t *));
    }

    return result;
}

LIB_EXPORT char *quixcc_compile(FILE *in, FILE *out, const char *options[])
{
    quixcc_job_t *job = quixcc_new();
    quixcc_set_input(job, in, "stdin");
    quixcc_set_output(job, out);

    if (options)
    {
        for (uint32_t i = 0; options[i]; i++)
            quixcc_add_option(job, options[i], true);
    }
    if (quixcc_run(job))
    {
        quixcc_dispose(job);
        return nullptr;
    }

    std::string str;
    for (uint32_t i = 0; i < job->m_result.m_count; i++)
    {
        quixcc_msg_t *msg = job->m_result.m_messages[i];
        if (!msg)
            continue;

        str += msg->message;
        str += '\n';
    }

    return safe_strdup(str.c_str());
}