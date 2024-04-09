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

#include <mutate/Routine.h>
#include <error/Logger.h>
#include <mutex>
#include <set>
#include <quixcc.h>
#include <iostream>
#include <algorithm>

using namespace libquixcc;

static std::string ConstructName(const std::string &prefix, const std::string &name)
{
    if (prefix.empty())
        return name;
    else
        return prefix + "::" + name;
}

static std::map<libquixcc::NodeType, libquixcc::Msg> error_message_index = {
    {NodeType::VarDeclNode, VAR_NAME_DUPLICATE},
    {NodeType::LetDeclNode, LET_NAME_DUPLICATE},
    {NodeType::FunctionDeclNode, FUNC_NAME_DUPLICATE},
    {NodeType::StructDefNode, STRUCT_NAME_DUPLICATE},
    {NodeType::StructFieldNode, STRUCT_FIELD_DUPLICATE},
    {NodeType::UnionDefNode, UNION_NAME_DUPLICATE},
    {NodeType::UnionFieldNode, UNION_FIELD_DUPLICATE},
    {NodeType::EnumDefNode, ENUM_NAME_DUPLICATE},
    {NodeType::EnumFieldNode, ENUM_FIELD_DUPLICATE},
    {NodeType::FunctionParamNode, PARAM_NAME_DUPLICATE},
    {NodeType::SubsystemNode, SUBSYSTEM_NAME_DUPLICATE},
    {NodeType::TypedefNode, TYPEDEF_NAME_DUPLICATE},
};

void libquixcc::mutate::DiscoverNamedConstructs(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    std::map<std::pair<NodeType, std::string>, std::shared_ptr<libquixcc::ParseNode>> named_construct_map;
    std::map<std::string, std::shared_ptr<libquixcc::ParseNode>> named_types_map;

    ast->dfs_preorder(ParseNodePreorderVisitor(
        [&named_construct_map, job, &named_types_map](std::string _namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            std::string tmp;
            bool is_type = false;

            switch ((*node)->ntype)
            {
            case NodeType::VarDeclNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::VarDeclNode>(*node)->m_name);
                break;
            case NodeType::LetDeclNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::LetDeclNode>(*node)->m_name);
                break;
            case NodeType::FunctionDeclNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::FunctionDeclNode>(*node)->m_name);
                break;
            case NodeType::StructDefNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::StructDefNode>(*node)->m_name);
                is_type = true;
                break;
            case NodeType::StructFieldNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::StructFieldNode>(*node)->m_name);
                break;
            case NodeType::UnionDefNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::UnionDefNode>(*node)->m_name);
                is_type = true;
                break;
            case NodeType::UnionFieldNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::UnionFieldNode>(*node)->m_name);
                break;
            case NodeType::EnumDefNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::EnumDefNode>(*node)->m_type->m_name);
                is_type = true;
                break;
            case NodeType::EnumFieldNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::EnumFieldNode>(*node)->m_name);
                break;
            case NodeType::FunctionParamNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::FunctionParamNode>(*node)->m_name);
                break;
            case NodeType::SubsystemNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::SubsystemNode>(*node)->m_name);
                break;
            case NodeType::TypedefNode:
                tmp = ConstructName(_namespace, std::static_pointer_cast<libquixcc::TypedefNode>(*node)->m_name);
                is_type = true;
                break;

            default:
                return;
            }

            auto key = std::make_pair((*node)->ntype, tmp);
            if (named_construct_map.contains(key))
            {
                LOG(ERROR) << feedback[error_message_index[(*node)->ntype]] << tmp << std::endl;
                return;
            }

            named_construct_map[key] = *node;

            if (is_type)
                named_types_map[tmp] = *node;
        },
        job->m_inner.prefix));

    for (auto &pair : named_construct_map)
        LOG(DEBUG) << "Found named construct: " << pair.first.second << std::endl;

    job->m_inner.m_named_construsts = named_construct_map;
    job->m_inner.m_named_types = named_types_map;
}