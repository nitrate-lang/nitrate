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

std::map<libquixcc::TypeNode *, libquixcc::MutTypeNode *> libquixcc::MutTypeNode::m_instances;
libquixcc::U8TypeNode *libquixcc::U8TypeNode::m_instance = nullptr;
libquixcc::U16TypeNode *libquixcc::U16TypeNode::m_instance = nullptr;
libquixcc::U32TypeNode *libquixcc::U32TypeNode::m_instance = nullptr;
libquixcc::U64TypeNode *libquixcc::U64TypeNode::m_instance = nullptr;
libquixcc::I8TypeNode *libquixcc::I8TypeNode::m_instance = nullptr;
libquixcc::I16TypeNode *libquixcc::I16TypeNode::m_instance = nullptr;
libquixcc::I32TypeNode *libquixcc::I32TypeNode::m_instance = nullptr;
libquixcc::I64TypeNode *libquixcc::I64TypeNode::m_instance = nullptr;
libquixcc::F32TypeNode *libquixcc::F32TypeNode::m_instance = nullptr;
libquixcc::F64TypeNode *libquixcc::F64TypeNode::m_instance = nullptr;
libquixcc::BoolTypeNode *libquixcc::BoolTypeNode::m_instance = nullptr;
libquixcc::VoidTypeNode *libquixcc::VoidTypeNode::m_instance = nullptr;
std::map<libquixcc::TypeNode *, libquixcc::PointerTypeNode *> libquixcc::PointerTypeNode::m_instances;
std::map<std::string, libquixcc::OpaqueTypeNode *> libquixcc::OpaqueTypeNode::m_instances;
libquixcc::StringTypeNode *libquixcc::StringTypeNode::m_instance = nullptr;
std::map<std::pair<std::vector<libquixcc::TypeNode *>, std::string>, libquixcc::StructTypeNode *> libquixcc::StructTypeNode::m_instances;
std::map<std::pair<std::vector<libquixcc::TypeNode *>, std::string>, libquixcc::RegionTypeNode *> libquixcc::RegionTypeNode::m_instances;
std::map<std::pair<std::vector<libquixcc::TypeNode *>, std::string>, libquixcc::UnionTypeNode *> libquixcc::UnionTypeNode::m_instances;
thread_local std::map<std::pair<libquixcc::TypeNode *, std::shared_ptr<libquixcc::ConstExprNode>>, libquixcc::ArrayTypeNode *> libquixcc::ArrayTypeNode::m_instances;
thread_local std::unordered_map<std::string, std::shared_ptr<libquixcc::UserTypeNode>> libquixcc::UserTypeNode::m_instances;
std::map<libquixcc::FunctionTypeNode::Inner, libquixcc::FunctionTypeNode *> libquixcc::FunctionTypeNode::s_instances;
std::map<std::pair<std::string, libquixcc::TypeNode *>, libquixcc::EnumTypeNode *> libquixcc::EnumTypeNode::m_instances;

bool libquixcc::TypeNode::is_composite() const
{
    return is<StructTypeNode>() || is<RegionTypeNode>() || is<UnionTypeNode>();
}

bool libquixcc::TypeNode::is_primitive() const
{
    return is_integer() || is_floating() || is_void();
}

bool libquixcc::TypeNode::is_ptr() const
{
    return is<PointerTypeNode>();
}

bool libquixcc::TypeNode::is_array() const
{
    return is<ArrayTypeNode>();
}

bool libquixcc::TypeNode::is_func() const
{
    return is<FunctionTypeNode>();
}

bool libquixcc::TypeNode::is_void() const
{
    return is<VoidTypeNode>();
}

bool libquixcc::TypeNode::is_signed() const
{
    switch (ntype)
    {
    case NodeType::I8TypeNode:
    case NodeType::I16TypeNode:
    case NodeType::I32TypeNode:
    case NodeType::I64TypeNode:
        return true;
    default:
        return false;
    }
}

bool libquixcc::TypeNode::is_integer() const
{
    switch (ntype)
    {
    case NodeType::U8TypeNode:
    case NodeType::U16TypeNode:
    case NodeType::U32TypeNode:
    case NodeType::U64TypeNode:
    case NodeType::I8TypeNode:
    case NodeType::I16TypeNode:
    case NodeType::I32TypeNode:
    case NodeType::I64TypeNode:
    case NodeType::BoolTypeNode:
        return true;
    default:
        return false;
    }
}

bool libquixcc::TypeNode::is_floating() const
{
    return is<F32TypeNode>() || is<F64TypeNode>();
}

bool libquixcc::TypeNode::is_bool() const
{
    return is<BoolTypeNode>();
}

std::vector<std::shared_ptr<libquixcc::GroupFieldNode>> libquixcc::GroupDefNode::optimize_layout(const std::vector<std::shared_ptr<libquixcc::GroupFieldNode>> &fields)
{
    /*
     * A heuristic for optimizing the layout of a group's fields is to sort them in descending order of size.
     * Downsides:
     * - Does not take custom padding into account
     * - Does not take alignment into account
     * - Does not take bitfields into account
     */
    std::vector<std::shared_ptr<libquixcc::GroupFieldNode>> copy = fields;
    size_t ptr_size = sizeof(void *);

    std::sort(copy.begin(), copy.end(), [ptr_size](const std::shared_ptr<GroupFieldNode> &a, const std::shared_ptr<GroupFieldNode> &b)
              { return a->m_type->size(ptr_size) > b->m_type->size(ptr_size); });

    return copy;
}

std::shared_ptr<libquixcc::StructDefNode> libquixcc::GroupDefNode::to_struct_def() const
{
    std::vector<std::shared_ptr<libquixcc::StructFieldNode>> fields;
    for (const auto &field : optimize_layout(m_fields))
    {
        fields.push_back(std::make_shared<libquixcc::StructFieldNode>(field->m_name, field->m_type, field->m_value));
    }

    return std::make_shared<libquixcc::StructDefNode>(m_name, fields);
}

std::shared_ptr<libquixcc::StructDefNode> libquixcc::RegionDefNode::to_struct_def() const
{
    std::vector<std::shared_ptr<libquixcc::StructFieldNode>> fields;
    for (const auto &field : m_fields)
    {
        fields.push_back(std::make_shared<libquixcc::StructFieldNode>(field->m_name, field->m_type, field->m_value));
    }

    return std::make_shared<libquixcc::StructDefNode>(m_name, fields);
}
