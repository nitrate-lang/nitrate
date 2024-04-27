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

#include <traversal/AST.h>
#include <parse/nodes/AllNodes.h>

template <typename T>
libquixcc::traversal::TraversePtr mk_ptr(std::shared_ptr<T> *ptr)
{
    return std::make_pair(libquixcc::traversal::TraversePtrType::Smart, reinterpret_cast<std::shared_ptr<libquixcc::ParseNode> *>(ptr));
}

template <typename T>
libquixcc::traversal::TraversePtr mk_ptr(T **ptr)
{
    return std::make_pair(libquixcc::traversal::TraversePtrType::Raw, reinterpret_cast<libquixcc::ParseNode **>(ptr));
}

size_t libquixcc::ParseNode::dfs_preorder(libquixcc::traversal::ASTTraversalState state)
{
    return traversal::ASTPreorderTraversal::next(state, this);
}

size_t libquixcc::traversal::ASTPreorderTraversal::dispatch(libquixcc::traversal::ASTTraversalState &state, libquixcc::ParseNode *node)
{
    typedef size_t (*Func)(libquixcc::traversal::ASTTraversalState &, libquixcc::ParseNode *);

    static const std::unordered_map<NodeType, Func> node_map =
        {
            {NodeType::ASTNopNode, (Func)ASTNopNode_iter},
            {NodeType::ExprStmtNode, (Func)ExprStmtNode_iter},
            {NodeType::NopStmtNode, (Func)NopStmtNode_iter},
            {NodeType::BlockNode, (Func)BlockNode_iter},
            {NodeType::StmtGroupNode, (Func)StmtGroupNode_iter},
            {NodeType::CastExprNode, (Func)CastExprNode_iter},
            {NodeType::StaticCastExprNode, (Func)StaticCastExprNode_iter},
            {NodeType::BitCastExprNode, (Func)BitCastExprNode_iter},
            {NodeType::SignedUpcastExprNode, (Func)SignedUpcastExprNode_iter},
            {NodeType::UnsignedUpcastExprNode, (Func)UnsignedUpcastExprNode_iter},
            {NodeType::DowncastExprNode, (Func)DowncastExprNode_iter},
            {NodeType::PtrToIntCastExprNode, (Func)PtrToIntCastExprNode_iter},
            {NodeType::IntToPtrCastExprNode, (Func)IntToPtrCastExprNode_iter},
            {NodeType::UnaryExprNode, (Func)UnaryExprNode_iter},
            {NodeType::BinaryExprNode, (Func)BinaryExprNode_iter},
            {NodeType::CallExprNode, (Func)CallExprNode_iter},
            {NodeType::ListExprNode, (Func)ListExprNode_iter},
            {NodeType::MemberAccessNode, (Func)MemberAccessNode_iter},
            {NodeType::ConstUnaryExprNode, (Func)ConstUnaryExprNode_iter},
            {NodeType::ConstBinaryExprNode, (Func)ConstBinaryExprNode_iter},
            {NodeType::IdentifierNode, (Func)IdentifierNode_iter},
            {NodeType::MutTypeNode, (Func)MutTypeNode_iter},
            {NodeType::U8TypeNode, (Func)U8TypeNode_iter},
            {NodeType::U16TypeNode, (Func)U16TypeNode_iter},
            {NodeType::U32TypeNode, (Func)U32TypeNode_iter},
            {NodeType::U64TypeNode, (Func)U64TypeNode_iter},
            {NodeType::I8TypeNode, (Func)I8TypeNode_iter},
            {NodeType::I16TypeNode, (Func)I16TypeNode_iter},
            {NodeType::I32TypeNode, (Func)I32TypeNode_iter},
            {NodeType::I64TypeNode, (Func)I64TypeNode_iter},
            {NodeType::F32TypeNode, (Func)F32TypeNode_iter},
            {NodeType::F64TypeNode, (Func)F64TypeNode_iter},
            {NodeType::BoolTypeNode, (Func)BoolTypeNode_iter},
            {NodeType::VoidTypeNode, (Func)VoidTypeNode_iter},
            {NodeType::PointerTypeNode, (Func)PointerTypeNode_iter},
            {NodeType::OpaqueTypeNode, (Func)OpaqueTypeNode_iter},
            {NodeType::StringTypeNode, (Func)StringTypeNode_iter},
            {NodeType::EnumTypeNode, (Func)EnumTypeNode_iter},
            {NodeType::StructTypeNode, (Func)StructTypeNode_iter},
            {NodeType::RegionTypeNode, (Func)RegionTypeNode_iter},
            {NodeType::UnionTypeNode, (Func)UnionTypeNode_iter},
            {NodeType::ArrayTypeNode, (Func)ArrayTypeNode_iter},
            {NodeType::FunctionTypeNode, (Func)FunctionTypeNode_iter},
            {NodeType::UserTypeNode, (Func)UserTypeNode_iter},
            {NodeType::IntegerLiteralNode, (Func)IntegerLiteralNode_iter},
            {NodeType::FloatLiteralNode, (Func)FloatLiteralNode_iter},
            {NodeType::StringLiteralNode, (Func)StringLiteralNode_iter},
            {NodeType::CharLiteralNode, (Func)CharLiteralNode_iter},
            {NodeType::BoolLiteralNode, (Func)BoolLiteralNode_iter},
            {NodeType::NullLiteralNode, (Func)NullLiteralNode_iter},
            {NodeType::TypedefNode, (Func)TypedefNode_iter},
            {NodeType::VarDeclNode, (Func)VarDeclNode_iter},
            {NodeType::LetDeclNode, (Func)LetDeclNode_iter},
            {NodeType::FunctionDeclNode, (Func)FunctionDeclNode_iter},
            {NodeType::StructDefNode, (Func)StructDefNode_iter},
            {NodeType::StructFieldNode, (Func)StructFieldNode_iter},
            {NodeType::RegionDefNode, (Func)RegionDefNode_iter},
            {NodeType::RegionFieldNode, (Func)RegionFieldNode_iter},
            {NodeType::GroupDefNode, (Func)GroupDefNode_iter},
            {NodeType::GroupFieldNode, (Func)GroupFieldNode_iter},
            {NodeType::UnionDefNode, (Func)UnionDefNode_iter},
            {NodeType::UnionFieldNode, (Func)UnionFieldNode_iter},
            {NodeType::EnumDefNode, (Func)EnumDefNode_iter},
            {NodeType::EnumFieldNode, (Func)EnumFieldNode_iter},
            {NodeType::FunctionDefNode, (Func)FunctionDefNode_iter},
            {NodeType::FunctionParamNode, (Func)FunctionParamNode_iter},
            {NodeType::SubsystemNode, (Func)SubsystemNode_iter},
            {NodeType::ExportNode, (Func)ExportNode_iter},
            {NodeType::InlineAsmNode, (Func)InlineAsmNode_iter},
            {NodeType::ReturnStmtNode, (Func)ReturnStmtNode_iter},
            {NodeType::RetifStmtNode, (Func)RetifStmtNode_iter},
            {NodeType::RetzStmtNode, (Func)RetzStmtNode_iter},
            {NodeType::RetvStmtNode, (Func)RetvStmtNode_iter},
            {NodeType::IfStmtNode, (Func)IfStmtNode_iter},
            {NodeType::WhileStmtNode, (Func)WhileStmtNode_iter},
            {NodeType::ForStmtNode, (Func)ForStmtNode_iter},
        };

    if (!node_map.contains(node->ntype))
        LOG(FATAL) << "No conversion function for node type " << (int)node->ntype << " found." << std::endl;

    return node_map.at(node->ntype)(state, node);
}

size_t libquixcc::traversal::ASTPreorderTraversal::ASTNopNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ASTNopNode *node)
{
    return 0;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ExprStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ExprStmtNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::NopStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::NopStmtNode *node)
{
    return 0;
}

size_t libquixcc::traversal::ASTPreorderTraversal::BlockNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::BlockNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&stmt));
        count += next(state, stmt);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::StmtGroupNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::StmtGroupNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&stmt));
        count += next(state, stmt);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::CastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::CastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::StaticCastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::StaticCastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::BitCastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::BitCastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::SignedUpcastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::SignedUpcastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::UnsignedUpcastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::UnsignedUpcastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::DowncastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::DowncastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::PtrToIntCastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::PtrToIntCastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::IntToPtrCastExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::IntToPtrCastExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::UnaryExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::UnaryExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::BinaryExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::BinaryExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_lhs));
    size_t count = next(state, node->m_lhs);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_rhs));
    count += next(state, node->m_rhs);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::CallExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::CallExprNode *node)
{
    size_t count = 0;
    for (auto &arg : node->m_positional_args)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&arg));
        count += next(state, arg);
    }
    for (auto &arg : node->m_named_args)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&arg.second));
        count += next(state, arg.second);
    }

    if (node->m_decl)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_decl));
        count += next(state, node->m_decl);
    }

    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ListExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ListExprNode *node)
{
    size_t count = 0;
    for (auto &elem : node->m_elements)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&elem));
        count += next(state, elem);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::MemberAccessNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::MemberAccessNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ConstUnaryExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ConstUnaryExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ConstBinaryExprNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ConstBinaryExprNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_lhs));
    size_t count = next(state, node->m_lhs);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_rhs));
    count += next(state, node->m_rhs);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::IdentifierNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::IdentifierNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::MutTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::MutTypeNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    return next(state, node->m_type) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::U8TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::U8TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::U16TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::U16TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::U32TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::U32TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::U64TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::U64TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::I8TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::I8TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::I16TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::I16TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::I32TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::I32TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::I64TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::I64TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::F32TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::F32TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::F64TypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::F64TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::BoolTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::BoolTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::VoidTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::VoidTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::PointerTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::PointerTypeNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    return next(state, node->m_type) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::OpaqueTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::OpaqueTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::StringTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::StringTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::EnumTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::EnumTypeNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_member_type));
    return next(state, node->m_member_type) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::StructTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::StructTypeNode *node)
{
    if (state.m_visited.contains(node))
        return 1;

    state.m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::RegionTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::RegionTypeNode *node)
{
    if (state.m_visited.contains(node))
        return 1;

    state.m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::UnionTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::UnionTypeNode *node)
{
    if (state.m_visited.contains(node))
        return 1;

    state.m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ArrayTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ArrayTypeNode *node)
{
    size_t count = 0;
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    count += next(state, node->m_type);
    if (node->m_size)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_size));
        count += next(state, node->m_size);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::FunctionTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::FunctionTypeNode *node)
{
    size_t count = 0;
    for (auto &param : node->m_params)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&param.second));
        count += next(state, param.second);
    }
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_return_type));
    count += next(state, node->m_return_type);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::UserTypeNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::UserTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::IntegerLiteralNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::IntegerLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::FloatLiteralNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::FloatLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::StringLiteralNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::StringLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::CharLiteralNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::CharLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::BoolLiteralNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::BoolLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::NullLiteralNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::NullLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::TypedefNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::TypedefNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_orig));
    return next(state, node->m_orig) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::VarDeclNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::VarDeclNode *node)
{
    state.m_visited.clear();

    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_init)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_init));
        count += next(state, node->m_init);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::LetDeclNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::LetDeclNode *node)
{
    state.m_visited.clear();

    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_init)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_init));
        count += next(state, node->m_init);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::FunctionDeclNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::FunctionDeclNode *node)
{
    state.m_visited.clear();

    size_t count = 0;
    state.m_prefix.push_back(node->m_name);

    for (auto &param : node->m_params)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&param));
        count += next(state, param);
    }
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    count += next(state, node->m_type);

    state.m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::StructDefNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::StructDefNode *node)
{
    size_t count = 0;
    state.m_prefix.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }

    for (auto &method : node->m_methods)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&method));
        count += next(state, method);
    }

    for (auto &method : node->m_static_methods)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&method));
        count += next(state, method);
    }

    state.m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::StructFieldNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::StructFieldNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::RegionDefNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::RegionDefNode *node)
{
    size_t count = 0;
    state.m_prefix.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }

    state.m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::RegionFieldNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::RegionFieldNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::GroupDefNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::GroupDefNode *node)
{
    size_t count = 0;
    state.m_prefix.push_back(node->m_name);

    for (auto &field : node->get_fields_mut())
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }

    state.m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::GroupFieldNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::GroupFieldNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::UnionDefNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::UnionDefNode *node)
{
    size_t count = 0;
    state.m_prefix.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }

    state.m_prefix.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::UnionFieldNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::UnionFieldNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::EnumDefNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::EnumDefNode *node)
{
    if (node->m_scoped)
        state.m_prefix.push_back(node->m_type->m_name);

    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&field));
        count += next(state, field);
    }

    if (node->m_scoped)
        state.m_prefix.pop_back();

    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::EnumFieldNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::EnumFieldNode *node)
{
    if (node->m_value)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_value));
        return next(state, node->m_value) + 1;
    }
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::FunctionDefNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::FunctionDefNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_decl));
    size_t count = next(state, node->m_decl);

    state.m_prefix.push_back(node->m_decl->m_name);

    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_body));
    count += next(state, node->m_body);

    state.m_prefix.pop_back();

    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::FunctionParamNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::FunctionParamNode *node)
{
    state.m_visited.clear();

    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::SubsystemNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::SubsystemNode *node)
{
    state.m_prefix.push_back(node->m_name);

    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_block));
    size_t count = next(state, node->m_block);

    state.m_prefix.pop_back();

    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ExportNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ExportNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&stmt));
        count += next(state, stmt);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::InlineAsmNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::InlineAsmNode *node)
{
    size_t count = 0;

    for (auto &[reg, expr] : node->m_outputs)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&expr));
        count += next(state, expr);
    }

    for (auto &[reg, expr] : node->m_outputs)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&expr));
        count += next(state, expr);
    }

    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ReturnStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ReturnStmtNode *node)
{
    if (node->m_expr)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_expr));
        return next(state, node->m_expr) + 1;
    }
    return 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::RetifStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::RetifStmtNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_return));
    count += next(state, node->m_return);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::RetzStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::RetzStmtNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_return));
    count += next(state, node->m_return);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::RetvStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::RetvStmtNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_cond));
    return next(state, node->m_cond) + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::IfStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::IfStmtNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_then));
    count += next(state, node->m_then);
    if (node->m_else)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_else));
        count += next(state, node->m_else);
    }
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::WhileStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::WhileStmtNode *node)
{
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_stmt));
    count += next(state, node->m_stmt);
    return count + 1;
}

size_t libquixcc::traversal::ASTPreorderTraversal::ForStmtNode_iter(libquixcc::traversal::ASTTraversalState &state, libquixcc::ForStmtNode *node)
{
    size_t count = 1;

    if (node->m_init)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_init));
        count += next(state, node->m_init);
    }

    if (node->m_cond)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_cond));
        count += next(state, node->m_cond);
    }

    if (node->m_step)
    {
        state.m_callback(state.m_prefix, node, mk_ptr(&node->m_step));
        count += next(state, node->m_step);
    }

    state.m_callback(state.m_prefix, node, mk_ptr(&node->m_stmt));
    count += next(state, node->m_stmt);

    return count;
}