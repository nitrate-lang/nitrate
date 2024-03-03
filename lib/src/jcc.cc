#include <jcc.h>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <openssl/rand.h>

#include <llvm/llvm-ctx.h>
#include <generate/generate.h>
#include <lexer/lex.h>
#include <prep/preprocess.h>
#include <error/message.h>
#include <error/exceptions.h>
#include <parse/parser.h>

#define LIB_EXPORT extern "C" __attribute__((visibility("default")))

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

static jcc_uuid_t jcc_uuid()
{
    jcc_uuid_t uuid;
    // if (RAND_bytes((unsigned char *)&uuid, sizeof(jcc_uuid_t)) != 1)
    // {
    //     uuid.m_high = (uint64_t)rand() << 32 | rand();
    //     uuid.m_low = (uint64_t)rand() << 32 | rand();
    // }
    /// TODO: Fix this
    uuid.m_high = uuid.m_low = 0;
    return uuid;
}

LIB_EXPORT jcc_job_t *jcc_new()
{
    jcc_job_t *job = (jcc_job_t *)safe_malloc(sizeof(jcc_job_t));
    memset(job, 0, sizeof(jcc_job_t));

    job->m_id = jcc_uuid();

    return job;
}

LIB_EXPORT bool jcc_dispose(jcc_job_t *job)
{
    if (!job)
        return false;

    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        jcc_option_t *option = job->m_options.m_options[i];
        if (!option)
            continue;

        if (option->m_u)
            free((void *)option->m_u);
        if (option->m_v)
            free((void *)option->m_v);

        option->m_u = nullptr;
        option->m_v = nullptr;
        free(option);

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
            jcc_msg_t *msg = job->m_result->m_feedback.m_messages[i];
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

    if (job->m_inner)
        delete (libj::LLVMContext *)job->m_inner;

    if (job->m_filename)
    {
        free((void *)job->m_filename);
        job->m_filename = nullptr;
    }

    memset(job, 0, sizeof(jcc_job_t));

    free(job);

    return true;
}

LIB_EXPORT void jcc_add_option(jcc_job_t *job, const char *name, const char *value, bool enabled)
{
    jcc_option_t *option;

    if (!job || !name)
        return;

    option = (jcc_option_t *)safe_malloc(sizeof(jcc_option_t));
    option->m_enabled = enabled;

    option->m_u = safe_strdup(name);

    if (value)
        option->m_v = safe_strdup(value);
    else
        option->m_v = nullptr;

    job->m_options.m_options = (jcc_option_t **)safe_realloc(job->m_options.m_options, (job->m_options.m_count + 1) * sizeof(jcc_option_t *));
    job->m_options.m_options[job->m_options.m_count++] = option;
}

LIB_EXPORT void jcc_remove_option(jcc_job_t *job, const char *name)
{
    if (!job || !name)
        return;

    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        jcc_option_t *option = job->m_options.m_options[i];
        if (!option)
            continue;

        if (strcmp(option->m_u, name) == 0)
        {
            if (option->m_u)
                free((void *)option->m_u);
            if (option->m_v)
                free((void *)option->m_v);

            option->m_u = nullptr;
            option->m_v = nullptr;
            free(option);

            job->m_options.m_options[i] = job->m_options.m_options[job->m_options.m_count - 1];
            job->m_options.m_options[job->m_options.m_count - 1] = nullptr;
            job->m_options.m_count--;

            break;
        }
    }
}

LIB_EXPORT void jcc_set_input(jcc_job_t *job, FILE *in, const char *filename)
{
    if (!job || !in || !filename)
        return;

    job->m_in = in;
    job->m_filename = safe_strdup(filename);
}

LIB_EXPORT void jcc_set_output(jcc_job_t *job, FILE *out)
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

static bool jcc_mutate_ast(jcc_job_t *job, libj::AST &ast)
{
    (void)job;
    (void)ast;

    /// TODO: Name resolution. Update all identifiers to use the fully qualified name
    /// TODO: Replace UserTypeNode with the defined type
    /// TODO: Type Coercion/Conversion
    /// TODO: Type Inference
    /// TODO: Evalulate constant expressions
    /// TODO:

    return true;
}

static bool jcc_verify_semantics(jcc_job_t *job, libj::AST &ast)
{
    (void)job;
    (void)ast;
    return true;
}

static bool jcc_optimize_ast(jcc_job_t *job, libj::AST &ast)
{
    (void)job;
    (void)ast;
    return true;
}

static bool get_include_directories(jcc_job_t *job, std::set<std::string> &dirs)
{
    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        jcc_option_t *option = job->m_options.m_options[i];
        if (!option)
            continue;

        if (strcmp(option->m_u, "I") == 0)
        {
            dirs.insert(option->m_v);
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

static bool get_compile_time_user_constants(jcc_job_t *job, std::map<std::string, std::string> &constants)
{
    for (uint32_t i = 0; i < job->m_options.m_count; i++)
    {
        jcc_option_t *option = job->m_options.m_options[i];
        if (!option)
            continue;

        if (strcmp(option->m_u, "D") == 0)
        {
            std::string def = option->m_v;
            size_t pos = def.find('=');
            if (pos != std::string::npos)
            {
                std::string key = def.substr(0, pos);
                std::string value = def.substr(pos + 1);

                if (!verify_user_constant(key, value))
                {
                    libj::message(*job, libj::Err::ERROR, "invalid user constant: " + key);
                    return false;
                }

                constants[key] = value;
            }
            else
            {
                constants[def] = "true";
            }

            continue;
        }
    }

    return true;
}

static bool preprocess_phase(jcc_job_t *job, std::shared_ptr<libj::PrepEngine> prep)
{
    std::set<std::string> dirs;
    std::map<std::string, std::string> constants;

    if (!get_include_directories(job, dirs))
    {
        libj::message(*job, libj::Err::ERROR, "failed to get include directories");
        return false;
    }
    for (auto &dir : dirs)
        prep->addpath(dir);
    if (!get_compile_time_user_constants(job, constants))
    {
        libj::message(*job, libj::Err::ERROR, "failed to get compile time user constants");
        return false;
    }
    for (auto &constant : constants)
        prep->set_static(constant.first, constant.second);

    if (!prep->set_source(job->m_in, job->m_filename))
    {
        libj::message(*job, libj::Err::ERROR, "failed to set source");
        return false;
    }

    return true;
}

static bool compile(jcc_job_t *job)
{
    // Create an AST before goto statements
    libj::AST ast;

    ///=========================================
    /// BEGIN: PREPROCESSOR/LEXER
    ///=========================================
    std::shared_ptr<libj::PrepEngine> prep = std::make_shared<libj::PrepEngine>(*job);
    libj::message(*job, libj::Err::DEBUG, "Preprocessing source");
    prep->setup();
    if (!preprocess_phase(job, prep))
        return false;
    libj::message(*job, libj::Err::DEBUG, "Finished preprocessing source");
    ///=========================================
    /// END: PREPROCESSOR/LEXER
    ///=========================================

    ///=========================================
    /// BEGIN: PARSER
    ///=========================================
    libj::Parser parser;
    libj::message(*job, libj::Err::DEBUG, "Building AST 1");
    if (!parser.parse(*job, prep, ast))
        return false;
    libj::message(*job, libj::Err::DEBUG, "Finished building AST 1");
    if (job->m_debug)
        libj::message(*job, libj::Err::DEBUG, "Dumping AST 1 (JSON): " + base64_encode(ast.to_json()));
    ///=========================================
    /// END: PARSER
    ///=========================================

    ///=========================================
    /// BEGIN: INTERMEDIATE PROCESSING
    ///=========================================
    if (!jcc_mutate_ast(job, ast))
        return false;

    if (!jcc_verify_semantics(job, ast))
        return false;

    if (!jcc_optimize_ast(job, ast))
        return false;
    ///=========================================
    /// END: INTERMEDIATE PROCESSING
    ///=========================================

    ///=========================================
    /// BEGIN: GENERATOR
    ///=========================================
    libj::Generator gen;
    libj::message(*job, libj::Err::DEBUG, "Generating output");
    if (!gen.synthesize_LLVM_IR(*job, ast))
    {
        libj::message(*job, libj::Err::ERROR, "failed to generate output");
        return false;
    }
    ///=========================================
    /// END: GENERATOR
    ///=========================================

    return true;
}

LIB_EXPORT bool jcc_run(jcc_job_t *job)
{
    if (!job->m_in || !job->m_out || !job->m_filename || job->m_inner != nullptr)
        return false;

    job->m_inner = new libj::LLVMContext(job->m_filename);
    job->m_result = (jcc_result_t *)safe_malloc(sizeof(jcc_result_t));
    memset(job->m_result, 0, sizeof(jcc_result_t));

    libj::message(*job, libj::Err::DEBUG, "Starting jcc run @ %s", get_datetime().c_str());
    bool success = false;

    try
    {
        if (compile(job))
        {
            libj::message(*job, libj::Err::DEBUG, "Compilation successful");
            success = true;
        }
        else
        {
            libj::message(*job, libj::Err::ERROR, "Compilation failed");
        }
    }
    catch (libj::PreprocessorException &)
    {
        libj::message(*job, libj::Err::ERROR, "Compilation failed because of a preprocessor error");
    }
    catch (libj::ParseException &)
    {
        libj::message(*job, libj::Err::ERROR, "Compilation failed because of a parse error");
    }
    catch (std::exception &e)
    {
        libj::message(*job, libj::Err::FATAL, "Compilation failed: %s", e.what());
    }

    libj::message(*job, libj::Err::DEBUG, "Finished jcc run @ %s", get_datetime().c_str());

    return success;
}

LIB_EXPORT const jcc_result_t *jcc_result(jcc_job_t *job)
{
    jcc_result_t *result = job->m_result;

    if (job->m_debug)
        return result;

    // remove debug messages
    uint32_t i = 0;

    while (i < result->m_feedback.m_count)
    {
        jcc_msg_t *msg = result->m_feedback.m_messages[i];
        if (msg->m_level != JCC_DEBUG)
        {
            i++;
            continue;
        }

        if (msg->message)
            free((void *)msg->message);
        msg->message = nullptr;
        free(msg);

        memmove(&result->m_feedback.m_messages[i], &result->m_feedback.m_messages[i + 1], (result->m_feedback.m_count - i - 1) * sizeof(jcc_msg_t *));
        result->m_feedback.m_count--;
    }

    if (result->m_feedback.m_count == 0)
    {
        free(result->m_feedback.m_messages);
        result->m_feedback.m_messages = nullptr;
    }
    else
    {
        result->m_feedback.m_messages = (jcc_msg_t **)safe_realloc(result->m_feedback.m_messages, result->m_feedback.m_count * sizeof(jcc_msg_t *));
    }

    return result;
}
