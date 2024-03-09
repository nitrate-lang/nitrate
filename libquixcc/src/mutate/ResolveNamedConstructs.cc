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
#include <error/Message.h>
#include <mutex>
#include <set>
#include <quixcc.h>
#include <iostream>
#include <algorithm>

using namespace libquixcc;

void libquixcc::mutate::ResolveNamedConstructs(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(ParseNodePreorderVisitor(
        [job](std::string _namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            if ((*node)->ntype != NodeType::UserTypeNode)
                return;

            libquixcc::UserTypeNode **user_type_ptr = reinterpret_cast<libquixcc::UserTypeNode **>(node);
            libquixcc::UserTypeNode *user_type = *user_type_ptr;
            std::string name = user_type->m_name;

            if (!job->m_inner->m_named_types.contains(user_type->m_name))
            {
                semanticmsg(*job, Err::ERROR, false, feedback[UNRESOLVED_TYPE], user_type->m_name);
                return;
            }

            auto named_type = job->m_inner->m_named_types[user_type->m_name];
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
            default:
                throw std::runtime_error("Unimplemented typeid in ResolveNamedConstructs");
            }

            *user_type_ptr = reinterpret_cast<libquixcc::UserTypeNode *>(type);

            semanticmsg(*job, Err::DEBUG, false, feedback[RESOLVED_TYPE], name.c_str(), type->to_json(ParseNodeJsonSerializerVisitor()).c_str());
        },
        job->m_inner->prefix));
}