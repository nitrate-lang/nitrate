#include <build/Engine.hh>
#include <core/Logger.hh>
#include <conf/Validate.hh>
#include <set>
#include <fstream>
#include <thread>
#include <quixcc.hpp>

#include <iostream>

#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif

using namespace qpkg::core;
using namespace qpkg::build;

qpkg::build::Engine::Engine(const std::string &package_src,
                            const std::string &output,
                            int jobs, std::unique_ptr<cache::ICache> cache,
                            qpkg::build::BuildSecurityConfig security,
                            qpkg::build::Optimization optimization,
                            bool debug,
                            bool verbose)
{
    if (verbose)
        LOG_ENABLE(core::DEBUG);

    m_package_src = package_src;
    m_output = output;
    m_cache = std::move(cache);
    m_security = security;
    m_optimization = optimization;
    m_debug = debug;
    m_verbose = verbose;

    m_jobs = jobs;

    if (m_jobs == 0)
    {
#ifdef __linux__
        m_jobs = sysconf(_SC_NPROCESSORS_ONLN);

        if (m_jobs < 1)
            m_jobs = 1;
#else
        m_jobs = 1;
#endif
    }

    if (!quixcc_init())
    {
        LOG(core::ERROR) << "Failed to initialize QuixCC" << std::endl;
        exit(1);
    }
}

std::optional<qpkg::conf::Config> qpkg::build::Engine::load_config(const std::filesystem::__cxx11::path &base)
{
    if (std::filesystem::exists(base / "qpkg.yaml"))
        return conf::YamlConfigParser().parsef(base / "qpkg.yaml");
    else
    {
        LOG(core::ERROR) << "No configuration file found in package source directory" << std::endl;
        return std::nullopt;
    }
}

std::vector<std::string> qpkg::build::Engine::get_source_files(const std::filesystem::__cxx11::path &base)
{
    std::vector<std::string> source_code_files;

    for (const auto &path : m_config["sources"].as<std::vector<std::string>>())
    {
        std::filesystem::path abs(base / path);

        if (std::filesystem::is_directory(abs))
        {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(abs))
                if (entry.path().string().ends_with(".q") && entry.is_regular_file())
                    source_code_files.push_back(std::filesystem::canonical(entry.path()).string());
        }
        else if (abs.string().ends_with(".q") && std::filesystem::is_regular_file(abs))
        {
            source_code_files.push_back(std::filesystem::canonical(abs).string());
        }
    }

    return source_code_files;
}

static qpkg::cache::CacheKey compute_cachekey(const std::filesystem::__cxx11::path &file)
{
    std::ifstream in(file, std::ios::binary);
    if (!in)
        throw std::runtime_error("Failed to open file");

    uint8_t buffer[4096];
    qpkg::cache::CacheHasher hasher;

    while (in)
    {
        in.read(reinterpret_cast<char *>(buffer), sizeof(buffer));
        hasher.update(buffer, in.gcount());
    }

    return hasher.finalize();
}

static int get_terminal_width()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

class CompilationProgressPrinter
{
    enum class State
    {
        NOT_STARTED,
        STARTING,
        FINISHED,
    };

    size_t m_total, m_real, m_current;
    State m_linker;
    State m_state;
    bool m_linking;
    std::queue<std::pair<State, std::string>> m_messages;

public:
    CompilationProgressPrinter() : m_total(0), m_real(0), m_current(0), m_linker(State::NOT_STARTED), m_state(State::NOT_STARTED), m_linking(false) {}

    void reset(size_t total, size_t real)
    {
        m_total = total;
        m_real = real;
        m_current = 0;
    }

    void starting(const std::string &msg)
    {
        if (m_current >= m_total)
            return;

        m_messages.push({State::STARTING, msg});
    }

    void finished(const std::string &msg)
    {
        if (m_current >= m_total)
            return;

        m_current++;

        m_messages.push({State::FINISHED, msg});
    }

    void linking()
    {
        m_linking = true;
        m_linker = State::STARTING;
    }

    void linked()
    {
        m_linker = State::FINISHED;
    }

    void print()
    {
        uint8_t percent = m_total == 0 ? 0 : (m_current * 100) / m_total;
        std::string percent_pad = "[";
        if (percent < 10)
            percent_pad += "  ";
        else if (percent < 100)
            percent_pad += " ";

        percent_pad += std::to_string(percent) + "%]";

        int width = get_terminal_width();

        // Clear the line
        std::cout << "\x1b[2K";

        while (!m_messages.empty())
        {
            auto [state, msg] = m_messages.front();
            m_messages.pop();

            switch (state)
            {
            case State::STARTING:
                std::cout << percent_pad << " \x1b[32mBuilding QUIX object " << msg << "\x1b[0m" << std::endl;
                break;
            case State::FINISHED:
                std::cout << percent_pad << " \x1b[32mFinished building QUIX object " << msg << "\x1b[0m" << std::endl;
                break;
            default:
                break;
            }
        }

        if (m_linking)
        {
            std::cout << percent_pad << " \x1b[32;1mLinking QUIX target\x1b[0m" << std::endl;
            m_linking = false;
        }

        std::cout << "\r\x1b[2K\x1b[37;42mProgress: " << percent_pad << "\x1b[0m [";

        int bar_width = width - 20;
        int pos = bar_width * percent / 100;

        for (int i = 0; i < bar_width; i++)
        {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        if (percent < 100)
            std::cout << "]\r";
        else if (m_linker == State::FINISHED)
        {
            std::cout << "\r\x1b[2K";
            std::cout << "Build complete" << std::endl;
            m_state = State::FINISHED;
        }

        std::cout.flush();
    }

    bool is_done()
    {
        return m_state == State::FINISHED;
    }
};

static CompilationProgressPrinter g_cc_printer;
static std::mutex g_cc_printer_mutex;

bool qpkg::build::Engine::build_source_file(const std::filesystem::__cxx11::path &base, const std::filesystem::__cxx11::path &build_dir, const std::filesystem::__cxx11::path &file) const
{
    cache::CacheKey key;
    std::string outfile = build_dir.string() + "/" + file.filename().string() + ".o";

    if (m_cache)
    {
        key = compute_cachekey(file);

        m_cache->acquire_lock();

        if (m_cache->contains(key))
        {
            LOG(core::DEBUG) << "Using cached object file for " << file << std::endl;

            auto cache_file = m_cache->loadf(key);
            std::filesystem::copy_file(cache_file, outfile, std::filesystem::copy_options::overwrite_existing);

            m_cache->release_lock();
            return true;
        }

        m_cache->release_lock();
    }

    LOG(core::DEBUG) << "Compiling source file " << file << std::endl;

    quixcc::CompilerBuilder builder;
    builder.set_output(outfile);
    builder.set_debug(m_debug);
    builder.set_verbosity(m_verbose ? quixcc::Verbosity::VERY_VERBOSE : quixcc::Verbosity::NORMAL);
    switch (m_optimization)
    {
    case Optimization::OPTIMIZE_DEFAULT:
        builder.set_optimization(quixcc::OptimizationLevel::SPEED_2);
        break;
    case Optimization::OPTIMIZE_SPEED:
        builder.set_optimization(quixcc::OptimizationLevel::SPEED_4);
        break;
    case Optimization::OPTIMIZE_SIZE:
        builder.set_optimization(quixcc::OptimizationLevel::SIZE);
        break;
    }

    for (const auto &header : m_config["headers"].as<std::vector<std::string>>())
        builder.add_include((base / header).string());

    builder.add_include(base.string());
    builder.add_source(file.string());
    builder.opt("-c");
    for (const auto &flag : m_config["cflags"].as<std::vector<std::string>>())
        builder.opt(flag);

    if (m_build_type == BuildType::SHAREDLIB)
        builder.opt("-shared");
    else if (m_build_type == BuildType::STATICLIB)
        builder.opt("-fPIC");

    builder.target(m_config["triple"].as<std::string>());
    builder.cpu(m_config["cpu"].as<std::string>());

    if (!builder.build().run(1).puts().ok())
    {
        LOG(core::ERROR) << "Failed to compile source file " << file << std::endl;
        return false;
    }

    if (m_cache)
    {
        m_cache->acquire_lock();
        m_cache->storef(key, outfile);
        m_cache->release_lock();
    }

    return true;
}

bool qpkg::build::Engine::link_objects(const std::vector<std::filesystem::__cxx11::path> &objects) const
{
    std::string cmd = "qld";

    for (const auto &flag : m_config["lflags"].as<std::vector<std::string>>())
        cmd += " " + flag;

    cmd += " -o '" + m_output + "'";

    switch (m_build_type)
    {
    case BuildType::SHAREDLIB:
        cmd += " -sharedlib";
        break;
    case BuildType::STATICLIB:
        cmd += " -staticlib";
        break;
    case BuildType::EXECUTABLE:
        break;
    }

    for (const auto &file : objects)
        cmd += " '" + file.string() + "'";

    if (m_debug)
        cmd += " -g";

    if (m_verbose)
        cmd += " -vv";

    LOG(core::DEBUG) << "Invoking linker: " << cmd << std::endl;

    if (system(cmd.c_str()) != 0)
        return false;

    return true;
}

void qpkg::build::Engine::run_threads(const std::filesystem::__cxx11::path &base, const std::vector<std::string> &source_files, const std::filesystem::__cxx11::path &build_dir) const
{
    size_t i, tcount;
    std::vector<std::thread> threads;

    tcount = std::min<size_t>(m_jobs, source_files.size());

    g_cc_printer_mutex.lock();
    g_cc_printer.reset(source_files.size(), source_files.size());
    g_cc_printer_mutex.unlock();

    for (i = 0; i < tcount; i++)
    {
        threads.push_back(std::thread([this, &base, &build_dir, &source_files, i, tcount]()
                                      {
            for (size_t j = i; j < source_files.size(); j += tcount)
            {
                g_cc_printer_mutex.lock();
                g_cc_printer.starting(source_files[j]);
                g_cc_printer_mutex.unlock();

                if (!build_source_file(base, build_dir, source_files[j]))
                {
                    LOG(core::ERROR) << "Failed to compile source file" << std::endl;
                    exit(1);
                }

                g_cc_printer_mutex.lock();
                g_cc_printer.finished(source_files[j]);
                g_cc_printer_mutex.unlock();
            } }));
    }

    std::thread printer_thread([]()
                               {
        while (true)
        {
            g_cc_printer_mutex.lock();
            g_cc_printer.print();
            bool done = g_cc_printer.is_done();
            g_cc_printer_mutex.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(50));


            if (done)
                break;
        } });

    printer_thread.detach();

    for (auto &thread : threads)
        thread.join();
}

bool qpkg::build::Engine::build_package(const std::filesystem::__cxx11::path &base, const std::vector<std::string> &source_files, const std::filesystem::__cxx11::path &build_dir)
{
    run_threads(base, source_files, build_dir);

    std::vector<std::filesystem::path> objects;
    for (const auto &file : source_files)
        objects.push_back(build_dir / (std::filesystem::path(file).filename().string() + ".o"));

    if (m_config["nolink"].as<bool>() == false)
    {
        g_cc_printer_mutex.lock();
        g_cc_printer.linking();
        g_cc_printer_mutex.unlock();

        if (!link_objects(objects))
        {
            LOG(core::ERROR) << "Failed to link object files" << std::endl;
            return false;
        }

        g_cc_printer_mutex.lock();
        g_cc_printer.linked();
        g_cc_printer_mutex.unlock();
    }
    else
    {
        LOG(core::DEBUG) << "Skipping linking" << std::endl;

        g_cc_printer_mutex.lock();
        g_cc_printer.linked();
        g_cc_printer_mutex.unlock();
    }

    while (true)
    {
        g_cc_printer_mutex.lock();
        bool done = g_cc_printer.is_done();
        g_cc_printer_mutex.unlock();

        if (done)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return true;
}

bool qpkg::build::Engine::run()
{
    std::filesystem::path base(m_package_src);

    auto _config = load_config(base);

    if (!_config || !conf::ValidateConfig(*_config, base))
        return false;

    m_config = *_config;
    conf::PopulateConfig(m_config);

    std::filesystem::path build_dir = base / ".qpkg" / "build";

    if (!std::filesystem::exists(build_dir))
        std::filesystem::create_directories(build_dir);

    if (m_config["target"].as<std::string>() == "sharedlib")
        m_build_type = BuildType::SHAREDLIB;
    else if (m_config["target"].as<std::string>() == "staticlib")
        m_build_type = BuildType::STATICLIB;
    else if (m_config["target"].as<std::string>() == "executable")
        m_build_type = BuildType::EXECUTABLE;
    else
    {
        LOG(core::ERROR) << "Invalid target type" << std::endl;
        return false;
    }

    if (m_output.empty())
    {
        switch (m_build_type)
        {
        case BuildType::SHAREDLIB:
            m_output = base / ("lib" + m_config["name"].as<std::string>() + ".so");
            std::filesystem::remove(m_output);
            break;
        case BuildType::STATICLIB:
            m_output = base / ("lib" + m_config["name"].as<std::string>() + ".a");
            std::filesystem::remove(m_output);
            break;
        case BuildType::EXECUTABLE:
            m_output = base / m_config["name"].as<std::string>();
            std::filesystem::remove(m_output);
            break;
        }
    }

    return build_package(base, get_source_files(base), build_dir);
}