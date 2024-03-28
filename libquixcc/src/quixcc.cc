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

#include <quixcc.h>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <regex>
#include <filesystem>
#include <random>

#include <LibMacro.h>
#include <llvm/LLVMWrapper.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <generate/Generate.h>
#include <lexer/Lex.h>
#include <prep/Preprocesser.h>
#include <error/Message.h>
#include <error/Exception.h>
#include <parse/Parser.h>
#include <libquixcc.h>
#include <mutate/Routine.h>

static void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        std::cerr << "error: out of memory" << std::endl;
        exit(1);
    }
    return ptr;
}

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
    quixcc_job_t *job = (quixcc_job_t *)safe_malloc(sizeof(quixcc_job_t));
    memset((void *)job, 0, sizeof(quixcc_job_t));

    job->m_id = quixcc_uuid();

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

    if (job->m_result)
    {
        for (uint32_t i = 0; i < job->m_result->m_feedback.m_count; i++)
        {
            quixcc_msg_t *msg = job->m_result->m_feedback.m_messages[i];
            if (!msg)
                continue;

            if (msg->message)
                free((void *)msg->message);

            msg->message = nullptr;
            free(msg);

            job->m_result->m_feedback.m_messages[i] = nullptr;
        }

        if (job->m_result->m_feedback.m_messages)
            free(job->m_result->m_feedback.m_messages);
        job->m_result->m_feedback.m_count = 0;
        job->m_result->m_feedback.m_messages = nullptr;

        free(job->m_result);
    }

    if (job->m_argset)
        delete job->m_argset;

    if (job->m_filename)
    {
        free((void *)job->m_filename);
        job->m_filename = nullptr;
    }

    memset((void *)job, 0, sizeof(quixcc_job_t));

    free(job);

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

static bool quixcc_mutate_ast(quixcc_job_t *job, std::shared_ptr<libquixcc::AST> ast)
{
    libquixcc::Mutation mutator;
    mutator.add_routine(libquixcc::mutate::DiscoverNamedConstructs);
    mutator.add_routine(libquixcc::mutate::ExtrapolateEnumFields);
    mutator.add_routine(libquixcc::mutate::ResolveNamedConstructs);
    mutator.add_routine(libquixcc::mutate::FoldConstExpr);
    mutator.add_routine(libquixcc::mutate::ConvertTypes);
    mutator.add_routine(libquixcc::mutate::InferTypes);
    mutator.add_routine(libquixcc::mutate::FilterNonGeneratable);
    mutator.run(job, ast);

    return true;
}

static bool quixcc_verify_semantics(quixcc_job_t *job, std::shared_ptr<libquixcc::AST> ast)
{
    (void)job;
    (void)ast;
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

    return true;
}

static bool quixcc_optimize_ast(quixcc_job_t *job, std::shared_ptr<libquixcc::AST> ast)
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

    for (auto &arg : *argmap)
    {
        if (arg.first.starts_with("-D"))
        {
            std::string key = arg.first.substr(2);
            std::string value = arg.second;

            if (!verify_user_constant(key, value))
            {
                libquixcc::message(*job, libquixcc::Err::ERROR, "invalid user constant: " + key);
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
                    libquixcc::message(*job, libquixcc::Err::ERROR, "invalid user constant: " + key);
                    return false;
                }
                constants[key] = value;
            }
        }
    }

    return true;
}

static bool preprocess_phase(quixcc_job_t *job, std::shared_ptr<libquixcc::PrepEngine> prep)
{
    std::set<std::string> dirs;
    std::map<std::string, std::string> constants;

    if (!get_include_directories(job, dirs))
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "failed to get include directories");
        return false;
    }
    for (auto &dir : dirs)
        prep->addpath(dir);
    if (!get_compile_time_user_constants(job, constants))
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "failed to get compile time user constants");
        return false;
    }
    for (auto &constant : constants)
        prep->set_static(constant.first, constant.second);

    if (!get_env_constants(job, constants))
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "failed to get environment constants");
        return false;
    }
    for (auto &constant : constants)
        prep->set_static(constant.first, constant.second);

    if (!prep->set_source(job->m_in, job->m_filename))
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "failed to set source");
        return false;
    }

    return true;
}

static bool compile(quixcc_job_t *job)
{
    // Create an AST before goto statements
    // libquixcc::AST ast;
    std::shared_ptr<libquixcc::AST> ast = std::make_shared<libquixcc::AST>();

    if (job->m_argset->contains("-PREP"))
    {
        libquixcc::message(*job, libquixcc::Err::DEBUG, "Preprocessing only");
        std::shared_ptr<libquixcc::PrepEngine> prep = std::make_shared<libquixcc::PrepEngine>(*job);
        prep->setup();
        if (!preprocess_phase(job, prep))
            return false;

        // Generate output
        std::string tmp;
        std::optional<libquixcc::Token> tok;
        while ((tok = prep->next())->type() != libquixcc::TokenType::Eof)
        {
            tmp = tok->serialize();
            fwrite(tmp.c_str(), 1, tmp.size(), job->m_out);
            fputc('\n', job->m_out);
        }
        fflush(job->m_out);
        return true;
    }

    if (job->m_argset->contains("-LEX"))
    {
        libquixcc::message(*job, libquixcc::Err::DEBUG, "Lexing only");
        libquixcc::StreamLexer lexer;

        if (!lexer.set_source(job->m_in, job->m_filename))
        {
            libquixcc::message(*job, libquixcc::Err::ERROR, "failed to set source");
            return false;
        }

        // Generate output
        std::string tmp;
        libquixcc::Token tok;
        while ((tok = lexer.next()).type() != libquixcc::TokenType::Eof)
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
    std::shared_ptr<libquixcc::PrepEngine> prep = std::make_shared<libquixcc::PrepEngine>(*job);
    libquixcc::message(*job, libquixcc::Err::DEBUG, "Preprocessing source");
    prep->setup();
    if (!preprocess_phase(job, prep))
        return false;
    libquixcc::message(*job, libquixcc::Err::DEBUG, "Finished preprocessing source");
    ///=========================================
    /// END: PREPROCESSOR/LEXER
    ///=========================================

    ///=========================================
    /// BEGIN: PARSER
    ///=========================================
    libquixcc::message(*job, libquixcc::Err::DEBUG, "Building AST 1");
    if (!parse(*job, prep, ast, false))
        return false;
    libquixcc::message(*job, libquixcc::Err::DEBUG, "Finished building AST 1");
    if (job->m_debug)
        libquixcc::message(*job, libquixcc::Err::DEBUG, "Dumping AST 1 (JSON): " + base64_encode(ast->to_json(libquixcc::ParseNodeJsonSerializerVisitor())));
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
        libquixcc::message(*job, libquixcc::Err::DEBUG, "Dumping AST 2 (JSON): " + base64_encode(ast->to_json(libquixcc::ParseNodeJsonSerializerVisitor())));

    ///=========================================
    /// END: INTERMEDIATE PROCESSING
    ///=========================================

    ///=========================================
    /// BEGIN: GENERATOR
    ///=========================================
    libquixcc::message(*job, libquixcc::Err::DEBUG, "Generating output");
    if (!libquixcc::generate(*job, ast))
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "failed to generate output");
        return false;
    }

    ///=========================================
    /// END: GENERATOR
    ///=========================================

    return true;
}

static bool verify_build_option(const std::string &option, const std::string &value)
{
    const static std::set<std::string> static_options = {
        "-S",            // assembly output
        "-PREP",         // preprocessor/Lexer output
        "-LEX",          // lexer output (no preprocessing)
        "-emit-ir",      // IR output
        "-c",            // compile only
        "-O0",           // optimization levels
        "-O1",           // optimization levels
        "-O2",           // optimization levels
        "-O3",           // optimization levels
        "-Os",           // optimization levels
        "-g",            // debug information
        "-flto",         // link time optimization
        "-static",       // static linking
        "-shared",       // shared library
        "-fPIC",         // position independent code
        "-fPIE",         // position independent executable
        "-staticlib",    // build static library
        "-v",            // verbose
        "-s",            // strip
        "-nostdlib",     // no standard library
        "-nostartfiles", // no standard startup files
    };
    const static std::vector<std::pair<std::regex, std::regex>> static_regexes = {
        // -D<name>[=<value>]
        {std::regex("-D[a-zA-Z_][a-zA-Z0-9_]*"), std::regex("[a-zA-Z0-9_ ]*")},
        {std::regex("-l[a-zA-Z0-9_]*"), std::regex("")},
        {std::regex("-I[a-zA-Z0-9_]*"), std::regex("")},
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

    std::map<std::string, std::string> *argmap = job->m_argset;

    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        std::string option = job->m_options.m_options[i];

        if (option.size() == 2 && option[0] == '-' && !okay_prefixes.contains(option[1]))
        {
            libquixcc::message(*job, libquixcc::Err::ERROR, "invalid build option: " + option);
            return false;
        }

        size_t pos = option.find('=');
        std::string key = option.substr(0, pos);
        std::string value;

        if (pos != std::string::npos)
            value = option.substr(pos + 1);

        if (!verify_build_option(key, value))
        {
            libquixcc::message(*job, libquixcc::Err::ERROR, "invalid build option: " + key);
            return false;
        }

        if (key == "-v" && value.empty())
            job->m_debug = true;

        argmap->insert({key, value});

        if (!value.empty())
            libquixcc::message(*job, libquixcc::Err::DEBUG, "Added switch entry: " + key + "=" + value);
        else
            libquixcc::message(*job, libquixcc::Err::DEBUG, "Added switch entry: " + key);
    }

    return verify_build_option_conflicts(job);
}

LIB_EXPORT bool quixcc_run(quixcc_job_t *job)
{
    if (!job->m_in || !job->m_out || !job->m_filename || job->m_inner != nullptr || job->m_argset != nullptr)
        return false;

    job->m_inner = std::make_shared<libquixcc::LLVMContext>(job->m_filename);
    job->m_argset = new std::map<std::string, std::string>();
    job->m_result = (quixcc_result_t *)safe_malloc(sizeof(quixcc_result_t));
    memset(job->m_result, 0, sizeof(quixcc_result_t));

    libquixcc::message(*job, libquixcc::Err::DEBUG, "Starting quixcc run @ %s", get_datetime().c_str());

    if (!build_argmap(job))
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "failed to build argmap");
        return false;
    }

    bool success = false;
    try
    {
        if (compile(job))
        {
            libquixcc::message(*job, libquixcc::Err::DEBUG, "Compilation successful");
            success = true;
        }
        else
        {
            libquixcc::message(*job, libquixcc::Err::ERROR, "Compilation failed");
        }
    }
    catch (libquixcc::PreprocessorException &)
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "Compilation was aborted while preprocessing source");
    }
    catch (libquixcc::ParseException &)
    {
        libquixcc::message(*job, libquixcc::Err::ERROR, "Compilation was aborted while parsing source");
    }
    catch (std::exception &e)
    {
        libquixcc::message(*job, libquixcc::Err::FATAL, "Compilation failed: %s", e.what());
    }

    libquixcc::message(*job, libquixcc::Err::DEBUG, "Finished quixcc run @ %s", get_datetime().c_str());

    return job->m_result->m_success = success;
}

LIB_EXPORT const quixcc_result_t *quixcc_result(quixcc_job_t *job)
{
    quixcc_result_t *result = job->m_result;

    if (job->m_debug)
        return result;

    // remove debug messages
    uint32_t i = 0;

    while (i < result->m_feedback.m_count)
    {
        quixcc_msg_t *msg = result->m_feedback.m_messages[i];
        if (msg->m_level != QUIXCC_DEBUG)
        {
            i++;
            continue;
        }

        if (msg->message)
            free((void *)msg->message);
        msg->message = nullptr;
        free(msg);

        memmove(&result->m_feedback.m_messages[i], &result->m_feedback.m_messages[i + 1], (result->m_feedback.m_count - i - 1) * sizeof(quixcc_msg_t *));
        result->m_feedback.m_count--;
    }

    if (result->m_feedback.m_count == 0)
    {
        free(result->m_feedback.m_messages);
        result->m_feedback.m_messages = nullptr;
    }
    else
    {
        result->m_feedback.m_messages = (quixcc_msg_t **)safe_realloc(result->m_feedback.m_messages, result->m_feedback.m_count * sizeof(quixcc_msg_t *));
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
    for (uint32_t i = 0; i < job->m_result->m_feedback.m_count; i++)
    {
        quixcc_msg_t *msg = job->m_result->m_feedback.m_messages[i];
        if (!msg)
            continue;

        str += msg->message;
        str += '\n';
    }

    return safe_strdup(str.c_str());
}