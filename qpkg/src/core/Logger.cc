/*
    ░▒▓███████▓▒░   ░▒▓███████▓▒░ ░▒▓████████▓▒░
    ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░        ░▒▓█▓▒░
    ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░        ░▒▓█▓▒░
    ░▒▓█▓▒░░▒▓█▓▒░  ░▒▓██████▓▒░  ░▒▓██████▓▒░
    ░▒▓█▓▒░░▒▓█▓▒░        ░▒▓█▓▒░ ░▒▓█▓▒░
    ░▒▓█▓▒░░▒▓█▓▒░        ░▒▓█▓▒░ ░▒▓█▓▒░
    ░▒▓███████▓▒░  ░▒▓███████▓▒░  ░▒▓█▓▒░

    ===      DELTASPLOIT SOURCE CODE       ===

    Copyright (C) 2024 DeltaSploit Development
    team. All rights reserved.

    No part of this software may be modified,
    distributed, transmitted, reproduced or
    in any other way used without prior written
    permission of the copyright holders.
*/

#include <core/Logger.hh>
#include <thread>
#include <functional>
#include <fstream>
#include <filesystem>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <execinfo.h>
#include <map>

#define LOGGER_MASK_SRC 1
#define LOGGER_ECHO 1
#define DEBUG_MODE 0

thread_local std::ostringstream qpkg::core::Logger::m_buffer;
thread_local const char *qpkg::core::Logger::m_func = nullptr;
thread_local const char *qpkg::core::Logger::m_file = nullptr;
thread_local int qpkg::core::Logger::m_line = 0;
static bool _G_is_color_enabled = (getenv("QUIXCC_COLOR") == nullptr) || (std::string(getenv("QUIXCC_COLOR")) == "1");

static std::map<std::pair<const char *, int>, std::string> file_to_str_mapping;
static std::unordered_map<const char *, std::string> func_to_str_mapping;

qpkg::core::Logger::Logger(qpkg::core::E level)
{
    m_level = level;
    m_verbose = false;
}

static std::string escape_log_message(const std::string &str)
{
    /*
     * Escape special characters
     * Log messages must not contain newlines escape characters
     */
    std::string escaped;
    for (auto c : str)
    {
        if (c == '\n')
            escaped += "\\n";
        else if (c == '\r')
            escaped += "\\r";
        else if (c == '\t')
            escaped += "\\t";
        else
            escaped += c;
    }

    return escaped;
}

void qpkg::core::Logger::push()
{
    if (DEBUG_MODE == 0 && m_level == DEBUG && !m_verbose)
        return;

    std::stringstream ss;

    if (_G_is_color_enabled)
    {
        switch (m_level)
        {
        case DEBUG:
            ss << "\x1b[49;1mdebug:\x1b[0m " << escape_log_message(m_buffer.str());
            break;
        case INFO:
            ss << "\x1b[37;49;1minfo:\x1b[0m \x1b[37;49;1m" << escape_log_message(m_buffer.str()) << "\x1b[0m";
            break;
        case WARN:
            ss << "\x1b[35;49;1mwarning:\x1b[0m \x1b[37;49;1m" << escape_log_message(m_buffer.str()) << "\x1b[0m";
            break;
        case ERROR:
            ss << "\x1b[31;49;1merror:\x1b[0m \x1b[37;49;1m" << escape_log_message(m_buffer.str()) << "\x1b[0m";
            break;
        }
    }
    else
    {
        switch (m_level)
        {
        case DEBUG:
            ss << "(debug): " << escape_log_message(m_buffer.str());
            break;
        case INFO:
            ss << "(info): " << escape_log_message(m_buffer.str());
            break;
        case WARN:
            ss << "(WARNING): " << escape_log_message(m_buffer.str());
            break;
        case ERROR:
            ss << "(ERROR): " << escape_log_message(m_buffer.str());
            break;
        }
    }

    /* Push log entry to queue */
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(ss.str());
}

qpkg::core::Logger &qpkg::core::Logger::operator()(const char *func, const char *file, int line)
{
    /* Save in thread local storage */
    m_func = func;
    m_file = file;
    m_line = line;
    m_buffer.str("");

    return *this;
}

qpkg::core::Logger &qpkg::core::operator<<(qpkg::core::Logger &log, std::ostream &(*var)(std::ostream &))
{
    /*
     * This function is called when std::endl is used
     * It will push the log entry to the queue
     */

    (void)var; // suppress unused warning
    log.push();
    return log;
}

void qpkg::core::Logger::flush(std::ofstream &file)
{
    /* Flush all log entries to file */
    std::lock_guard<std::mutex> lock(m_mutex);

    while (!m_queue.empty())
    {
/* Write log entry to stderr and log file */
#if LOGGER_ECHO == 1
        std::cerr << m_queue.front() << std::endl;
#endif
        file << m_queue.front() << "\n";

        /* Pop log entry */
        m_queue.pop();
    }
}

static void flush_atexit()
{
    qpkg::core::LoggerSpool::getInst().flush(qpkg::core::LoggerSpool::LOG_FILE);
}

qpkg::core::LoggerSpool::LoggerSpool(std::chrono::duration<int> flushInterval)
{
    m_okay = false;

    /* Initialize loggers */
    m_loggers[DEBUG] = std::make_unique<Logger>(DEBUG);
    m_loggers[INFO] = std::make_unique<Logger>(INFO);
    m_loggers[WARN] = std::make_unique<Logger>(WARN);
    m_loggers[ERROR] = std::make_unique<Logger>(ERROR);

    /*
     * Not sure how many of these handlers are necessary
     * But I want this logger class to LOG no matter what
     */

    /* Flush logs at exit(x) */
    std::atexit(flush_atexit);

    try
    {
        /*
            If this code fails, the process must exit to avoid a recursive call to terminate
        */

        // check if folders exist
        std::filesystem::path path(LOG_FILE);
        if (!std::filesystem::exists(path.parent_path()))
            std::filesystem::create_directories(path.parent_path());

        if (std::filesystem::exists(path.parent_path()) && !std::filesystem::is_directory(path.parent_path()))
        {
            std::cerr << "logger: unable to create log file directories: " << path.parent_path() << " is not a directory" << std::endl;
            _exit(1); // exit to avoid recursive call to terminate
        }

        if (!std::filesystem::exists(LOG_FILE))
        {
            std::ofstream file(LOG_FILE);
            if (!file.is_open())
            {
                std::cerr << "logger: unable to create log file: " << LOG_FILE << std::endl;
                _exit(1); // exit to avoid recursive call to terminate
            }
            file.close();

            // set permissions
            std::filesystem::permissions(path, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::group_read | std::filesystem::perms::group_write | std::filesystem::perms::others_read | std::filesystem::perms::others_write);
        }

        // check write access
        std::ofstream file(LOG_FILE, std::ios::app);
        if (!file.is_open())
        {
            std::cerr << "logger: unable to open log file \"" << LOG_FILE << "\": " << strerror(errno) << std::endl;
            _exit(1); // exit to avoid recursive call to terminate
        }

        file.close();
    }
    catch (std::exception &e)
    {
    }

    /* Create a worker thread to save logs */
    std::thread(
        [this, flushInterval]()
        {
    while (true)
    { 
      auto x = std::chrono::steady_clock::now() + flushInterval;
      flush(LOG_FILE);
      std::this_thread::sleep_until(x);
    } })
        .detach();

    m_okay = true;
}

qpkg::core::LoggerSpool &qpkg::core::LoggerSpool::getInst()
{
    /* This memory MUST never be freed */
    static qpkg::core::LoggerSpool *instance = new qpkg::core::LoggerSpool();
    return *instance;
}

qpkg::core::Logger &qpkg::core::LoggerSpool::operator[](qpkg::core::E level)
{
    return *m_loggers[level];
}

void qpkg::core::LoggerSpool::flush(const char *filepath)
{
    /* If this fails, we have serious issues */
    if (!m_okay)
        return;

    try
    {
        /* Flush all loggers to file */
        std::ofstream file(filepath, std::ios::app);
        if (!file.is_open())
            return;

        /* Flush all loggers */
        for (auto &logger : m_loggers)
            logger.second->flush(file);

        /* Close file, implicit flush */
        file.close();
    }
    catch (std::exception &e)
    {
    }
}
