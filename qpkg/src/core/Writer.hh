#ifndef __QPKG_CORE_WRITER_HH__
#define __QPKG_CORE_WRITER_HH__

#include <string>

namespace qpkg {
namespace core {
void log_owrite(const std::string &msg);
void log_ewrite(const std::string &msg);
}  // namespace core
}  // namespace qpkg

#endif  // __QPKG_CORE_WRITER_HH__