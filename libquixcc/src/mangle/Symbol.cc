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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
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

    output = node->to_json(ParseNodeJsonSerializerVisitor());
    return true;
}

LIB_EXPORT char *quixcc_demangle_symbol(const char *mangled)
{
    std::string output;
    if (!libquixcc::Symbol::demangle_tojson(mangled, output))
        return nullptr;

    return strdup(output.c_str());
}