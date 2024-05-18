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

#define QUIXCC_INTERNAL

#include <mutate/Routine.h>
#include <core/Logger.h>
#include <mutex>
#include <set>
#include <quixcc.h>
#include <iostream>
#include <algorithm>

using namespace libquixcc;

void General(const std::vector<std::string> &_namespace, libquixcc::ParseNode *parent, libquixcc::traversal::TraversePtr node)
{
    if (node.first != traversal::TraversePtrType::Smart)
        return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if (ptr->isof<ConstExprNode>())
    {
        auto const_expr = std::static_pointer_cast<ConstExprNode>(ptr)->promote<ConstExprNode>();
        *std::get<std::shared_ptr<ParseNode> *>(node.second) = const_expr;
    }
    else if (ptr->isof<ExprNode>())
    {
        auto expr = std::static_pointer_cast<ExprNode>(ptr)->promote<ExprNode>();
        *std::get<std::shared_ptr<ParseNode> *>(node.second) = expr;
    }
    else if (ptr->is<LetDeclNode>())
    {
        auto let_decl = std::static_pointer_cast<LetDeclNode>(ptr);
        if (!let_decl->m_type)
            return; // Inference will be done later

        if (!let_decl->m_init)
            return;

        auto expr = std::static_pointer_cast<ExprNode>(let_decl->m_init)->promote<ExprNode>();
        if (expr->is_same(let_decl->m_type))
            return; // No need to convert

        if (!let_decl->m_type->is_primitive())
            return; // We can't convert this

        // Semantic analysis will catch this
        let_decl->m_init = std::make_shared<StaticCastExprNode>(expr, let_decl->m_type);
    }
    else if (ptr->is<ConstDeclNode>())
    {
        auto const_decl = std::static_pointer_cast<ConstDeclNode>(ptr);
        if (!const_decl->m_type)
            return; // Inference will be done later

        if (!const_decl->m_init)
            return;

        auto expr = std::static_pointer_cast<ExprNode>(const_decl->m_init)->promote<ExprNode>();
        if (expr->is_same(const_decl->m_type))
            return; // No need to convert

        if (!const_decl->m_type->is_primitive())
            return; // We can't convert this

        // Semantic analysis will catch this
        const_decl->m_init = std::make_shared<StaticCastExprNode>(expr, const_decl->m_type);
    }
    else if (ptr->is<VarDeclNode>())
    {
        auto var_decl = std::static_pointer_cast<VarDeclNode>(ptr);
        if (!var_decl->m_type)
            return; // Inference will be done later

        if (!var_decl->m_init)
            return;

        auto expr = std::static_pointer_cast<ExprNode>(var_decl->m_init)->promote<ExprNode>();
        if (expr->is_same(var_decl->m_type))
            return; // No need to convert

        if (!var_decl->m_type->is_primitive())
            return; // We can't convert this

        // Semantic analysis will catch this
        var_decl->m_init = std::make_shared<StaticCastExprNode>(expr, var_decl->m_type);
    }
    else if (ptr->is<CallExprNode>())
    {
        /// TODO: Implement this
        // auto call_expr = std::static_pointer_cast<CallExprNode>(ptr);

        // for (auto &arg : call_expr->m_positional_args)
        // {
        // }

        // for (auto &arg : call_expr->m_named_args)
        // {
        // }
    }
}

void libquixcc::mutate::ConvertTypes(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(traversal::ASTTraversalState(General, {}));
    ast->dfs_preorder(traversal::ASTTraversalState([&](const std::vector<std::string> &_namespace, libquixcc::ParseNode *parent, traversal::TraversePtr node)
                                                   {
    if (node.first != traversal::TraversePtrType::Smart)
        return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if (ptr->isof<ConstExprNode>())
    {
        return;
    }
    else if (ptr->isof<ExprNode>())
    {
        return;
    }
    else if (ptr->is<LetDeclNode>())
    {
        return;
    }
    else if (ptr->is<ConstDeclNode>())
    {
        auto const_decl = std::static_pointer_cast<ConstDeclNode>(ptr);
        if (const_decl->m_type)
            return;

        if (!const_decl->m_init)
            return;

        auto expr = std::static_pointer_cast<ExprNode>(const_decl->m_init)->promote<ExprNode>();

        TIState state(ast);
        auto type = expr->infer(state);
        if (!type)
            LOG(ERROR) << "Failed to infer type for constant declaration" << std::endl;
        
        const_decl->m_type = type;
    }
    else if (ptr->is<VarDeclNode>())
    {
        return;
    } }, {}));
}