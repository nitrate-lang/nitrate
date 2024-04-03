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

static void resolve_user_type_nodes(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(ParseNodePreorderVisitor(
        [job](std::string _namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            if ((*node)->ntype != NodeType::UserTypeNode)
                return;

            libquixcc::UserTypeNode **user_type_ptr = reinterpret_cast<libquixcc::UserTypeNode **>(node);
            libquixcc::UserTypeNode *user_type = *user_type_ptr;
            std::string name = user_type->m_name;

            if (!job->m_inner.m_named_types.contains(user_type->m_name))
            {
                LOG(ERROR) << feedback[UNRESOLVED_TYPE] << user_type->m_name << std::endl;
                return;
            }

            auto named_type = job->m_inner.m_named_types[user_type->m_name];
            TypeNode *type = nullptr;

            switch (named_type->ntype)
            {
            case NodeType::StructDefNode:
                type = std::static_pointer_cast<libquixcc::StructDefNode>(named_type)->get_type();
                break;
            case NodeType::UnionDefNode:
                type = std::static_pointer_cast<libquixcc::UnionDefNode>(named_type)->get_type();
                break;
            case NodeType::EnumDefNode:
                type = std::static_pointer_cast<libquixcc::EnumDefNode>(named_type)->get_type();
                break;
            case NodeType::TypedefNode:
                type = std::static_pointer_cast<libquixcc::TypedefNode>(named_type)->m_orig;
                break;
            default:
                throw std::runtime_error("Unimplemented typeid in ResolveNamedConstructs");
            }

            *user_type_ptr = reinterpret_cast<libquixcc::UserTypeNode *>(type);

            LOG(DEBUG) << feedback[RESOLVED_TYPE] << name << type->to_json(ParseNodeJsonSerializerVisitor()) << std::endl;
        },
        job->m_inner.prefix));
}

static void resolve_enum_values(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(ParseNodePreorderVisitor(
        [job](std::string _namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            if ((*node)->ntype != NodeType::IdentifierNode)
                return;

            auto ident = std::static_pointer_cast<libquixcc::IdentifierNode>(*node);

            if (!job->m_inner.m_named_construsts.contains(std::make_pair(NodeType::EnumFieldNode, ident->m_name)))
                return;

            auto enum_field = std::static_pointer_cast<libquixcc::EnumFieldNode>(job->m_inner.m_named_construsts[std::make_pair(NodeType::EnumFieldNode, ident->m_name)]);

            *node = enum_field->m_value;
        },
        job->m_inner.prefix));
}

void libquixcc::mutate::ResolveNamedConstructs(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    resolve_user_type_nodes(job, ast);
    resolve_enum_values(job, ast);
}