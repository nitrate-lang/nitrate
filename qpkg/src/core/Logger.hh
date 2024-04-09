#ifndef __QPKG_CORE_LOGGER_HH__
#define __QPKG_CORE_LOGGER_HH__

#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <memory>
#include <chrono>
#include <sstream>

namespace qpkg
{
    namespace core
    {
        /// @brief Log levels
        enum E
        {
            DEBUG = 0,
            INFO = 1,
            WARN = 2,
            ERROR = 3,
            FAILED = 4 // Like ERROR, but does not terminate the program
        };

        class Logger
        {
            std::queue<std::string> m_queue;
            std::mutex m_mutex;
            thread_local static std::ostringstream m_buffer;
            thread_local static const char *m_func;
            thread_local static const char *m_file;
            thread_local static int m_line;
            E m_level;
            bool m_verbose;
            void flush(std::ofstream &file);

            friend class LoggerSpool;

        public:
            Logger(E level = DEBUG);

            inline void on(bool v = true) { m_verbose = v; }

            template <class T>
            Logger &operator+=(const T &msg) // append to thread-local buffer
            {
                m_buffer << msg;
                return *this;
            }

            Logger &operator()(const char *func = __PRETTY_FUNCTION__, const char *file = __FILE__, int line = __LINE__);

            // construct and push log message
            void push();
        };

        /// @brief Thread-safe logger spool
        class LoggerSpool
        {
            std::unordered_map<E, std::unique_ptr<Logger>> m_loggers;
            bool m_okay;
            LoggerSpool(std::chrono::duration<int> flushInterval = FLUSH_INTERVAL);

        public:
            static LoggerSpool &getInst();
            Logger &operator[](E level);

            /// @brief Synchronize all loggers
            void flush(const char *filepath);

            constexpr static std::chrono::duration<int> FLUSH_INTERVAL = std::chrono::seconds(1);
            constexpr static const char *LOG_FILE = "/var/log/quix/qpkg.log";
        };

#define LOG(lvl) qpkg::core::LoggerSpool::getInst()[lvl](__PRETTY_FUNCTION__, __FILE__, __LINE__)

        template <class T>
        Logger &operator<<(Logger &logger, const T &msg)
        {
            // Append generic datatype to buffer
            return logger += msg;
        }

        /// @brief Log messages must be terminated with std::endl
        Logger &operator<<(Logger &log, std::ostream &(*var)(std::ostream &));
    };
};

#endif // __QPKG_CORE_LOGGER_HH__