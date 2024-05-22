#ifndef __QPKG_CACHE_LOCALCACHE_HH__
#define __QPKG_CACHE_LOCALCACHE_HH__

#include <cache/GenericCache.hh>
#include <string>
#include <map>
#include <filesystem>
#include <mutex>

namespace qpkg
{
    namespace cache
    {
        class DirectoryCache : public ICache
        {
            std::filesystem::path m_cacheDir;
            std::map<CacheKey, std::pair<std::string, std::chrono::system_clock::time_point>> m_cacheMap;
            std::set<CacheKey> m_keys;
            std::mutex m_mutex;

            bool init();
            bool discover();

            DirectoryCache(const DirectoryCache &) = delete;
            DirectoryCache &operator=(const DirectoryCache &) = delete;

        public:
            DirectoryCache() = default;
            ~DirectoryCache();

            void acquire_lock() override;
            void release_lock() override;
            bool setup(const std::string &location, bool init = false) override;
            bool contains(const CacheKey &key) override;
            std::set<CacheKey> keys() override;
            void loadb(const CacheKey &key, std::vector<uint8_t> &value) override;
            std::string loadf(const CacheKey &key) override;
            std::chrono::system_clock::time_point timestamp(const CacheKey &key) override;
            void storeb(const CacheKey &key, const std::vector<uint8_t> &value) override;
            void storef(const CacheKey &key, const std::string &filepath) override;
            void remove(const CacheKey &key) override;
            void clear() override;
            inline std::string location() override { return m_cacheDir; }
            size_t count() override;
            size_t size() override;
            void sync() override;
        };
    }
}

#endif // __QPKG_CACHE_LOCALCACHE_HH__