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

#include <IR/IRGeneric.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

enum class IR
{
    Alpha,
    Beta,
    Gamma,
    Delta,
};

enum class NodeType
{
    Generic,
    Subsystem,
};

using namespace libquixcc::ir;

class IRAlpha : public libquixcc::ir::IRGeneric<IR::Alpha, NodeType::Generic>
{
protected:
    Result<bool> print_text_impl(std::ostream &os, bool debug) const override
    {
        os << "IRAlpha";
        return true;
    }

    Result<bool> print_bitcode_impl(std::ostream &os) const override
    {
        os << "IRAlpha";
        return true;
    }

    Result<bool> deserialize_text_impl(std::istream &is) override
    {
        return true;
    }

    Result<bool> deserialize_bitcode_impl(std::istream &is) override
    {
        return true;
    }

    size_t node_count_impl() const override
    {
        return 0;
    }

    boost::uuids::uuid graph_hash_impl() const override
    {
        return boost::uuids::nil_uuid();
    }

    std::string_view graph_hash_algorithm_name_impl() const override
    {
        return "node-recurse-sha1-trunc96";
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
        return true;
    };

public:
    IRAlpha() : IRGeneric<IR::Alpha, NodeType::Generic>()
    {
    }

    ~IRAlpha() = default;
};

class SubsystemNode : public libquixcc::ir::Value<NodeType::Subsystem>
{
public:
    std::string m_name;
};

void x()
{
    std::unique_ptr<libquixcc::ir::IRGeneric<IR::Alpha, NodeType::Generic>> ir(new IRAlpha());

    if (!ir->print<PrintMode::Bitcode>(std::cout))
        throw std::runtime_error("Failed to print IR");

    auto root = QIR_ROOT<SubsystemNode>(ir);

    for (auto it = root->begin(); it != root->end(); ++it)
    {
        auto &node = *it;

        if (!node.is<SubsystemNode>())
            continue;

        auto sub = qir_cast<SubsystemNode>(&node);
    }
}