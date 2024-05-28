#include <core/Adapter.hh>
#include <core/Writer.hh>
#include <core/Collector.hh>
#include <mutex>
#include <sstream>

std::string qpkg::core::FormatAdapter::format(const std::string &msg, qpkg::core::Level lvl, float weight)
{
    static std::mutex m_mutex;
    std::lock_guard<std::mutex> lock(m_mutex);

    struct State
    {
        float m_progress;
    } state;

    state.m_progress += weight;

    if (state.m_progress < 0.0f)
        state.m_progress = 0.0f;
    else if (state.m_progress > 1.0f)
        state.m_progress = 1.0f;

    int percent = static_cast<int>(state.m_progress * 100.0f);

    std::stringstream ss;

    ss << "[";
    if (percent < 10)
        ss << "  ";
    else if (percent < 100)
        ss << " ";
    ss << percent << "%] ";

    switch (lvl)
    {
    case qpkg::core::Level::DEBUG:
        ss << msg;
        break;
    case qpkg::core::Level::GOOD:
        ss << "\x1b[32;49m" << msg << "\x1b[0m";
        break;
    case qpkg::core::Level::BOLD:
        ss << "\x1b[32;49;1m" << msg << "\x1b[0m";
        break;
    case qpkg::core::Level::INFO:
        ss << msg;
        break;
    case qpkg::core::Level::WARN:
        ss << "\x1b[36;49m" << msg << "\x1b[0m";
        break;
    case qpkg::core::Level::ERROR:
        ss << "\x1b[31;49;1m" << msg << "\x1b[0m";
        break;
    case qpkg::core::Level::FATAL:
        ss << "\x1b[31;49;1;4m" << msg << "\x1b[0m";
        break;
    }

    return ss.str();
}

void qpkg::core::FormatAdapter::push(const std::string &msg, qpkg::core::Level lvl, float weight, bool use_colors, bool debug)
{
    if (lvl == qpkg::core::Level::DEBUG && !debug)
        return;

    (void)use_colors;

    /// TODO: implement no-color support

    if (lvl == qpkg::core::Level::ERROR || lvl == qpkg::core::Level::FATAL)
    {
        log_ewrite(format(msg, lvl, weight));
    }
    else
    {
        log_owrite(format(msg, lvl, weight));
    }
}

void qpkg::core::FormatAdapter::PluginAndInit(bool debug, bool use_colors)
{
    auto push_f = [use_colors, debug](const std::string &msg, qpkg::core::Level lvl, float weight)
    {
        push(msg, lvl, weight, use_colors, debug);
    };

    auto &glog = GlobalCollector::get();
    glog.setWriter(push_f);
}