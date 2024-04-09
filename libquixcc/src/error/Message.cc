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

#include <error/Logger.h>
#include <error/Exception.h>

LOGGER_SETUP();

using namespace libquixcc;

void libquixcc::LoggerGroup::push_message_to_job(quixcc_job_t &job, libquixcc::E type, const std::string &message)
{
    job.m_result.m_messages = (quixcc_msg_t **)realloc(job.m_result.m_messages, (job.m_result.m_count + 1) * sizeof(quixcc_msg_t *));
    quixcc_msg_t *msg = (quixcc_msg_t *)malloc(sizeof(quixcc_msg_t));
    msg->line = 0;
    msg->column = 0;
    msg->message = strdup(message.c_str());
    msg->m_level = (quixcc_msg_level_t)type;
    job.m_result.m_messages[job.m_result.m_count] = msg;
    job.m_result.m_count++;

    if (type == E::DEBUG && job.m_debug)
        std::cerr << message << std::endl;

    if (type == E::ERROR || type == E::FATAL)
    {
        job.m_tainted = true;
        throw Exception();
    }
    else if (type == E::FAILED)
    {
        job.m_tainted = true;
    }
}

libquixcc::Logger &libquixcc::LoggerGroup::operator[](libquixcc::E level)
{
    if (!m_job->m_debug && level == E::DEBUG)
        return *m_hole;
    return *m_loggers[level];
}

bool libquixcc::LoggerGroup::is_color_enabled()
{
#if defined(_WIN32) || defined(_WIN64)
    return false;
#else
    static bool __G_is_color_enabled = getenv("QUIXCC_COLOR") == nullptr;

    return __G_is_color_enabled;
#endif
}

std::string libquixcc::LoggerGroup::format_message_ansi(const std::string &message, libquixcc::E type, const libquixcc::Token &tok)
{
    std::string msg;

    if (!tok.nil())
    {
        msg += "\x1b[49;1m" + tok.loc().file + ":";
        msg += std::to_string(tok.loc().line) + ":" + std::to_string(tok.loc().col) + ":\x1b[0m ";
    }

    switch (type)
    {
    case E::DEBUG:
        msg += "\x1b[49;1mdebug:\x1b[0m " + message + "\x1b[0m";
        break;
    case E::SUCCESS:
        msg += "\x1b[32;49;1msuccess:\x1b[0m " + message + "\x1b[0m";
        break;
    case E::INFO:
        msg += "\x1b[37;49;1minfo:\x1b[0m \x1b[37;49m" + message + "\x1b[0m";
        break;
    case E::WARN:
        msg += "\x1b[35;49;1mwarn:\x1b[0m \x1b[37;49;1m" + message + "\x1b[0m";
        break;
    case E::ERROR:
    case E::FAILED:
        msg += "\x1b[31;49;1merror:\x1b[0m \x1b[37;49;1m" + message + "\x1b[0m";
        break;
    case E::FATAL:
        msg += "\x1b[31;49;1mINTERNAL COMPILER ERROR:\x1b[0m \x1b[37;49;1m" + message + "\x1b[0m";
        break;
    default:
        msg += message + "\x1b[0m";
        break;
    }

    return msg;
}

std::string libquixcc::LoggerGroup::format_message_nocolor(const std::string &message, libquixcc::E type, const libquixcc::Token &tok)
{
    std::string msg;

    if (!tok.nil())
    {
        msg += tok.loc().file + ":";
        msg += std::to_string(tok.loc().line) + ":" + std::to_string(tok.loc().col) + ": ";
    }

    switch (type)
    {
    case E::DEBUG:
        msg += "(debug): " + message;
        break;
    case E::SUCCESS:
        msg += "(success): " + message;
        break;
    case E::INFO:
        msg += "(info): " + message;
        break;
    case E::WARN:
        msg += "(warn): " + message;
        break;
    case E::ERROR:
    case E::FAILED:
        msg += "(ERROR): " + message;
        break;
    case E::FATAL:
        msg += "(FATAL): " + message;
        break;
    default:
        msg += message;
        break;
    }

    return msg;
}
