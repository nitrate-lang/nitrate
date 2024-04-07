#ifndef __QPKG_CACHE_LOCALCACHE_HH__
#define __QPKG_CACHE_LOCALCACHE_HH__

#include <cache/GenericCache.hh>
#include <string>
#include <map>
#include <filesystem>

namespace qpkg
{
    namespace cache
    {
        class DirectoryCache : public ICache
        {
            std::filesystem::path m_cacheDir;
            std::map<CacheKey, std::pair<std::string, std::chrono::system_clock::time_point>> m_cacheMap;
            std::set<CacheKey> m_keys;

            bool init();
            bool discover();

        public:
            DirectoryCache() = default;
            ~DirectoryCache();

            bool setup(const std::string &location, bool init = false) override;
            bool contains(const CacheKey &key) override;
            std::set<CacheKey> keys() override;
            void loadb(const CacheKey &key, std::vector<uint8_t> &value);
            std::string loadf(const CacheKey &key);
            std::chrono::system_clock::time_point timestamp(const CacheKey &key);
            void storeb(const CacheKey &key, const std::vector<uint8_t> &value);
            void storef(const CacheKey &key, const std::string &filepath);
            void remove(const CacheKey &key);
            void clear();
            inline std::string location() { return m_cacheDir; }
            size_t count();
            size_t size();
            void sync() override;
        };
    }
}

#endif // __QPKG_CACHE_LOCALCACHE_HH__