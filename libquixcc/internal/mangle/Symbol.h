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

#ifndef __QUIXCC_MANGLE_SYMBOL_H__
#define __QUIXCC_MANGLE_SYMBOL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <memory>
#include <string>
#include <parse/nodes/BasicNodes.h>

namespace libquixcc
{
    class Symbol
    {
        Symbol() = delete;

        static std::string mangle_quix(const DeclNode *node, const std::string &prefix);
        static std::shared_ptr<DeclNode> demangle_quix(std::string input);

        static std::string mangle_cxx(const DeclNode *node, const std::string &prefix);
        static std::shared_ptr<DeclNode> demangle_cxx(std::string input);

        static std::string mangle_c(const DeclNode *node, const std::string &prefix);
        static std::shared_ptr<DeclNode> demangle_c(std::string input);

        const static std::string quix_abiprefix;
        const static std::string cxx_abiprefix;
        const static std::string c_abiprefix;

    public:
        static std::string mangle(const DeclNode *node, const std::string &prefix, ExportLangType lang);

        static std::shared_ptr<DeclNode> demangle(const std::string &mangled);
        static bool demangle_tojson(const std::string &mangled, std::string &output);
    };
}

#endif // __QUIXCC_MANGLE_SYMBOL_H__