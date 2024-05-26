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

#include <parsetree/Iterate.h>
#include <parsetree/nodes/AllNodes.h>

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

size_t libquixcc::ParseNode::dfs_preorder(libquixcc::traversal::ParseTreeTraversalState state)
{
    return traversal::ParseTreePreorder::next(state, this);
}

size_t libquixcc::traversal::ParseTreePreorder::dispatch(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ParseNode *node)
{
    typedef size_t (*Func)(libquixcc::traversal::ParseTreeTraversalState &, libquixcc::ParseNode *);

    static const std::unordered_map<NodeType, Func> node_map =
        {
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
            {NodeType::IndexNode, (Func)IndexNode_iter},
            {NodeType::ConstUnaryExprNode, (Func)ConstUnaryExprNode_iter},
            {NodeType::ConstBinaryExprNode, (Func)ConstBinaryExprNode_iter},
            {NodeType::IdentifierNode, (Func)IdentifierNode_iter},
            {NodeType::MutTypeNode, (Func)MutTypeNode_iter},
            {NodeType::U8TypeNode, (Func)U8TypeNode_iter},
            {NodeType::U16TypeNode, (Func)U16TypeNode_iter},
            {NodeType::U32TypeNode, (Func)U32TypeNode_iter},
            {NodeType::U64TypeNode, (Func)U64TypeNode_iter},
            {NodeType::U128TypeNode, (Func)U128TypeNode_iter},
            {NodeType::I8TypeNode, (Func)I8TypeNode_iter},
            {NodeType::I16TypeNode, (Func)I16TypeNode_iter},
            {NodeType::I32TypeNode, (Func)I32TypeNode_iter},
            {NodeType::I64TypeNode, (Func)I64TypeNode_iter},
            {NodeType::I128TypeNode, (Func)I128TypeNode_iter},
            {NodeType::F32TypeNode, (Func)F32TypeNode_iter},
            {NodeType::F64TypeNode, (Func)F64TypeNode_iter},
            {NodeType::BoolTypeNode, (Func)BoolTypeNode_iter},
            {NodeType::VoidTypeNode, (Func)VoidTypeNode_iter},
            {NodeType::PointerTypeNode, (Func)PointerTypeNode_iter},
            {NodeType::OpaqueTypeNode, (Func)OpaqueTypeNode_iter},
            {NodeType::StringTypeNode, (Func)StringTypeNode_iter},
            {NodeType::EnumTypeNode, (Func)EnumTypeNode_iter},
            {NodeType::StructTypeNode, (Func)StructTypeNode_iter},
            {NodeType::GroupTypeNode, (Func)GroupTypeNode_iter},
            {NodeType::RegionTypeNode, (Func)RegionTypeNode_iter},
            {NodeType::UnionTypeNode, (Func)UnionTypeNode_iter},
            {NodeType::ArrayTypeNode, (Func)ArrayTypeNode_iter},
            {NodeType::VectorTypeNode, (Func)VectorTypeNode_iter},
            {NodeType::FunctionTypeNode, (Func)FunctionTypeNode_iter},
            {NodeType::UserTypeNode, (Func)UserTypeNode_iter},
            {NodeType::IntegerNode, (Func)IntegerNode_iter},
            {NodeType::FloatLiteralNode, (Func)FloatLiteralNode_iter},
            {NodeType::StringNode, (Func)StringNode_iter},
            {NodeType::CharNode, (Func)CharNode_iter},
            {NodeType::BoolLiteralNode, (Func)BoolLiteralNode_iter},
            {NodeType::NullLiteralNode, (Func)NullLiteralNode_iter},
            {NodeType::TypedefNode, (Func)TypedefNode_iter},
            {NodeType::VarDeclNode, (Func)VarDeclNode_iter},
            {NodeType::LetDeclNode, (Func)LetDeclNode_iter},
            {NodeType::ConstDeclNode, (Func)ConstDeclNode_iter},
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

size_t libquixcc::traversal::ParseTreePreorder::ExprStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ExprStmtNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::NopStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::NopStmtNode *node)
{
    return 0;
}

size_t libquixcc::traversal::ParseTreePreorder::BlockNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::BlockNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&stmt));
        count += next(state, stmt);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::StmtGroupNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::StmtGroupNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&stmt));
        count += next(state, stmt);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::CastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::CastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::StaticCastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::StaticCastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::BitCastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::BitCastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::SignedUpcastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::SignedUpcastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::UnsignedUpcastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::UnsignedUpcastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::DowncastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::DowncastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::PtrToIntCastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::PtrToIntCastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::IntToPtrCastExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::IntToPtrCastExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    count += next(state, node->m_expr);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::UnaryExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::UnaryExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::BinaryExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::BinaryExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_lhs));
    size_t count = next(state, node->m_lhs);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_rhs));
    count += next(state, node->m_rhs);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::CallExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::CallExprNode *node)
{
    size_t count = 0;
    
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_callee));
    count += next(state, node->m_callee);

    for (auto &arg : node->m_positional_args)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&arg));
        count += next(state, arg);
    }
    for (auto &arg : node->m_named_args)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&arg.second));
        count += next(state, arg.second);
    }

    if (node->m_decl)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_decl));
        count += next(state, node->m_decl);
    }

    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ListExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ListExprNode *node)
{
    size_t count = 0;
    for (auto &elem : node->m_elements)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&elem));
        count += next(state, elem);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::MemberAccessNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::MemberAccessNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::IndexNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::IndexNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    size_t count = next(state, node->m_expr);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_index));
    count += next(state, node->m_index);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ConstUnaryExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ConstUnaryExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
    return next(state, node->m_expr) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ConstBinaryExprNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ConstBinaryExprNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_lhs));
    size_t count = next(state, node->m_lhs);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_rhs));
    count += next(state, node->m_rhs);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::IdentifierNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::IdentifierNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::MutTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::MutTypeNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    return next(state, node->m_type) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::U8TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::U8TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::U16TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::U16TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::U32TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::U32TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::U64TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::U64TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::U128TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::U128TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::I8TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::I8TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::I16TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::I16TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::I32TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::I32TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::I64TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::I64TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::I128TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::I128TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::F32TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::F32TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::F64TypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::F64TypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::BoolTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::BoolTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::VoidTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::VoidTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::PointerTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::PointerTypeNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    return next(state, node->m_type) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::OpaqueTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::OpaqueTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::StringTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::StringTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::EnumTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::EnumTypeNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_member_type));
    return next(state, node->m_member_type) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::StructTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::StructTypeNode *node)
{
    if (state.m_visited.contains(node))
        return 1;

    state.m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::GroupTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::GroupTypeNode *node)
{
    if (state.m_visited.contains(node))
        return 1;

    state.m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::RegionTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::RegionTypeNode *node)
{
    if (state.m_visited.contains(node))
        return 1;

    state.m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::UnionTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::UnionTypeNode *node)
{
    if (state.m_visited.contains(node))
        return 1;

    state.m_visited.insert(node); // Prevent infinite recursion

    size_t count = 0;
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ArrayTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ArrayTypeNode *node)
{
    size_t count = 0;
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    count += next(state, node->m_type);
    if (node->m_size)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_size));
        count += next(state, node->m_size);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::VectorTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::VectorTypeNode *node)
{
    size_t count = 0;
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    count += next(state, node->m_type);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::FunctionTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::FunctionTypeNode *node)
{
    size_t count = 0;
    for (auto &param : node->m_params)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&param.second));
        count += next(state, param.second);
    }
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_return_type));
    count += next(state, node->m_return_type);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::UserTypeNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::UserTypeNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::IntegerNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::IntegerNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::FloatLiteralNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::FloatLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::StringNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::StringNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::CharNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::CharNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::BoolLiteralNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::BoolLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::NullLiteralNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::NullLiteralNode *node)
{
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::TypedefNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::TypedefNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_orig));
    return next(state, node->m_orig) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::VarDeclNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::VarDeclNode *node)
{
    state.m_visited.clear();

    size_t count = 0;

    if (node->m_type)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
        count += next(state, node->m_type);
    }

    if (node->m_init)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_init));
        count += next(state, node->m_init);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::LetDeclNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::LetDeclNode *node)
{
    state.m_visited.clear();

    size_t count = 0;
    
    if (node->m_type)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
        count += next(state, node->m_type);
    }

    if (node->m_init)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_init));
        count += next(state, node->m_init);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ConstDeclNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ConstDeclNode *node)
{
    state.m_visited.clear();
    size_t count = 0;

    if (node->m_type)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
        count += next(state, node->m_type);
    }

    if (node->m_init)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_init));
        count += next(state, node->m_init);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::FunctionDeclNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::FunctionDeclNode *node)
{
    state.m_visited.clear();

    size_t count = 0;
    state.m_scope.push_back(node->m_name);

    for (auto &param : node->m_params)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&param));
        count += next(state, param);
    }
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    count += next(state, node->m_type);

    state.m_scope.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::StructDefNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::StructDefNode *node)
{
    size_t count = 0;
    state.m_scope.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }

    for (auto &method : node->m_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    for (auto &method : node->m_static_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    state.m_scope.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::StructFieldNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::StructFieldNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::RegionDefNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::RegionDefNode *node)
{
   size_t count = 0;
    state.m_scope.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }

    for (auto &method : node->m_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    for (auto &method : node->m_static_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    state.m_scope.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::RegionFieldNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::RegionFieldNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::GroupDefNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::GroupDefNode *node)
{
   size_t count = 0;
    state.m_scope.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }

    for (auto &method : node->m_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    for (auto &method : node->m_static_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    state.m_scope.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::GroupFieldNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::GroupFieldNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::UnionDefNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::UnionDefNode *node)
{
    size_t count = 0;
    state.m_scope.push_back(node->m_name);

    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }

    for (auto &method : node->m_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    for (auto &method : node->m_static_methods)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&method));
        count += next(state, method);
    }

    state.m_scope.pop_back();
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::UnionFieldNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::UnionFieldNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::EnumDefNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::EnumDefNode *node)
{
    if (node->m_scoped)
        state.m_scope.push_back(node->m_type->m_name);

    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    for (auto &field : node->m_fields)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&field));
        count += next(state, field);
    }

    if (node->m_scoped)
        state.m_scope.pop_back();

    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::EnumFieldNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::EnumFieldNode *node)
{
    if (node->m_value)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_value));
        return next(state, node->m_value) + 1;
    }
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::FunctionDefNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::FunctionDefNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_decl));
    size_t count = next(state, node->m_decl);

    state.m_scope.push_back(node->m_decl->m_name);

    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_body));
    count += next(state, node->m_body);

    state.m_scope.pop_back();

    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::FunctionParamNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::FunctionParamNode *node)
{
    state.m_visited.clear();

    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_type));
    size_t count = next(state, node->m_type);
    if (node->m_value)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_value));
        count += next(state, node->m_value);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::SubsystemNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::SubsystemNode *node)
{
    state.m_ns.push_back(node->m_name);
    state.m_scope.push_back(node->m_name);

    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_block));
    size_t count = next(state, node->m_block);

    state.m_scope.pop_back();
    state.m_ns.pop_back();

    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ExportNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ExportNode *node)
{
    size_t count = 0;
    for (auto &stmt : node->m_stmts)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&stmt));
        count += next(state, stmt);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::InlineAsmNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::InlineAsmNode *node)
{
    size_t count = 0;

    for (auto &[reg, expr] : node->m_outputs)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&expr));
        count += next(state, expr);
    }

    for (auto &[reg, expr] : node->m_outputs)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&expr));
        count += next(state, expr);
    }

    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ReturnStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ReturnStmtNode *node)
{
    if (node->m_expr)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_expr));
        return next(state, node->m_expr) + 1;
    }
    return 1;
}

size_t libquixcc::traversal::ParseTreePreorder::RetifStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::RetifStmtNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_return));
    count += next(state, node->m_return);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::RetzStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::RetzStmtNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_return));
    count += next(state, node->m_return);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::RetvStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::RetvStmtNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_cond));
    return next(state, node->m_cond) + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::IfStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::IfStmtNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_then));
    count += next(state, node->m_then);
    if (node->m_else)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_else));
        count += next(state, node->m_else);
    }
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::WhileStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::WhileStmtNode *node)
{
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_cond));
    size_t count = next(state, node->m_cond);
    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_stmt));
    count += next(state, node->m_stmt);
    return count + 1;
}

size_t libquixcc::traversal::ParseTreePreorder::ForStmtNode_iter(libquixcc::traversal::ParseTreeTraversalState &state, libquixcc::ForStmtNode *node)
{
    size_t count = 1;

    if (node->m_init)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_init));
        count += next(state, node->m_init);
    }

    if (node->m_cond)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_cond));
        count += next(state, node->m_cond);
    }

    if (node->m_step)
    {
        state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_step));
        count += next(state, node->m_step);
    }

    state.m_callback(state.m_ns, state.m_scope, node, mk_ptr(&node->m_stmt));
    count += next(state, node->m_stmt);

    return count;
}