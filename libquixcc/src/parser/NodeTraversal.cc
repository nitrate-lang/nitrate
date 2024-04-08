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

#include <parse/NodeTraversal.h>
#include <parse/nodes/AllNodes.h>

void libquixcc::ParseNodePreorderVisitor::push_prefix(const std::string &name)
{
    if (m_prefix.empty())
        m_prefix = name;
    else
        m_prefix += "::" + name;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ParseNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ASTNopNode *node)
{
    return 0;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ExprNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ConstExprNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StmtNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ExprStmtNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_expr));
    return node->m_expr->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::NopStmtNode *node)
{
    return 0;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::DeclNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::DefNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::BlockNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&stmt));
        count += stmt->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnaryExprNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_expr));
    return node->m_expr->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::BinaryExprNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_lhs));
    size_t count = node->m_lhs->dfs_preorder(*this);
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_rhs));
    count += node->m_rhs->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::CallExprNode *node)
{
    size_t count = 0;
    for (auto &arg : node->m_positional_args)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&arg));
        count += arg->dfs_preorder(*this);
    }
    for (auto &arg : node->m_named_args)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&arg.second));
        count += arg.second->dfs_preorder(*this);
    }

    if (node->m_decl)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_decl));
        count += node->m_decl->dfs_preorder(*this);
    }

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ConstUnaryExprNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_expr));
    return node->m_expr->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ConstBinaryExprNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_lhs));
    size_t count = node->m_lhs->dfs_preorder(*this);
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_rhs));
    count += node->m_rhs->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::IdentifierNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::U8TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::U16TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::U32TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::U64TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::I8TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::I16TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::I32TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::I64TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::F32TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::F64TypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::BoolTypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::VoidTypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::PointerTypeNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    return node->m_type->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StringTypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::EnumTypeNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_member_type));
    return node->m_member_type->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StructTypeNode *node)
{
    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&field));
        count += field->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnionTypeNode *node)
{
    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&field));
        count += field->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ArrayTypeNode *node)
{
    size_t count = 0;
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    count += node->m_type->dfs_preorder(*this);
    if (node->m_size)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_size));
        count += node->m_size->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionTypeNode *node)
{
    size_t count = 0;
    for (auto &param : node->m_params)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&param));
        count += param->dfs_preorder(*this);
    }
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_return_type));
    count += node->m_return_type->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UserTypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::LiteralNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::IntegerLiteralNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FloatLiteralNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StringLiteralNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::CharLiteralNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::BoolLiteralNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::NullLiteralNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::TypedefNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_orig));
    return node->m_orig->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::VarDeclNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_init)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_init));
        count += node->m_init->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::LetDeclNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_init)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_init));
        count += node->m_init->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StructDeclNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnionDeclNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::EnumDeclNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    return node->m_type->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionDeclNode *node)
{
    size_t count = 0;
    std::string old_prefix = m_prefix;
    push_prefix(node->m_name);

    for (auto &param : node->m_params)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&param));
        count += param->dfs_preorder(*this);
    }
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    count += node->m_type->dfs_preorder(*this);

    m_prefix = old_prefix;
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StructDefNode *node)
{
    size_t count = 0;
    std::string old_prefix = m_prefix;
    push_prefix(node->m_name);

    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&field));
        count += field->dfs_preorder(*this);
    }

    m_prefix = old_prefix;
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StructFieldNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnionDefNode *node)
{
    size_t count = 0;
    std::string old_prefix = m_prefix;
    push_prefix(node->m_name);

    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&field));
        count += field->dfs_preorder(*this);
    }

    m_prefix = old_prefix;
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnionFieldNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::EnumDefNode *node)
{
    std::string old_prefix = m_prefix;
    if (node->m_scoped)
        push_prefix(node->m_decl->m_type->m_name);

    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_decl));
    size_t count = node->m_decl->dfs_preorder(*this);
    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&field));
        count += field->dfs_preorder(*this);
    }

    m_prefix = old_prefix;

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::EnumFieldNode *node)
{
    if (node->m_value)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_value));
        return node->m_value->dfs_preorder(*this) + 1;
    }
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionDefNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_decl));
    size_t count = node->m_decl->dfs_preorder(*this);
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_body));
    count += node->m_body->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionParamNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::SubsystemNode *node)
{
    std::string old_prefix = m_prefix;

    push_prefix(node->m_name);

    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_block));
    size_t count = node->m_block->dfs_preorder(*this);

    m_prefix = old_prefix;

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ExportNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&stmt));
        count += stmt->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ReturnStmtNode *node)
{
    if (node->m_expr)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_expr));
        return node->m_expr->dfs_preorder(*this) + 1;
    }
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RetifStmtNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_return));
    count += node->m_return->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RetzStmtNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_return));
    count += node->m_return->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RetvStmtNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_cond));
    return node->m_cond->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::IfStmtNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_then));
    count += node->m_then->dfs_preorder(*this);
    if (node->m_else)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_else));
        count += node->m_else->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::WhileStmtNode *node)
{
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_stmt));
    count += node->m_stmt->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ForStmtNode *node)
{
    size_t count = 1;

    if (node->m_init)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_init));
        count += node->m_init->dfs_preorder(*this);
    }

    if (node->m_cond)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_cond));
        count += node->m_cond->dfs_preorder(*this);
    }

    if (node->m_step)
    {
        m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_step));
        count += node->m_step->dfs_preorder(*this);
    }

    m_callback(m_prefix, node, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(&node->m_stmt));
    count += node->m_stmt->dfs_preorder(*this);

    return count;
}