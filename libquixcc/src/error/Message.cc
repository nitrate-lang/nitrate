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

#include <error/Message.h>
#include <error/Exception.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

using namespace libquixcc;

#define STRING_BUFFER_SIZE 4096

static bool __G_is_color_enabled = getenv("QUIXCC_COLOR") == nullptr;

static void push_message_to_job(quixcc_job_t &job, Err type, const std::string &message)
{
    job.m_result->m_feedback.m_messages = (quixcc_msg_t **)realloc(job.m_result->m_feedback.m_messages, (job.m_result->m_feedback.m_count + 1) * sizeof(quixcc_msg_t *));
    quixcc_msg_t *msg = (quixcc_msg_t *)malloc(sizeof(quixcc_msg_t));
    msg->line = 0;
    msg->column = 0;
    msg->message = strdup(message.c_str());
    msg->m_level = (quixcc_msg_level_t)type;
    msg->m_allocated = true;
    job.m_result->m_feedback.m_messages[job.m_result->m_feedback.m_count] = msg;
    job.m_result->m_feedback.m_count++;
}

static bool is_color_enabled()
{
#if defined(_WIN32) || defined(_WIN64)
    return false;
#else
    return __G_is_color_enabled;
#endif
}

static std::string make_message_colored(Err type, const std::string &format, va_list args)
{
    std::string msg;

    switch (type)
    {
    case Err::DEBUG:
        msg = "\x1b[49;1mdebug:\x1b[0m " + format;
        break;
    case Err::SUCCESS:
        msg = "\x1b[32;49;1msuccess:\x1b[0m " + format;
        break;
    case Err::INFO:
        msg = "\x1b[37;49;1minfo:\x1b[0m \x1b[37;49m" + format;
        break;
    case Err::WARN:
        msg = "\x1b[35;49;1mwarn:\x1b[0m \x1b[37;49;1m" + format;
        break;
    case Err::ERROR:
        msg = "\x1b[31;49;1merror:\x1b[0m \x1b[37;49;1m" + format;
        break;
    case Err::FATAL:
        msg = "\x1b[31;49;1mINTERNAL COMPILER ERROR:\x1b[0m \x1b[37;49;1m" + format;
        break;
    default:
        msg = format;
        break;
    }

    size_t len = msg.size();

    std::string tmp;
    tmp.resize(STRING_BUFFER_SIZE + len);
    len += vsnprintf(&tmp[0], tmp.size() - 1, msg.c_str(), args);
    msg = tmp.substr(0, len);

    return msg + "\x1b[0m";
}

static std::string make_message_nocolor(Err type, const std::string &format, va_list args)
{
    std::string msg;

    switch (type)
    {
    case Err::DEBUG:
        msg = "(debug): " + format;
        break;
    case Err::SUCCESS:
        msg = "(success): " + format;
        break;
    case Err::INFO:
        msg = "(info): " + format;
        break;
    case Err::WARN:
        msg = "(warn): " + format;
        break;
    case Err::ERROR:
        msg = "(ERROR): " + format;
        break;
    case Err::FATAL:
        msg = "(FATAL): " + format;
        break;
    default:
        msg = format;
        break;
    }

    size_t len = msg.size();

    std::string tmp;
    tmp.resize(STRING_BUFFER_SIZE + len);
    len += vsnprintf(&tmp[0], tmp.size() - 1, msg.c_str(), args);
    msg = tmp.substr(0, len);

    return msg;
}

static std::string make_parser_message_colored(const std::string &file, const libquixcc::Token &tok, Err type, const std::string &format, va_list args)
{
    std::string msg = "\x1b[49;1m" + file + ":";

    msg += std::to_string(tok.loc().line) + ":" + std::to_string(tok.loc().col) + ":\x1b[0m ";

    switch (type)
    {
    case Err::DEBUG:
        msg += "\x1b[49;1mdebug:\x1b[0m " + format;
        break;
    case Err::SUCCESS:
        msg += "\x1b[32;49;1msuccess:\x1b[0m " + format;
        break;
    case Err::INFO:
        msg += "\x1b[37;49;1minfo:\x1b[0m \x1b[37;49m" + format;
        break;
    case Err::WARN:
        msg += "\x1b[35;49;1mwarn:\x1b[0m \x1b[37;49;1m" + format;
        break;
    case Err::ERROR:
        msg += "\x1b[31;49;1merror:\x1b[0m \x1b[37;49;1m" + format;
        break;
    case Err::FATAL:
        msg += "\x1b[31;49;1mINTERNAL COMPILER ERROR:\x1b[0m \x1b[37;49;1m" + format;
        break;
    default:
        msg += format;
        break;
    }

    size_t len = msg.size();

    std::string tmp;
    tmp.resize(STRING_BUFFER_SIZE + len);
    len += vsnprintf(&tmp[0], tmp.size() - 1, msg.c_str(), args);
    msg = tmp.substr(0, len);

    return msg + "\x1b[0m";
}

static std::string make_parser_message_nocolor(const std::string &file, const libquixcc::Token &tok, Err type, const std::string &format, va_list args)
{
    std::string msg = file + ":";

    msg += std::to_string(tok.loc().line) + ":" + std::to_string(tok.loc().col) + ": ";

    switch (type)
    {
    case Err::DEBUG:
        msg += "(debug): " + format;
        break;
    case Err::SUCCESS:
        msg += "(success): " + format;
        break;
    case Err::INFO:
        msg += "(info): " + format;
        break;
    case Err::WARN:
        msg += "(warn): " + format;
        break;
    case Err::ERROR:
        msg += "(ERROR): " + format;
        break;
    case Err::FATAL:
        msg += "(FATAL): " + format;
        break;
    default:
        msg += format;
        break;
    }

    size_t len = msg.size();

    std::string tmp;
    tmp.resize(STRING_BUFFER_SIZE + len);
    len += vsnprintf(&tmp[0], tmp.size() - 1, msg.c_str(), args);
    msg = tmp.substr(0, len);

    return msg;
}

void libquixcc::message(quixcc_job_t &job, Err type, const std::string &format, ...)
{
    if (!job.m_debug && type == Err::DEBUG)
        return;

    va_list args;
    va_start(args, format);
    std::string msg;

    if (is_color_enabled())
        msg = make_message_colored(type, format, args);
    else
        msg = make_message_nocolor(type, format, args);

    va_end(args);

    push_message_to_job(job, type, msg);
}

void libquixcc::parmsg(quixcc_job_t &job, const libquixcc::Token &tok, libquixcc::Err type, const std::string &format, ...)
{
    if (!job.m_debug && type == Err::DEBUG)
        return;

    va_list args;
    va_start(args, format);
    std::string msg;
    std::string filename = tok.loc().file;
    if (filename.empty())
        filename = job.m_filename;

    if (is_color_enabled())
        msg = make_parser_message_colored(filename, tok, type, format, args);
    else
        msg = make_parser_message_nocolor(filename, tok, type, format, args);

    va_end(args);

    push_message_to_job(job, type, msg);

    if (type == Err::FATAL || type == Err::ERROR)
        throw libquixcc::ParseException();
}

void libquixcc::prepmsg(quixcc_job_t &job, const libquixcc::Token &tok, libquixcc::Err type, const std::string &format, ...)
{
    if (!job.m_debug && type == Err::DEBUG)
        return;

    va_list args;
    va_start(args, format);
    std::string msg;
    std::string filename = tok.loc().file;
    if (filename.empty())
        filename = job.m_filename;

    if (is_color_enabled())
        msg = make_parser_message_colored(filename, tok, type, format, args);
    else
        msg = make_parser_message_nocolor(filename, tok, type, format, args);

    va_end(args);

    push_message_to_job(job, type, msg);

    if (type == Err::FATAL || type == Err::ERROR)
        throw libquixcc::PreprocessorException();
}

void libquixcc::semanticmsg(quixcc_job_t &job, libquixcc::Err type, bool fatal_now, const std::string &format, ...)
{
    if (!job.m_debug && type == Err::DEBUG)
        return;

    va_list args;
    va_start(args, format);
    std::string msg;

    if (is_color_enabled())
        msg = make_message_colored(type, format, args);
    else
        msg = make_message_nocolor(type, format, args);

    va_end(args);

    push_message_to_job(job, type, msg);

    if (fatal_now)
    {
        if (type == Err::FATAL || type == Err::ERROR)
            throw libquixcc::SemanticException();
    }
}