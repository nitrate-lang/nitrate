#include <core/Collector.hh>
#include <memory>

qpkg::core::LoggerStream::LoggerStream(std::ostringstream &stream, std::function<void(const std::string &)> flush)
    : m_stream(stream), m_flush(flush)
{
}

qpkg::core::LoggerStream &qpkg::core::LoggerStream::operator<<(std::ostream &(*)(std::ostream &))
{
    m_flush(m_stream.str());
    return *this;
}

///=============================================================================

qpkg::core::ThreadLogger &qpkg::core::ThreadLogger::get(qpkg::core::Level level, qpkg::core::LoggerFlushCallback flush)
{
    static thread_local std::map<Level, std::unique_ptr<ThreadLogger>> m_instances;

    if (!m_instances.count(level))
        m_instances[level] = std::make_unique<ThreadLogger>(level, flush);

    m_instances[level]->m_stream.str("");
    return *m_instances[level];
}

qpkg::core::ThreadLogger::ThreadLogger(qpkg::core::Level level, qpkg::core::LoggerFlushCallback flush)
    : m_flush(flush), m_level(level)
{
}

qpkg::core::ThreadLogger &qpkg::core::ThreadLogger::push(const std::string &msg, float weight)
{
    m_flush(msg, m_level, weight);
    return *this;
}

qpkg::core::LoggerStream qpkg::core::ThreadLogger::stream(float weight)
{
    return qpkg::core::LoggerStream(m_stream, [this, weight](const std::string &msg)
                                    { m_flush(msg, m_level, weight); });
}

///=============================================================================

qpkg::core::GlobalCollector &qpkg::core::GlobalCollector::get()
{
    static GlobalCollector *instance = new GlobalCollector();
    return *instance;
}

void qpkg::core::GlobalCollector::setWriter(qpkg::core::LoggerFlushCallback new_flush)
{
    m_flush = new_flush;
}

void qpkg::core::GlobalCollector::push(const std::string &msg, qpkg::core::Level lvl, float weight)
{
    m_flush(msg, lvl, weight);
}

qpkg::core::ThreadLogger &qpkg::core::GlobalCollector::debug()
{
    return ThreadLogger::get(Level::DEBUG, m_flush);
}

qpkg::core::ThreadLogger &qpkg::core::GlobalCollector::good()
{
    return ThreadLogger::get(Level::GOOD, m_flush);
}

qpkg::core::ThreadLogger &qpkg::core::GlobalCollector::bold()
{
    return ThreadLogger::get(Level::BOLD, m_flush);
}

qpkg::core::ThreadLogger &qpkg::core::GlobalCollector::info()
{
    return ThreadLogger::get(Level::INFO, m_flush);
}

qpkg::core::ThreadLogger &qpkg::core::GlobalCollector::warn()
{
    return ThreadLogger::get(Level::WARN, m_flush);
}

qpkg::core::ThreadLogger &qpkg::core::GlobalCollector::error()
{
    return ThreadLogger::get(Level::ERROR, m_flush);
}

qpkg::core::ThreadLogger &qpkg::core::GlobalCollector::fatal()
{
    return ThreadLogger::get(Level::FATAL, m_flush);
}

///=============================================================================

qpkg::core::Process::Process(const std::string &name)
    : m_name(name), m_phase(Phase::PENDING)
{
}

qpkg::core::Process qpkg::core::Process::create(const std::string &name)
{
    return Process(name);
}

qpkg::core::Process &qpkg::core::Process::debug(const std::string &msg, float weight)
{
    m_phase = Phase::RUNNING;
    GlobalCollector::get().push(msg, Level::DEBUG, weight);
    return *this;
}

qpkg::core::Process &qpkg::core::Process::good(const std::string &msg, float weight)
{
    m_phase = Phase::RUNNING;
    GlobalCollector::get().push(msg, Level::GOOD, weight);
    return *this;
}

qpkg::core::Process &qpkg::core::Process::bold(const std::string &msg, float weight)
{
    m_phase = Phase::RUNNING;
    GlobalCollector::get().push(msg, Level::BOLD, weight);
    return *this;
}

qpkg::core::Process &qpkg::core::Process::info(const std::string &msg, float weight)
{
    m_phase = Phase::RUNNING;
    GlobalCollector::get().push(msg, Level::INFO, weight);
    return *this;
}

qpkg::core::Process &qpkg::core::Process::warn(const std::string &msg, float weight)
{
    m_phase = Phase::RUNNING;
    GlobalCollector::get().push(msg, Level::WARN, weight);
    return *this;
}

qpkg::core::Process &qpkg::core::Process::error(const std::string &msg, float weight)
{
    m_phase = Phase::RUNNING;
    GlobalCollector::get().push(msg, Level::ERROR, weight);
    return *this;
}

qpkg::core::Process &qpkg::core::Process::fatal(const std::string &msg, float weight)
{
    m_phase = Phase::RUNNING;
    GlobalCollector::get().push(msg, Level::FATAL, weight);
    return *this;
}

qpkg::core::LoggerStream qpkg::core::Process::stream(qpkg::core::Level lvl, float weight)
{
    m_phase = Phase::RUNNING;
    switch (lvl)
    {
    case Level::DEBUG:
        return GlobalCollector::get().debug().stream(weight);
    case Level::GOOD:
        return GlobalCollector::get().good().stream(weight);
    case Level::BOLD:
        return GlobalCollector::get().bold().stream(weight);
    case Level::INFO:
        return GlobalCollector::get().info().stream(weight);
    case Level::WARN:
        return GlobalCollector::get().warn().stream(weight);
    case Level::ERROR:
        return GlobalCollector::get().error().stream(weight);
    case Level::FATAL:
        return GlobalCollector::get().fatal().stream(weight);
    }
}

void qpkg::core::Process::done()
{
    m_phase = Phase::HALTED;
}

///=============================================================================

qpkg::core::LoggerStream qpkg::core::LOG(qpkg::core::Level lvl)
{
    switch (lvl)
    {
    case Level::DEBUG:
        return GlobalCollector::get().debug().stream(0);
    case Level::GOOD:
        return GlobalCollector::get().good().stream(0);
    case Level::BOLD:
        return GlobalCollector::get().bold().stream(0);
    case Level::INFO:
        return GlobalCollector::get().info().stream(0);
    case Level::WARN:
        return GlobalCollector::get().warn().stream(0);
    case Level::ERROR:
        return GlobalCollector::get().error().stream(0);
    case Level::FATAL:
        return GlobalCollector::get().fatal().stream(0);
    }
}