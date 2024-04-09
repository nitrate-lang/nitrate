#include <cache/CacheDirectory.hh>
#include <core/Logger.hh>
#include <filesystem>
#include <fstream>

constexpr char CACHE_INDEX_HEADER[] = "QPKG_CACHE_V1";

qpkg::cache::DirectoryCache::~DirectoryCache()
{
    if (!m_cacheDir.empty())
        this->sync();
}

void qpkg::cache::DirectoryCache::acquire_lock()
{
    m_mutex.lock();
}

void qpkg::cache::DirectoryCache::release_lock()
{
    m_mutex.unlock();
}

bool qpkg::cache::DirectoryCache::init()
{
    LOG(core::DEBUG) << "Initializing cache directory: " << m_cacheDir << std::endl;

    try
    {
        if (!std::filesystem::create_directories(m_cacheDir))
        {
            LOG(core::ERROR) << "Failed to create cache directory: " << m_cacheDir << std::endl;
            return false;
        }

        std::string record_file = m_cacheDir / "index.db";

        std::ofstream record(record_file, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!record.is_open())
        {
            LOG(core::ERROR) << "Failed to create cache index file: " << record_file << std::endl;
            return false;
        }

        record.write(CACHE_INDEX_HEADER, sizeof(CACHE_INDEX_HEADER));

        if (!std::filesystem::create_directories(m_cacheDir / "data"))
        {
            LOG(core::ERROR) << "Failed to create cache data directory: " << m_cacheDir / "data" << std::endl;
            return false;
        }

        LOG(core::DEBUG) << "Cache directory created: " << m_cacheDir << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        LOG(core::ERROR) << "Failed to create cache directory: " << m_cacheDir << std::endl;
        return false;
    }
}

struct Record
{
    uint8_t key[qpkg::cache::CacheKeySize];
    uint32_t timestamp;
} __attribute__((packed));

bool qpkg::cache::DirectoryCache::discover()
{
    LOG(core::DEBUG) << "Discovering cache directory: " << m_cacheDir << std::endl;

    std::string record_file = std::filesystem::path(m_cacheDir) / "index.db";

    try
    {
        std::ifstream record(record_file, std::ios::binary | std::ios::in);
        if (!record.is_open())
        {
            LOG(core::ERROR) << "Failed to open cache index file: " << record_file << std::endl;
            return false;
        }

        char header[sizeof(CACHE_INDEX_HEADER)];
        if (record.readsome(header, sizeof(header)) != sizeof(header) || std::string(header) != CACHE_INDEX_HEADER)
        {
            LOG(core::ERROR) << "Invalid cache index file: " << record_file << std::endl;
            return false;
        }

        Record entry;
        while (record.readsome(reinterpret_cast<char *>(&entry), sizeof(entry)) == sizeof(entry))
        {
            CacheKey key;
            std::copy(std::begin(entry.key), std::end(entry.key), std::begin(key));

            std::string filepath = m_cacheDir / "data" / keyToString(key);

            if (!std::filesystem::exists(filepath))
            {
                LOG(core::ERROR) << "Cache corrupted. Unable to locate file: " << filepath << std::endl;
                return false;
            }

            m_cacheMap[key] = std::make_pair(filepath, std::chrono::system_clock::from_time_t(entry.timestamp));
            m_keys.insert(key);
        }

        LOG(core::DEBUG) << "Found " << m_cacheMap.size() << " cache entries" << std::endl;

        return true;
    }
    catch (const std::exception &e)
    {
        LOG(core::ERROR) << "Failed to discover cache directory: " << m_cacheDir << std::endl;
        return false;
    }
}

bool qpkg::cache::DirectoryCache::setup(const std::string &location, bool init)
{
    m_cacheDir = std::filesystem::path(location) / ".qpkg" / "cache";

    if (std::filesystem::exists(m_cacheDir))
        return this->discover();

    if (init)
        return this->init();

    return false;
}

bool qpkg::cache::DirectoryCache::contains(const CacheKey &key)
{
    return m_cacheMap.contains(key);
}

std::set<qpkg::cache::CacheKey> qpkg::cache::DirectoryCache::keys()
{
    return m_keys;
}

void qpkg::cache::DirectoryCache::loadb(const CacheKey &key, std::vector<uint8_t> &value)
{
    if (!m_cacheMap.contains(key))
        throw std::out_of_range("Key not found in cache");

    if (!std::filesystem::exists(m_cacheMap[key].first))
    {
        LOG(core::ERROR) << "Cache corrupted. Unable to locate file: " << m_cacheMap[key].first << std::endl;
        throw std::runtime_error("Cache corrupted. Unable to locate file: " + m_cacheMap[key].first);
    }

    try
    {
        std::ifstream file(m_cacheMap[key].first, std::ios::binary | std::ios::in);
        if (!file.is_open())
            throw std::runtime_error("Failed to open cache file: " + m_cacheMap[key].first);

        file.seekg(0, std::ios::end);
        value.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(value.data()), value.size());
    }
    catch (const std::exception &e)
    {
        LOG(core::ERROR) << "Failed to load cache entry: " << keyToString(key) << std::endl;
        throw std::runtime_error("Failed to load cache entry: " + keyToString(key));
    }
}

std::string qpkg::cache::DirectoryCache::loadf(const CacheKey &key)
{
    if (!m_cacheMap.contains(key))
        throw std::out_of_range("Key not found in cache");

    if (!std::filesystem::exists(m_cacheMap[key].first))
    {
        LOG(core::ERROR) << "Cache corrupted. Unable to locate file: " << m_cacheMap[key].first << std::endl;
        throw std::runtime_error("Cache corrupted. Unable to locate file: " + m_cacheMap[key].first);
    }

    return m_cacheMap[key].first;
}

std::chrono::_V2::system_clock::time_point qpkg::cache::DirectoryCache::timestamp(const qpkg::cache::CacheKey &key)
{
    if (!m_cacheMap.contains(key))
        throw std::out_of_range("Key not found in cache");

    return m_cacheMap[key].second;
}

void qpkg::cache::DirectoryCache::storeb(const CacheKey &key, const std::vector<uint8_t> &value)
{
    std::string filepath = m_cacheDir / "data" / keyToString(key);

    try
    {
        std::ofstream file(filepath, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open())
            throw std::runtime_error("Failed to open cache file: " + filepath);

        file.write(reinterpret_cast<const char *>(value.data()), value.size());

        m_cacheMap[key] = std::make_pair(filepath, std::chrono::system_clock::now());
        m_keys.insert(key);
    }
    catch (const std::exception &e)
    {
        LOG(core::ERROR) << "Failed to store cache entry: " << keyToString(key) << std::endl;
        throw std::runtime_error("Failed to store cache entry: " + keyToString(key));
    }
}

#include <iostream>

void qpkg::cache::DirectoryCache::storef(const CacheKey &key, const std::string &filepath)
{
    std::string cache_file = m_cacheDir / "data" / keyToString(key);

    if (!std::filesystem::exists(filepath))
    {
        LOG(core::ERROR) << "File not found: " << filepath << std::endl;
        throw std::runtime_error("File not found: " + filepath);
    }

    try
    {
        std::filesystem::copy_file(filepath, cache_file, std::filesystem::copy_options::overwrite_existing);
        m_cacheMap[key] = std::make_pair(cache_file, std::chrono::system_clock::now());
        m_keys.insert(key);
    }
    catch (const std::exception &e)
    {
        LOG(core::ERROR) << "Failed to store cache entry: " << keyToString(key) << std::endl;
        throw std::runtime_error("Failed to store cache entry: " + keyToString(key));
    }
}

void qpkg::cache::DirectoryCache::remove(const CacheKey &key)
{
    m_cacheMap.erase(key);
    m_keys.erase(key);
}

void qpkg::cache::DirectoryCache::clear()
{
    m_cacheMap.clear();
    m_keys.clear();
}

size_t qpkg::cache::DirectoryCache::count()
{
    return m_cacheMap.size();
}

size_t qpkg::cache::DirectoryCache::size()
{
    sync();

    size_t size = 0;

    for (const auto &[_, value] : m_cacheMap)
    {
        if (!std::filesystem::exists(value.first))
        {
            LOG(core::ERROR) << "Cache corrupted. Unable to locate file: " << value.first << std::endl;
            throw std::runtime_error("Cache corrupted. Unable to locate file: " + value.first);
        }

        size += std::filesystem::file_size(value.first);
    }

    return size;
}

void qpkg::cache::DirectoryCache::sync()
{
    LOG(core::DEBUG) << "Syncing cache directory: " << m_cacheDir << std::endl;

    std::string record_file = std::filesystem::path(m_cacheDir) / "index.db";

    try
    {
        std::ofstream record(record_file, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!record.is_open())
        {
            LOG(core::ERROR) << "Failed to open cache index file: " << record_file << std::endl;
            throw std::runtime_error("Failed to open cache index file: " + record_file);
        }

        record.write(CACHE_INDEX_HEADER, sizeof(CACHE_INDEX_HEADER));

        for (const auto &[key, value] : m_cacheMap)
        {
            if (value.first.size() > std::numeric_limits<uint16_t>::max())
            {
                LOG(core::ERROR) << "Cache entry too large: " << keyToString(key) << std::endl;
                throw std::runtime_error("Cache entry too large: " + keyToString(key));
            }

            Record entry;
            std::copy(std::begin(key), std::end(key), std::begin(entry.key));
            entry.timestamp = std::chrono::system_clock::to_time_t(value.second);

            record.write(reinterpret_cast<const char *>(&entry), sizeof(entry));
        }

        LOG(core::DEBUG) << "Synced " << m_cacheMap.size() << " cache entries" << std::endl;
        LOG(core::DEBUG) << "Cache directory synced: " << m_cacheDir << std::endl;
    }
    catch (const std::exception &e)
    {
        LOG(core::ERROR) << "Failed to sync cache directory: " << m_cacheDir << std::endl;
        throw std::runtime_error("Failed to sync cache directory: " + m_cacheDir.string());
    }
}