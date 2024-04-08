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
#include <openssl/rand.h>
#include <map>

#define LOGGER_MASK_SRC 1
#define LOGGER_ECHO 0

thread_local std::ostringstream qpkg::core::Logger::m_buffer;
thread_local const char *qpkg::core::Logger::m_func = nullptr;
thread_local const char *qpkg::core::Logger::m_file = nullptr;
thread_local int qpkg::core::Logger::m_line = 0;

static std::string generate_logprefix()
{
    /*
     * Generate a random 6 byte tag to prepend in each log entry.
     * This will make it easier to group logs from the same process
     */

    uint8_t buf[6];
    if (RAND_bytes((uint8_t *)buf, sizeof(buf)) != 1)
        throw std::runtime_error("logger: unable to generate random log prefix");

    /* HEX ENCODE */
    std::stringstream ss;
    for (auto c : buf)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;

    return ss.str();
}

static std::string get_runtime_logprefix()
{
    /*
     * This function is called once per process
     * It will generate a random log prefix and save it
     */
    static std::string prefix = generate_logprefix();
    return prefix;
}

static std::string get_timestamp()
{
    /*
     * Get current time in UTC
     */

    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = std::gmtime(&now_time_t);
    std::array<char, 30> timestamp;

    std::strftime(timestamp.data(), timestamp.size(), "%Y-%m-%d %H:%M:%S", now_tm);

    return timestamp.data();
}

static std::unordered_map<qpkg::core::E, std::string> level_to_str = {
    {qpkg::core::DEBUG, "HELP"},
    {qpkg::core::INFO, "INFO"},
    {qpkg::core::WARN, "WARN"},
    {qpkg::core::ERROR, "FAIL"}};

static std::map<std::pair<const char *, int>, std::string> file_to_str_mapping;
static std::unordered_map<const char *, std::string> func_to_str_mapping;

static std::string mask_source(const std::string &str)
{
    /*
        Hex encoded CRC32 hash the source data
    */

#if LOGGER_MASK_SRC == 0
    return str;
#else

    uint32_t crc = 0xffffffff;
    for (auto c : str)
    {
        crc ^= c;
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
    }

    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << crc;
    return ss.str();

#endif
}

static std::string get_file_str(const char *file, int line)
{
    /*
     * This function is called once per source file path
     * It will strip out the path before the src/ directory
     * and cache the result
     */

    auto p = std::make_pair(file, line);
    if (!file_to_str_mapping.contains(p))
    {
        // strip out stuff before src/ directory
        std::string file_str(file);
        auto pos = file_str.find("src/");
        if (pos == std::string::npos)
            throw std::runtime_error("logger: file not in src/ directory. unable to format");

        file_to_str_mapping[p] = mask_source(file_str.substr(pos) + ":" + std::to_string(line));
    }

    return file_to_str_mapping[p];
}

static std::string get_func_str(const char *func)
{
    /*
     * This function is called once per function name
     * It will cache the result
     */
    if (!func_to_str_mapping.contains(func))
        func_to_str_mapping[func] = mask_source(func);

    return func_to_str_mapping[func];
}

qpkg::core::Logger::Logger(qpkg::core::E level)
{
    m_level = level;
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
    /* Build log entry */
    /*
     * FORMAT:
     *   [logprefix] [timestamp] LEVEL [function] [file:line] message
     */

    std::stringstream log;

    /* Build message */
    log << '[' << get_runtime_logprefix() << "] ";
    log << '[' << get_timestamp() << ']';
    log << " ";
    log << level_to_str[m_level];
#if LOGGER_MASK_SRC == 1
    log << " <" << get_func_str(m_func) << get_file_str(m_file, m_line) << "> ";
#else
    log << " <" << get_func_str(m_func) << " | " << get_file_str(m_file, m_line) << "> ";
#endif
    log << escape_log_message(m_buffer.str());
    m_buffer.str("");

    /* Push log entry to queue */
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(log.str());
}

qpkg::core::Logger &qpkg::core::Logger::operator()(const char *func, const char *file, int line)
{
    /* Save in thread local storage */
    m_func = func;
    m_file = file;
    m_line = line;

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

__attribute__((noinline))
__attribute__((optimize("O0")))
__attribute__((naked))
__attribute__((used)) void
reference_address(void)
{
}

static std::string generate_backtrace()
{
    /*
     * Generate a backtrace
     * This function is called when a fault is detected
     */

    std::array<void *, 64> buffer = {};

    /* Generate backtrace */
    int nptrs = backtrace(buffer.data(), buffer.size());
    char **strings = backtrace_symbols(buffer.data(), nptrs);
    std::string backtrace_str;

    /* Use a reference address so devs can get around ASLR */
    uintptr_t ref = (uintptr_t)&reference_address;

    backtrace_str += "Reference address: ";

    // hex encode address
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(sizeof(uintptr_t) * 2) << std::hex << ref;
    backtrace_str += "0x" + ss.str() + std::string("\n");

    for (int i = 0; i < nptrs; i++)
        backtrace_str += strings[i] + std::string("\n");

    free(strings);

    return backtrace_str;
}

static void pushlog_onfault(int sig)
{
    (void)sig; // suppress unused warning

    static std::mutex m;
    std::lock_guard<std::mutex> lock(m); // only one thread can handle this

    LOG(qpkg::core::ERROR) << "Caught signal: " << sig << std::endl; // LOG is a macro that uses qpkg::core::LoggerSpool::getInst()[level]
    LOG(qpkg::core::INFO) << "Backtrace: " << generate_backtrace() << std::endl;
    LOG(qpkg::core::INFO) << "Fault. Dumping logs to " << qpkg::core::LoggerSpool::LOG_FILE << std::endl;

    qpkg::core::LoggerSpool::getInst().flush(qpkg::core::LoggerSpool::LOG_FILE);

    std::cerr << "Fault. Dumping logs to " << qpkg::core::LoggerSpool::LOG_FILE << std::endl;

    signal(SIGABRT, SIG_IGN);

    /* Kill process and generate core dump */
    abort();
}

static void pushlog_nonfatal(int sig)
{
    (void)sig; // suppress unused warning

    static std::mutex m;
    std::lock_guard<std::mutex> lock(m); // only one thread can handle this

    LOG(qpkg::core::WARN) << "Caught signal: " << sig << std::endl; // LOG is a macro that uses qpkg::core::LoggerSpool::getInst()[level]
    LOG(qpkg::core::INFO) << "Exiting... Dumping logs to " << qpkg::core::LoggerSpool::LOG_FILE << std::endl;

    qpkg::core::LoggerSpool::getInst().flush(qpkg::core::LoggerSpool::LOG_FILE);

    std::cerr << "Exiting... Dumping logs to " << qpkg::core::LoggerSpool::LOG_FILE << std::endl;

    /* Kill process and don't generate core dump */
    exit(1);
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

    signal(SIGSEGV, pushlog_onfault);
    signal(SIGABRT, pushlog_onfault);
    signal(SIGILL, pushlog_onfault);
    signal(SIGFPE, pushlog_onfault);
    signal(SIGBUS, pushlog_onfault);
    signal(SIGSYS, pushlog_onfault);
    signal(SIGXCPU, pushlog_onfault);
    signal(SIGXFSZ, pushlog_onfault);
    signal(SIGPIPE, pushlog_onfault);

    signal(SIGINT, pushlog_nonfatal);
    signal(SIGTERM, pushlog_nonfatal);
    signal(SIGHUP, pushlog_nonfatal);
    signal(SIGQUIT, pushlog_nonfatal);

    try
    {
        /*
            If this code fails, the process must exit to avoid a recursive call to terminate
        */

        // check if folders exist
        std::filesystem::path path(LOG_FILE);
        if (!std::filesystem::exists(path.parent_path()))
            std::filesystem::create_directories(path.parent_path());

        // check access
        if (std::filesystem::exists(path.parent_path()) && !std::filesystem::is_directory(path.parent_path()))
        {
            std::cerr << "logger: unable to create log file directories: " << path.parent_path() << " is not a directory" << std::endl;
            _exit(1); // exit to avoid recursive call to terminate
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
        std::cerr << "logger: " << e.what() << std::endl;
        _exit(1); // exit to avoid recursive call to terminate
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

    /* Flush all loggers to file */
    std::ofstream file(filepath, std::ios::app);
    if (!file.is_open())
        throw std::runtime_error("logger: unable to open log file \"" + std::string(LOG_FILE) + "\": " + std::string(strerror(errno)));

    /* Flush all loggers */
    for (auto &logger : m_loggers)
        logger.second->flush(file);

    /* Close file, implicit flush */
    file.close();
}
