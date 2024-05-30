#ifndef __QPKG_BUILD_ENGINE_HH__
#define __QPKG_BUILD_ENGINE_HH__

#include <string>
#include <set>
#include <optional>
#include <filesystem>
#include <cache/CacheDirectory.hh>
#include <conf/Config.hh>
#include <core/Logger.hh>

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
            enum class BuildType
            {
                SHAREDLIB,
                STATICLIB,
                EXECUTABLE
            };

            std::string m_package_src;
            std::string m_package_name;
            std::unique_ptr<cache::ICache> m_cache;
            conf::Config m_config;
            std::string m_output;
            int m_jobs;
            BuildSecurityConfig m_security;
            Optimization m_optimization;
            bool m_debug;
            bool m_verbose;
            BuildType m_build_type;

            std::optional<qpkg::conf::Config> load_config(const std::filesystem::path &base);
            std::vector<std::string> get_source_files(const std::filesystem::path &base);
            bool run_threads(const std::filesystem::path &base, const std::vector<std::string> &source_files,
                             const std::filesystem::path &build_dir, qpkg::core::Process &log) const;
            bool build_package(const std::filesystem::path &base, const std::vector<std::string> &source_files, const std::filesystem::path &build_dir);
            bool build_source_file(const std::filesystem::path &base, const std::filesystem::path &build_dir, const std::filesystem::path &file) const;
            bool link_objects(const std::vector<std::filesystem::path> &objects) const;

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