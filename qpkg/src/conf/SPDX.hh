#ifndef __QPKG_CONF_SPDX_HH__
#define __QPKG_CONF_SPDX_HH__

#include <map>
#include <string>

namespace qpkg
{
    namespace conf
    {
        extern const std::map<std::string, const char*> valid_licenses;
    }
}

#endif // __QPKG_CONF_SPDX_HH__