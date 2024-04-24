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

#include <parse/NodeTraversal.h>
#include <parse/nodes/AllNodes.h>

template <typename T>
libquixcc::TraversePtr mk_ptr(std::shared_ptr<T> *ptr)
{
    return std::make_pair(libquixcc::TraversePtrType::Smart, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(ptr));
}

template <typename T>
libquixcc::TraversePtr mk_ptr(T **ptr)
{
    return std::make_pair(libquixcc::TraversePtrType::Raw, reinterpret_cast<libquixcc::ParseNode **>(ptr));
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ASTNopNode *node)
{
    return 0;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ExprStmtNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_expr));
    return node->m_expr->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::NopStmtNode *node)
{
    return 0;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::BlockNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        m_callback(m_prefix, node, mk_ptr(&stmt));
        count += stmt->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StmtGroupNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        m_callback(m_prefix, node, mk_ptr(&stmt));
        count += stmt->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::CastExprNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    m_callback(m_prefix, node, mk_ptr(&node->m_expr));
    count += node->m_expr->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnaryExprNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_expr));
    return node->m_expr->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::BinaryExprNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_lhs));
    size_t count = node->m_lhs->dfs_preorder(*this);
    m_callback(m_prefix, node, mk_ptr(&node->m_rhs));
    count += node->m_rhs->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::CallExprNode *node)
{
    size_t count = 0;
    for (auto &arg : node->m_positional_args)
    {
        m_callback(m_prefix, node, mk_ptr(&arg));
        count += arg->dfs_preorder(*this);
    }
    for (auto &arg : node->m_named_args)
    {
        m_callback(m_prefix, node, mk_ptr(&arg.second));
        count += arg.second->dfs_preorder(*this);
    }

    if (node->m_decl)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_decl));
        count += node->m_decl->dfs_preorder(*this);
    }

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ListExprNode *node)
{
    size_t count = 0;
    for (auto &elem : node->m_elements)
    {
        m_callback(m_prefix, node, mk_ptr(&elem));
        count += elem->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::MemberAccessNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_expr));
    return node->m_expr->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ConstUnaryExprNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_expr));
    return node->m_expr->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ConstBinaryExprNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_lhs));
    size_t count = node->m_lhs->dfs_preorder(*this);
    m_callback(m_prefix, node, mk_ptr(&node->m_rhs));
    count += node->m_rhs->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::IdentifierNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::MutTypeNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    return node->m_type->dfs_preorder(*this) + 1;
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
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    return node->m_type->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::OpaqueTypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StringTypeNode *node)
{
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::EnumTypeNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_member_type));
    return node->m_member_type->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StructTypeNode *node)
{
    if (m_visited.contains(node))
        return 1;

    m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RegionTypeNode *node)
{
    if (m_visited.contains(node))
        return 1;

    m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnionTypeNode *node)
{
    if (m_visited.contains(node))
        return 1;

    m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ArrayTypeNode *node)
{
    size_t count = 0;
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    count += node->m_type->dfs_preorder(*this);
    if (node->m_size)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_size));
        count += node->m_size->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionTypeNode *node)
{
    size_t count = 0;
    for (auto &param : node->m_params)
    {
        m_callback(m_prefix, node, mk_ptr(&param.second));
        count += param.second->dfs_preorder(*this);
    }
    m_callback(m_prefix, node, mk_ptr(&node->m_return_type));
    count += node->m_return_type->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UserTypeNode *node)
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
    m_callback(m_prefix, node, mk_ptr(&node->m_orig));
    return node->m_orig->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::VarDeclNode *node)
{
    m_visited.clear();

    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_init)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_init));
        count += node->m_init->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::LetDeclNode *node)
{
    m_visited.clear();

    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_init)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_init));
        count += node->m_init->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionDeclNode *node)
{
    m_visited.clear();

    size_t count = 0;
    m_prefix.push_back(node->m_name);

    for (auto &param : node->m_params)
    {
        m_callback(m_prefix, node, mk_ptr(&param));
        count += param->dfs_preorder(*this);
    }
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    count += node->m_type->dfs_preorder(*this);

    m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StructDefNode *node)
{
    size_t count = 0;
    m_prefix.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }

    for (auto &method : node->m_methods)
    {
        m_callback(m_prefix, node, mk_ptr(&method));
        count += method->dfs_preorder(*this);
    }

    for (auto &method : node->m_static_methods)
    {
        m_callback(m_prefix, node, mk_ptr(&method));
        count += method->dfs_preorder(*this);
    }

    m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::StructFieldNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RegionDefNode *node)
{
    size_t count = 0;
    m_prefix.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }

    m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RegionFieldNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::GroupDefNode *node)
{
    size_t count = 0;
    m_prefix.push_back(node->m_name);

    for (auto &field : node->get_fields_mut())
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }

    m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::GroupFieldNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnionDefNode *node)
{
    size_t count = 0;
    m_prefix.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }

    m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::UnionFieldNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::EnumDefNode *node)
{
    if (node->m_scoped)
        m_prefix.push_back(node->m_type->m_name);

    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    for (auto &field : node->m_fields)
    {
        m_callback(m_prefix, node, mk_ptr(&field));
        count += field->dfs_preorder(*this);
    }

    if (node->m_scoped)
        m_prefix.pop_back();

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::EnumFieldNode *node)
{
    if (node->m_value)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_value));
        return node->m_value->dfs_preorder(*this) + 1;
    }
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionDefNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_decl));
    size_t count = node->m_decl->dfs_preorder(*this);

    m_prefix.push_back(node->m_decl->m_name);

    m_callback(m_prefix, node, mk_ptr(&node->m_body));
    count += node->m_body->dfs_preorder(*this);

    m_prefix.pop_back();

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::FunctionParamNode *node)
{
    m_visited.clear();

    m_callback(m_prefix, node, mk_ptr(&node->m_type));
    size_t count = node->m_type->dfs_preorder(*this);
    if (node->m_value)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_value));
        count += node->m_value->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::SubsystemNode *node)
{
    m_prefix.push_back(node->m_name);

    m_callback(m_prefix, node, mk_ptr(&node->m_block));
    size_t count = node->m_block->dfs_preorder(*this);

    m_prefix.pop_back();

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ExportNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        m_callback(m_prefix, node, mk_ptr(&stmt));
        count += stmt->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::InlineAsmNode *node)
{
    size_t count = 0;

    for (auto &[reg, expr] : node->m_outputs)
    {
        m_callback(m_prefix, node, mk_ptr(&expr));
        count += expr->dfs_preorder(*this);
    }

    for (auto &[reg, expr] : node->m_outputs)
    {
        m_callback(m_prefix, node, mk_ptr(&expr));
        count += expr->dfs_preorder(*this);
    }

    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ReturnStmtNode *node)
{
    if (node->m_expr)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_expr));
        return node->m_expr->dfs_preorder(*this) + 1;
    }
    return 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RetifStmtNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, mk_ptr(&node->m_return));
    count += node->m_return->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RetzStmtNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, mk_ptr(&node->m_return));
    count += node->m_return->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::RetvStmtNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_cond));
    return node->m_cond->dfs_preorder(*this) + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::IfStmtNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, mk_ptr(&node->m_then));
    count += node->m_then->dfs_preorder(*this);
    if (node->m_else)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_else));
        count += node->m_else->dfs_preorder(*this);
    }
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::WhileStmtNode *node)
{
    m_callback(m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = node->m_cond->dfs_preorder(*this);
    m_callback(m_prefix, node, mk_ptr(&node->m_stmt));
    count += node->m_stmt->dfs_preorder(*this);
    return count + 1;
}

size_t libquixcc::ParseNodePreorderVisitor::visit(libquixcc::ForStmtNode *node)
{
    size_t count = 1;

    if (node->m_init)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_init));
        count += node->m_init->dfs_preorder(*this);
    }

    if (node->m_cond)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_cond));
        count += node->m_cond->dfs_preorder(*this);
    }

    if (node->m_step)
    {
        m_callback(m_prefix, node, mk_ptr(&node->m_step));
        count += node->m_step->dfs_preorder(*this);
    }

    m_callback(m_prefix, node, mk_ptr(&node->m_stmt));
    count += node->m_stmt->dfs_preorder(*this);

    return count;
}