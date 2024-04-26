////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <LibMacro.h>
#include <mangle/Symbol.h>
#include <parse/nodes/AllNodes.h>

const std::string libquixcc::Symbol::quix_abiprefix = "_ZJ0";
const std::string libquixcc::Symbol::cxx_abiprefix = "_Z";
const std::string libquixcc::Symbol::c_abiprefix = "";

std::string libquixcc::Symbol::join(const std::string &a, const std::string &b)
{
    if (a.empty())
        return b;
    if (b.empty())
        return a;
    return a + "::" + b;
}

std::string libquixcc::Symbol::join(const std::vector<std::string> &namespaces, const std::string &name)
{
    std::string result;
    for (size_t i = 0; i < namespaces.size(); i++)
        result = join(result, namespaces[i]);

    return join(result, name);
}

std::string libquixcc::Symbol::mangle(const libquixcc::DeclNode *node, const std::string &prefix, ExportLangType lang)
{
    switch (lang)
    {
    case ExportLangType::Default:
        return mangle_quix(node, prefix);
    case ExportLangType::C:
        return mangle_c(node, prefix);
    case ExportLangType::CXX:
        return mangle_cxx(node, prefix);
    case ExportLangType::DLang:
        throw std::runtime_error("DLang export not yet supported");
    default:
        throw std::runtime_error("Invalid export language type");
    }
}

std::shared_ptr<libquixcc::DeclNode> libquixcc::Symbol::demangle(const std::string &mangled)
{
    std::string input;

    if (mangled.starts_with(quix_abiprefix))
        return demangle_quix(mangled);
    else if (mangled.starts_with(cxx_abiprefix))
        return demangle_cxx(mangled);
    else if (mangled.starts_with(c_abiprefix))
        return demangle_c(mangled);
    else
        return nullptr;
}

bool libquixcc::Symbol::demangle_tojson(const std::string &mangled, std::string &output)
{
    auto node = demangle(mangled);
    if (node == nullptr)
        return false;

    output = node->to_json();
    return true;
}

LIB_EXPORT char *quixcc_demangle_symbol(const char *mangled)
{
    std::string output;
    if (!libquixcc::Symbol::demangle_tojson(mangled, output))
        return nullptr;

    return strdup(output.c_str());
}