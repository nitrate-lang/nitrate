#ifndef __QPKG_BUILD_ENGINE_HH__
#define __QPKG_BUILD_ENGINE_HH__

#include <string>
#include <set>
#include <optional>
#include <filesystem>
#include <cache/CacheDirectory.hh>
#include <conf/Parser.hh>

namespace qpkg
{
    namespace build
    {
        enum class Optimization
        {
            OPTIMIZE_DEFAULT,
            OPTIMIZE_SIZE,
            OPTIMIZE_SPEED
        };

        struct BuildSecurityConfig
        {
            std::set<std::string> trustkeys;
            std::optional<std::pair<std::string, std::string>> certify;
            bool disable_sigcheck;

            BuildSecurityConfig() : disable_sigcheck(false) {}

            BuildSecurityConfig(std::set<std::string> trustkeys,
                                std::optional<std::pair<std::string, std::string>> certify,
                                bool disable_sigcheck)
                : trustkeys(trustkeys),
                  certify(certify),
                  disable_sigcheck(disable_sigcheck) {}
        };

        class Engine
        {
            std::string m_package_src;
            std::unique_ptr<cache::ICache> m_cache;
            std::string m_output;
            int m_jobs;
            BuildSecurityConfig m_security;
            Optimization m_optimization;
            bool m_debug;
            bool m_verbose;

            std::optional<qpkg::conf::Config> load_config(const std::filesystem::path &base);

        public:
            Engine(const std::string &package_src,
                   const std::string &output,
                   int jobs,
                   std::unique_ptr<cache::ICache> cache,
                   BuildSecurityConfig security,
                   Optimization optimization,
                   bool debug, bool verbose);

            bool run();
        };
    }
}

#endif // __QPKG_BUILD_ENGINE_HH__