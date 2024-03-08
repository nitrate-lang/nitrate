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

#include <mutate/FoldConstExpr.h>
#include <stack>
#include <queue>

#include <iostream>

using namespace libquixcc;

static void fold_const_string_expr(libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
{
    if ((*node)->ntype != NodeType::ConstBinaryExprNode)
        return;

    auto bin_expr = std::static_pointer_cast<libquixcc::ConstBinaryExprNode>(*node);
    std::string concat;

    std::queue<std::shared_ptr<libquixcc::ConstExprNode>> stack;
    stack.push(bin_expr->m_lhs);
    stack.push(bin_expr->m_rhs);

    while (!stack.empty())
    {
        auto node = stack.front();
        stack.pop();

        if (node->ntype == NodeType::ConstBinaryExprNode)
        {
            auto bin_node = std::static_pointer_cast<libquixcc::ConstBinaryExprNode>(node);
            if (bin_node->m_op != Operator::Plus)
                return;

            stack.push(bin_node->m_lhs);
            stack.push(bin_node->m_rhs);
        }
        else if (node->ntype == NodeType::StringLiteralNode)
        {
            auto str_node = std::static_pointer_cast<libquixcc::StringLiteralNode>(node);
            concat += str_node->m_val;
        }
        else
        {
            return; // Not a string literal
        }
    }

    parent->replace_child(*node, libquixcc::StringLiteralNode::create(concat));
}

void libquixcc::mutate::FoldConstExpr(quixcc_job_t *job, std::shared_ptr<libquixcc::AST> ast)
{
    ast->dfs_preorder(ParseNodePreorderVisitor(fold_const_string_expr));
}