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

#include <parse/nodes/AllNodes.h>

std::unordered_map<std::string, std::shared_ptr<libquixcc::StringLiteralNode>> libquixcc::StringLiteralNode::m_instances;
std::unordered_map<std::string, std::shared_ptr<libquixcc::CharLiteralNode>> libquixcc::CharLiteralNode::m_instances;
std::shared_ptr<libquixcc::BoolLiteralNode> libquixcc::BoolLiteralNode::m_true_instance;
std::shared_ptr<libquixcc::BoolLiteralNode> libquixcc::BoolLiteralNode::m_false_instance;
std::unordered_map<std::string, std::shared_ptr<libquixcc::FloatLiteralNode>> libquixcc::FloatLiteralNode::m_instances;
std::unordered_map<std::string, std::shared_ptr<libquixcc::IntegerLiteralNode>> libquixcc::IntegerLiteralNode::m_instances;
std::shared_ptr<libquixcc::NullLiteralNode> libquixcc::NullLiteralNode::m_instance;

uint8_t get_numbits(std::string s);

libquixcc::IntegerLiteralNode::IntegerLiteralNode(const std::string &val)
{
    ntype = NodeType::IntegerLiteralNode;
    m_val = val;

    if (val.starts_with("-"))
    {
        m_val_type = I64TypeNode::create();
        m_value = (int64_t)std::stoll(val);
    }
    else
    {
        m_val_type = U64TypeNode::create();
        m_value = (uint64_t)std::stoull(val);
    }
}

libquixcc::FloatLiteralNode::FloatLiteralNode(const std::string &val)
{
    ntype = NodeType::FloatLiteralNode;
    m_val = val;
    m_value = std::stod(val);

    uint8_t numbits = get_numbits(val);

    if (numbits == 32)
        m_val_type = F32TypeNode::create();
    else if (numbits == 64)
        m_val_type = F64TypeNode::create();
}

libquixcc::TypeNode *libquixcc::StringLiteralNode::infer(libquixcc::TIState &state) const
{
    return StringTypeNode::create();
}

libquixcc::TypeNode *libquixcc::CharLiteralNode::infer(libquixcc::TIState &state) const
{
    return I8TypeNode::create();
}

libquixcc::TypeNode *libquixcc::BoolLiteralNode::infer(libquixcc::TIState &state) const
{
    return BoolTypeNode::create();
}

libquixcc::TypeNode *libquixcc::NullLiteralNode::infer(libquixcc::TIState &state) const
{
    return PointerTypeNode::create(VoidTypeNode::create());
}