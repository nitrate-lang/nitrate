#ifndef __QPKG_CONF_VALIDATE_HH__
#define __QPKG_CONF_VALIDATE_HH__

#include <conf/Parser.hh>
#include <filesystem>

namespace qpkg
{
    namespace conf
    {
        bool ValidateConfig(const Config &config, const std::filesystem::path &base);
        void PopulateConfig(Config &config);
    }
}

#endif // __QPKG_CONF_VALIDATE_HH__