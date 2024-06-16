#ifndef __QPKG_CLEAN_CLEANUP_HH__
#define __QPKG_CLEAN_CLEANUP_HH__

#include <filesystem>
#include <optional>
#include <set>
#include <string>

namespace qpkg {
namespace clean {
bool CleanPackageSource(const std::string &package_src, bool verbose = false);
}
}  // namespace qpkg

#endif  // __QPKG_CLEAN_CLEANUP_HH__