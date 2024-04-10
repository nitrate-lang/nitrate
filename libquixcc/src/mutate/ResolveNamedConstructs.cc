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
            case NodeType::GroupDefNode:
                type = std::static_pointer_cast<libquixcc::GroupDefNode>(named_type)->get_type();
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

static void resolve_function_decls_to_calls(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(ParseNodePreorderVisitor(
        [job](std::string _namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            if ((*node)->ntype != NodeType::CallExprNode)
                return;

            auto expr = std::static_pointer_cast<libquixcc::CallExprNode>(*node);

            if (!job->m_inner.m_named_construsts.contains(std::make_pair(NodeType::FunctionDeclNode, expr->m_name)))
            {
                LOG(ERROR) << feedback[UNRESOLVED_FUNCTION] << expr->m_name << std::endl;
                return;
            }

            auto func_decl = std::static_pointer_cast<libquixcc::FunctionDeclNode>(job->m_inner.m_named_construsts[std::make_pair(NodeType::FunctionDeclNode, expr->m_name)]);
            expr->m_decl = func_decl;
        },
        job->m_inner.prefix));
}

void libquixcc::mutate::ResolveNamedConstructs(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    resolve_user_type_nodes(job, ast);
    resolve_enum_values(job, ast);
    resolve_function_decls_to_calls(job, ast);
}