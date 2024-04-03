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

#include <error/Logger.h>
#include <error/Exception.h>

LOGGER_SETUP();

using namespace libquixcc;

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
