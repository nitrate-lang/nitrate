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

#include <parse/nodes/ControlFlow.h>
#include <parse/nodes/ExprNode.h>

std::unique_ptr<libquixcc::StmtNode> libquixcc::ReturnStmtNode::reduce(libquixcc::ReductionState &state) const
{
    if (!m_expr)
        return std::make_unique<ReturnStmtNode>(nullptr);
    
    std::shared_ptr<ExprNode> reduced_expr = m_expr->reduce<ExprNode>(state);
    if (!reduced_expr)
        LOG(FATAL) << "Failed to reduce return expression" << std::endl;

    return std::make_unique<ReturnStmtNode>(reduced_expr);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::RetifStmtNode::reduce(libquixcc::ReductionState &state) const
{
    std::shared_ptr<ExprNode> reduced_cond = m_cond->reduce<ExprNode>(state);
    std::shared_ptr<ExprNode> reduced_return = m_return->reduce<ExprNode>(state);

    if (!reduced_cond || !reduced_return)
        LOG(FATAL) << "Failed to reduce retif statement" << std::endl;

    auto stmt = std::make_shared<ReturnStmtNode>(reduced_return);
    return std::make_unique<IfStmtNode>(reduced_cond, stmt, nullptr);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::RetzStmtNode::reduce(libquixcc::ReductionState &state) const
{
    auto expr_reduced = m_cond->reduce<ExprNode>(state);
    auto stmt_reduced = m_return->reduce<ExprNode>(state);
    if (!expr_reduced || !stmt_reduced)
        LOG(FATAL) << "Failed to reduce retz statement" << std::endl;

    auto inv_expr = std::make_shared<UnaryExprNode>(Operator::LogicalNot, expr_reduced);
    auto stmt = std::make_shared<ReturnStmtNode>(stmt_reduced);
    return std::make_unique<IfStmtNode>(inv_expr, stmt, nullptr);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::RetvStmtNode::reduce(libquixcc::ReductionState &state) const
{
    auto expr_reduced = m_cond->reduce<ExprNode>(state);
    auto stmt_reduced = std::make_shared<ReturnStmtNode>(nullptr);
    if (!expr_reduced)
        LOG(FATAL) << "Failed to reduce retv statement" << std::endl;

    return std::make_unique<IfStmtNode>(expr_reduced, stmt_reduced, nullptr);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::IfStmtNode::reduce(libquixcc::ReductionState &state) const
{
    std::shared_ptr<ExprNode> reduced_cond = m_cond->reduce<ExprNode>(state);
    std::shared_ptr<StmtNode> reduced_then = m_then->reduce(state);

    if (!reduced_cond || !reduced_then)
        LOG(FATAL) << "Failed to reduce if statement" << std::endl;

    std::shared_ptr<StmtNode> reduced_else = nullptr;
    if (m_else)
    {
        reduced_else = m_else->reduce(state);
        if (!reduced_else)
            LOG(FATAL) << "Failed to reduce else statement" << std::endl;
    }

    return std::make_unique<IfStmtNode>(reduced_cond, reduced_then, reduced_else);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::WhileStmtNode::reduce(libquixcc::ReductionState &state) const
{
    std::shared_ptr<ExprNode> reduced_cond = m_cond->reduce<ExprNode>(state);
    std::shared_ptr<StmtNode> reduced_body = nullptr;

    if (!reduced_cond)
        LOG(FATAL) << "Failed to reduce while condition" << std::endl;

    if (m_stmt)
    {
        reduced_body = m_stmt->reduce(state);
        if (!reduced_body)
            LOG(FATAL) << "Failed to reduce while body" << std::endl;
    }

    return std::make_unique<WhileStmtNode>(reduced_cond, reduced_body);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::ForStmtNode::reduce(libquixcc::ReductionState &state) const
{
    std::shared_ptr<BlockNode> body = nullptr;

    auto block_node = std::static_pointer_cast<BlockNode>(m_stmt);
    for (auto &stmt : block_node->m_stmts)
    {
        std::shared_ptr<StmtNode> reduced_stmt = stmt->reduce(state);
        if (!reduced_stmt)
            LOG(FATAL) << "Failed to reduce for loop statement" << std::endl;
        body->m_stmts.push_back(reduced_stmt);
    }

    if (m_step)
        body->m_stmts.push_back(std::make_shared<ExprStmtNode>(m_step));

    std::unique_ptr<BlockNode> group = std::make_unique<BlockNode>();

    auto while_loop = std::make_shared<WhileStmtNode>(m_cond, body);

    if (m_init)
        group->m_stmts.push_back(std::make_shared<ExprStmtNode>(m_init));

    group->m_stmts.push_back(while_loop);

    return group;
}