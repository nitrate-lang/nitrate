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

#ifndef __QUIXCC_LOGGER_H__
#define __QUIXCC_LOGGER_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <functional>
#include <queue>
#include <lexer/Token.h>
#include <core/Messages.h>
#include <libquixcc.h>

namespace libquixcc
{
    enum E
    {
        RAW,
        DEBUG,
        SUCCESS,
        INFO,
        WARN,
        ERROR,
        FAILED, // Just like error, but doesn't throw an exception
        FATAL
    };

    enum class log
    {
        format,
        raw
    };

    typedef std::function<void(const std::string &, E type)> LogDispatchFunc;
    typedef std::function<std::string(const std::string &, E type, const Token &)> LogFormatFunc;

    class Logger
    {
        std::stringstream m_buf;
        std::queue<size_t> m_replacement_idx;
        std::vector<std::string> m_parts;
        Token m_tok;
        LogDispatchFunc m_dispatch;
        LogFormatFunc m_fmt;
        E m_level;
        libquixcc::log m_mode;

    public:
        inline Logger(E level) : m_dispatch(nullptr), m_fmt(nullptr), m_level(level), m_mode(log::format) {}

        template <class T>
        Logger &operator+=(const T &msg)
        {
            if (m_mode == log::raw)
            {
                m_buf << msg;
                return *this;
            }

            if (!m_replacement_idx.empty())
            {
                m_parts[m_replacement_idx.front()] = msg;
                m_replacement_idx.pop();
                if (m_replacement_idx.empty())
                {
                    for (auto &part : m_parts)
                        m_buf << part;
                    m_parts = {};
                }
                return *this;
            }

            std::stringstream ss;
            ss << msg;
            auto s = ss.str();

            // Split the msg into parts by dilimiter '{}'
            size_t start = 0, end = 0, idx = 0;
            while ((end = s.find("{}", start)) != std::string::npos)
            {
                m_parts.push_back(s.substr(start, end - start));
                m_parts.push_back(""); // placeholder for replacement
                m_replacement_idx.push(idx + 1);
                idx += 2;
                start = end + 2;
            }

            if (start < s.length())
                m_parts.push_back(s.substr(start));

            if (!idx)
                m_buf << s;

            return *this;
        }

        inline Logger &log(const std::string &message, const Token &tok = Token())
        {
            m_buf << message;
            m_tok = tok;
            flush();
            return *this;
        }

        inline void mode(libquixcc::log m)
        {
            m_mode = m;
        }

        inline void flush()
        {
            if (!m_dispatch || !m_fmt)
                throw std::runtime_error("Logger dispatch function not set");

            m_dispatch(m_fmt(m_buf.str(), m_level, m_tok), m_level);
            m_buf.str("");
            m_parts = {};
            m_replacement_idx = {};
            m_tok = Token();

            mode(log::format);
        }

        inline void base(const Token &tok) { m_tok = tok; }

        inline E level() { return m_level; }

        inline void conf(LogDispatchFunc func, LogFormatFunc fmt)
        {
            m_dispatch = func;
            m_fmt = fmt;
        }
    };

    class LoggerGroup
    {
        std::unordered_map<E, Logger *> m_loggers;
        Logger *m_hole;
        quixcc_job_t *m_job;

        static void push_message_to_job(quixcc_job_t &job, E type, const std::string &message);

        static bool is_color_enabled();
        static std::string format_message_ansi(const std::string &message, E type, const Token &tok);
        static std::string format_message_nocolor(const std::string &message, E type, const Token &tok);

        static inline std::string format_message(const std::string &message, E type, const Token &tok)
        {
            return is_color_enabled() ? format_message_ansi(message, type, tok) : format_message_nocolor(message, type, tok);
        }

    public:
        inline LoggerGroup()
        {
            m_loggers[DEBUG] = new Logger(DEBUG);
            m_loggers[SUCCESS] = new Logger(SUCCESS);
            m_loggers[INFO] = new Logger(INFO);
            m_loggers[WARN] = new Logger(WARN);
            m_loggers[ERROR] = new Logger(ERROR);
            m_loggers[FAILED] = new Logger(FAILED);
            m_loggers[FATAL] = new Logger(FATAL);
            m_hole = new Logger(DEBUG);
        }

        ~LoggerGroup()
        {
            for (auto &logger : m_loggers)
                delete logger.second;
            delete m_hole;
        }

        inline void setup(quixcc_job_t &job)
        {
            m_job = &job;

            for (auto &logger : m_loggers)
            {
                logger.second->conf([&job](const std::string &message, E type)
                                    { push_message_to_job(job, type, message); },
                                    format_message);
            }

            m_hole->conf([](const std::string &message, E type)
                         { (void)message; (void)type; },
                         [](const std::string &message, E type, const Token &tok)
                         { (void)message; (void)type; (void)tok; return ""; });
        }

        Logger &operator[](E level);
    };

    template <class T>
    Logger &operator<<(Logger &logger, const T &msg)
    {
        return logger += msg;
    }

    static inline Logger &operator<<(Logger &l, libquixcc::log m)
    {
        l.mode(m);
        return l;
    }
    static inline void operator<<(Logger &log, std::ostream &(*var)(std::ostream &)) { log.flush(); }
    static inline Logger &operator<<(Logger &log, const Token &tok)
    {
        log.base(tok);
        return log;
    }

    extern thread_local LoggerGroup _G_loggerGrp;

#define LOG(lvl) libquixcc::_G_loggerGrp[lvl]
#define LOGGER_SETUP() thread_local libquixcc::LoggerGroup libquixcc::_G_loggerGrp;

    static inline void LoggerConfigure(quixcc_job_t &job)
    {
        _G_loggerGrp.setup(job);
    }

};

#endif // __QUIXCC_LOGGER_H__