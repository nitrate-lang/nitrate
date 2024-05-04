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

#ifndef __QUIXCC_IR_BETAIR_H__
#define __QUIXCC_IR_BETAIR_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/alpha/AlphaIR.h>
#include <IR/IRModule.h>
#include <IR/Type.h>

namespace libquixcc
{
    namespace ir
    {
        namespace beta
        {
            enum class NodeType
            {
                Generic,
                Group,
                Node,
            };

            class IRBeta : public libquixcc::ir::IRModule<IR::Beta, NodeType::Group>
            {
            protected:
                Result<bool> print_impl(std::ostream &os, bool debug) const override
                {
                    if (!m_root)
                    {
                        os << "IRBeta_1_0(" + m_name + ")";
                        return true;
                    }

                    os << "IRBeta_1_0(" + m_name + ",[";

                    Result<bool> result;
                    if (debug)
                        result = m_root->print<PrintMode::Debug>(os);
                    else
                        result = m_root->print<PrintMode::Text>(os);

                    os << "])";

                    return result;
                }

                Result<bool> deserialize_impl(std::istream &is) override
                {
                    throw std::runtime_error("Not implemented");
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
                IRBeta(const std::string_view &name) : IRModule<IR::Beta, NodeType::Group>(name) {}
                ~IRBeta() = default;

                bool from_alpha(const std::unique_ptr<libquixcc::ir::alpha::IRAlpha> &alpha);
            };
        }
    }
}

#endif // __QUIXCC_IR_BETAIR_H__