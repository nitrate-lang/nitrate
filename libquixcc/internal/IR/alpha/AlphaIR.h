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

#ifndef __QUIXCC_IR_ALPHAIR_H__
#define __QUIXCC_IR_ALPHAIR_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <parse/nodes/AllNodes.h>
#include <IR/IRModule.h>
#include <IR/Type.h>

namespace libquixcc
{
    namespace ir
    {
        namespace alpha
        {
            enum class NodeType
            {
                Generic,
                Group,
                Node,
            };

            class IRAlpha : public libquixcc::ir::IRModule<IR::Alpha, NodeType::Group>
            {
            protected:
                Result<bool> print_text_impl(std::ostream &os, bool debug) const override
                {
                    if (!m_root)
                    {
                        os << "IRAlpha_1_0(" + m_name + ")";
                        return true;
                    }

                    os << "IRAlpha_1_0(" + m_name + ",[";

                    Result<bool> result;
                    if (debug)
                        result = m_root->print<PrintMode::Debug>(os);
                    else
                        result = m_root->print<PrintMode::Text>(os);

                    os << "])";

                    return result;
                }

                Result<bool> deserialize_text_impl(std::istream &is) override
                {
                    throw std::runtime_error("Not implemented");
                }

                std::string_view ir_dialect_name_impl() const override
                {
                    return "QIR-Alpha";
                }

                unsigned ir_dialect_version_impl() const override
                {
                    return 1;
                }

                std::string_view ir_dialect_family_impl() const override
                {
                    return "QIR";
                }

                std::string_view ir_dialect_description_impl() const override
                {
                    return "Quix Alpha Intermediate Representation (QIR-Alpha-V1.0) is a high-level intermediate representation for the Quix language. It contains high level information such as control flow, lambda expressions, coroutines, heap allocations, and other high-level constructs.";
                }

                bool verify_impl() const override
                {
                    if (!m_root)
                        return false;

                    return m_root->verify();
                };

            public:
                IRAlpha(const std::string_view &name) : IRModule<IR::Alpha, NodeType::Group>(name) {}

                bool from_ast(const std::shared_ptr<BlockNode> &ast);

                ~IRAlpha() = default;
            };
        }
    }
}

#endif // __QUIXCC_IR_ALPHAIR_H__