#include <build/EngineBuilder.hh>
#include <core/Logger.hh>
#include <fstream>

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::set_package_src(const std::string &directory)
{
    m_package_src = directory;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::set_output(const std::string &output)
{
    m_output = output;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::disable_cache()
{
    m_disable_cache = true;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::jobs(int jobs)
{
    m_jobs = jobs;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::verbose()
{
    m_verbose = true;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::optimize()
{
    m_optimize = true;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::optimize_size()
{
    m_optimize_size = true;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::debug()
{
    m_debug = true;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::certify(const std::string &pkcs12)
{
    m_certify.first = pkcs12;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::certify_password(const std::string &password)
{
    m_certify.second = password;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::disable_sigcheck()
{
    m_disable_sigcheck = true;
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::trustkey(const std::string &key)
{
    m_trustkeys.insert(key);
    return *this;
}

qpkg::build::EngineBuilder &qpkg::build::EngineBuilder::trustkeys(const std::string &keyfile)
{
    m_trustkeyfiles.insert(keyfile);
    return *this;
}

static bool validate_trustkey(const std::string &key)
{
    if (key.size() != 66 || key[0] != '0' || key[1] != 'x')
        return false;

    for (size_t i = 2; i < key.size(); i++)
    {
        if (!std::isxdigit(key[i]))
            return false;
    }

    return true;
}

static bool validate_and_extract_trustkeyfile(const std::string &keyfile, std::set<std::string> &trustkeys)
{
    try
    {
        std::ifstream file(keyfile);
        if (!file.is_open())
            return false;

        std::string line;
        while (std::getline(file, line))
        {
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

            line = line.substr(0, line.find_first_of('#'));
            if (line.empty())
                continue;

            if (!validate_trustkey(line))
                return false;

            trustkeys.insert(line);
        }

        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

static bool validate_certify(const std::pair<std::string, std::string> &certify)
{
    (void)certify;
    /// TODO: PKCS12 validation
    return true;
}

qpkg::build::Engine qpkg::build::EngineBuilder::build()
{
    if (!std::filesystem::exists(m_package_src))
    {
        LOG(core::ERROR) << "Package source directory does not exist" << std::endl;
        throw std::runtime_error("Package source directory does not exist");
    }

    if (m_jobs < 0)
    {
        LOG(core::ERROR) << "Invalid number of jobs" << std::endl;
        throw std::runtime_error("Invalid number of jobs");
    }

    for (const auto &key : m_trustkeys)
    {
        if (!validate_trustkey(key))
        {
            LOG(core::ERROR) << "Invalid trust key: " << key << std::endl;
            throw std::runtime_error("Invalid trust key");
        }
    }

    for (const auto &keyfile : m_trustkeyfiles)
    {
        std::set<std::string> trustkeys;
        if (!validate_and_extract_trustkeyfile(keyfile, trustkeys))
        {
            LOG(core::ERROR) << "Invalid trust key file: " << keyfile << std::endl;
            throw std::runtime_error("Invalid trust key file");
        }

        m_trustkeys.insert(trustkeys.begin(), trustkeys.end());
    }

    if (!validate_certify(m_certify))
    {
        LOG(core::ERROR) << "Invalid certification" << std::endl;
        throw std::runtime_error("Invalid certification");
    }

    BuildSecurityConfig security(m_trustkeys,
                                 m_certify.first.empty() ? std::nullopt : std::make_optional(m_certify),
                                 m_disable_sigcheck);

    Optimization optimization;
    if (m_optimize)
        optimization = Optimization::OPTIMIZE_SPEED;
    else if (m_optimize_size)
        optimization = Optimization::OPTIMIZE_SIZE;
    else
        optimization = Optimization::OPTIMIZE_DEFAULT;

    std::unique_ptr<cache::ICache> cache;
    if (!m_disable_cache)
    {
        cache = std::make_unique<cache::DirectoryCache>();
        if (!cache->setup(m_package_src, true))
        {
            LOG(core::ERROR) << "Failed to setup cache" << std::endl;
            throw std::runtime_error("Failed to setup cache");
        }
    }

    return Engine(m_package_src, m_output, m_jobs, std::move(cache), security, optimization, m_debug, m_verbose);
}