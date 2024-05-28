#ifndef __QPKG_CORE_ADAPTER_HH__
#define __QPKG_CORE_ADAPTER_HH__

#include <string>
#include <functional>
#include <cstdint>

namespace qpkg
{
    namespace core
    {
        enum Level
        {
            DEBUG,
            GOOD,
            BOLD,
            INFO,
            WARN,
            ERROR,
            FATAL,
        };

        typedef std::function<void(const std::string &, Level lvl, float weight)> LoggerFlushCallback;

        class FormatAdapter
        {
            static std::string format(const std::string &msg, Level lvl, float weight);
            static void push(const std::string &msg, Level lvl, float weight, bool use_colors, bool debug);

        public:
            /// @brief Initialize the adapter
            /// @warning Must be called before any other Logger method
            static void PluginAndInit(bool debug = false, bool use_colors = true);
        };

    }
}

#endif // __QPKG_CORE_ADAPTER_HH__