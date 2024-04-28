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

#include <core/Logger.h>
#include <core/Exception.h>

LOGGER_SETUP(); // Define the logger

void libquixcc::LoggerGroup::push_message_to_job(quixcc_job_t &job, libquixcc::E type, const std::string &message)
{
    // Create a new message
    quixcc_msg_t *msg = (quixcc_msg_t *)malloc(sizeof(quixcc_msg_t));
    msg->line = 0;
    msg->column = 0;
    msg->message = strdup(message.c_str());
    msg->m_level = (quixcc_msg_level_t)type;

    // Add the message to the job
    job.m_result.m_messages = (quixcc_msg_t **)realloc(job.m_result.m_messages, (job.m_result.m_count + 1) * sizeof(quixcc_msg_t *));
    job.m_result.m_messages[job.m_result.m_count] = msg;
    job.m_result.m_count++;

    // Flush debug messages to stderr
    if (type == E::DEBUG && job.m_debug)
        std::cerr << message << std::endl;

    // Throw an exception if the message is an error or fatal
    if (type == E::ERROR || type == E::FATAL)
    {
        job.m_tainted = true;

        // At worst, the compiler will catch this upstream
        throw Exception();
    }
    else if (type == E::FAILED)
    {
        // This means we are in an invalid state, behavior is probably undefined.
        // If/When we segfault, the compiler will catch this upstream
        job.m_tainted = true;
    }
}

libquixcc::Logger &libquixcc::LoggerGroup::operator[](libquixcc::E level)
{
    if (!m_job->m_debug && level == E::DEBUG)
        return *m_hole; // Null-object pattern

    return *m_loggers[level];
}

bool libquixcc::LoggerGroup::is_color_enabled()
{
#if defined(_WIN32) || defined(_WIN64)
    // I don't know windows color codes
    return false;
#else
    static bool __G_is_color_enabled = (getenv("QUIXCC_COLOR") == nullptr) || (std::string(getenv("QUIXCC_COLOR")) == "1");
    return __G_is_color_enabled;
#endif
}

std::string libquixcc::LoggerGroup::format_message_ansi(const std::string &message, libquixcc::E type, const libquixcc::Token &tok)
{
    std::string msg;

    msg.reserve(message.size() + 20);

    if (!tok.nil())
    {
        /* Add the file, line, and column information */
        msg += "\x1b[49;1m" + tok.loc().file + ":";
        msg += std::to_string(tok.loc().line) + ":" + std::to_string(tok.loc().col) + ":\x1b[0m ";
    }

    /* ANSI color codes */
    switch (type)
    {
    case E::DEBUG:
        return msg + "\x1b[49;1mdebug:\x1b[0m " + message + "\x1b[0m";
    case E::SUCCESS:
        return msg + "\x1b[32;49;1msuccess:\x1b[0m " + message + "\x1b[0m";
    case E::INFO:
        return msg + "\x1b[37;49;1minfo:\x1b[0m \x1b[37;49m" + message + "\x1b[0m";
    case E::WARN:
        return msg + "\x1b[35;49;1mwarn:\x1b[0m \x1b[37;49;1m" + message + "\x1b[0m";
    case E::ERROR:
    case E::FAILED:
        return msg + "\x1b[31;49;1merror:\x1b[0m \x1b[37;49;1m" + message + "\x1b[0m";
    case E::FATAL:
        return msg + "\x1b[31;49;1mINTERNAL COMPILER ERROR:\x1b[0m \x1b[37;49;1m" + message + "\x1b[0m";
    default:
        return message + "\x1b[0m";
    }
}

std::string libquixcc::LoggerGroup::format_message_nocolor(const std::string &message, libquixcc::E type, const libquixcc::Token &tok)
{
    std::string msg;

    msg.reserve(message.size() + 20);

    if (!tok.nil())
    {
        /* Add the file, line, and column information */
        msg += tok.loc().file + ":";
        msg += std::to_string(tok.loc().line) + ":" + std::to_string(tok.loc().col) + ": ";
    }

    switch (type)
    {
    case E::DEBUG:
        return msg + "(debug): " + message;
    case E::SUCCESS:
        return msg + "(success): " + message;
    case E::INFO:
        return msg + "(info): " + message;
    case E::WARN:
        return msg + "(warn): " + message;
    case E::ERROR:
    case E::FAILED:
        return msg + "(ERROR): " + message;
    case E::FATAL:
        return msg + "(FATAL): " + message;
    default:
        return message;
    }
}
