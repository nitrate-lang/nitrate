#ifndef __QPKG_CLEAN_CLEANUP_HH__
#define __QPKG_CLEAN_CLEANUP_HH__

#include <string>
#include <set>
#include <optional>
#include <filesystem>

namespace qpkg
{
    namespace clean
    {
        bool CleanPackageSource(const std::string &package_src, bool recursive = false, bool verbose = false);
    }
}

#endif // __QPKG_CLEAN_CLEANUP_HH__