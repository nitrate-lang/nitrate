#ifndef __QPKG_BUILD_ENGINE_BUILDER_HH__
#define __QPKG_BUILD_ENGINE_BUILDER_HH__

#include <string>
#include <set>
#include <build/Engine.hh>

namespace qpkg
{
    namespace build
    {
        class EngineBuilder
        {
            std::string m_package_src;
            std::string m_output;
            std::set<std::string> m_trustkeys;
            std::set<std::string> m_trustkeyfiles;
            std::pair<std::string, std::string> m_certify;
            int m_jobs;
            bool m_disable_cache;
            bool m_verbose;
            bool m_optimize;
            bool m_optimize_size;
            bool m_debug;
            bool m_disable_sigcheck;

        public:
            EngineBuilder() : m_jobs(0), m_disable_cache(false), m_verbose(false),
                              m_optimize(false), m_optimize_size(false),
                              m_debug(false), m_disable_sigcheck(false) {}

            EngineBuilder &set_package_src(const std::string &directory);
            EngineBuilder &set_output(const std::string &output);
            EngineBuilder &disable_cache();
            EngineBuilder &jobs(int jobs);
            EngineBuilder &verbose();
            EngineBuilder &optimize();
            EngineBuilder &optimize_size();
            EngineBuilder &debug();
            EngineBuilder &certify(const std::string &pkcs12);
            EngineBuilder &certify_password(const std::string &password);
            EngineBuilder &disable_sigcheck();
            EngineBuilder &trustkey(const std::string &key);
            EngineBuilder &trustkeys(const std::string &keyfile);

            Engine build();
        };
    }
}

#endif // __QPKG_BUILD_ENGINE_BUILDER_HH__