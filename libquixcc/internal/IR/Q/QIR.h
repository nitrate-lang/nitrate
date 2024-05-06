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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
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

#ifndef __QUIXCC_IR_QIR_H__
#define __QUIXCC_IR_QIR_H__

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
        namespace q
        {
            enum class NodeType
            {
                Generic,
                Group,
                Node,
            };

            class RootNode : public libquixcc::ir::Value<Q>
            {
            protected:
                Result<bool> print_impl(std::ostream &os, PState &state) const override;
                boost::uuids::uuid hash_impl() const override;
                bool verify_impl() const override;

            public:
                static const RootNode *create()
                {
                    return nullptr;
                }
            };

            class QModule : public libquixcc::ir::IRModule<IR::Q, RootNode *>
            {
            protected:
                Result<bool> print_impl(std::ostream &os, PState &state) const override
                {
                    if (!m_root)
                    {
                        os << "QIR_1_0(" + m_name + ")";
                        return true;
                    }

                    os << "QIR_1_0(" + m_name + ",[";

                    Result<bool> result = m_root->print(os, state);
                    os << "])";

                    return result;
                }

                std::string_view ir_dialect_name_impl() const override;
                unsigned ir_dialect_version_impl() const override;
                std::string_view ir_dialect_family_impl() const override;
                std::string_view ir_dialect_description_impl() const override;

                bool verify_impl() const override
                {
                    if (!m_root)
                        return false;

                    return m_root->verify();
                };

            public:
                QModule(const std::string_view &name) : IRModule<IR::Q, RootNode *>(name) {}
                ~QModule() = default;

                bool from_ast(const std::shared_ptr<BlockNode> &ast);
            };
        }
    }
}

#endif // __QUIXCC_IR_QIR_H__