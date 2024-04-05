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

#include <parse/nodes/BasicNodes.h>
#include <parse/nodes/AllNodes.h>

void libquixcc::ParseNode::replace_child(std::shared_ptr<libquixcc::ParseNode> &old_node, std::shared_ptr<libquixcc::ParseNode> new_node)
{
    old_node = new_node;
}

static void nop(std::string _namespace, libquixcc::ParseNode *parrent, std::shared_ptr<libquixcc::ParseNode> *node)
{
    (void)_namespace;
    (void)parrent;
    (void)node;
    return;
}

size_t libquixcc::ParseNode::count()
{
    return dfs_preorder(ParseNodePreorderVisitor(nop, ""));
}

bool libquixcc::ConstExprNode::is_primitive() const
{
    switch (type()->ntype)
    {
    case NodeType::U8TypeNode:
    case NodeType::U16TypeNode:
    case NodeType::U32TypeNode:
    case NodeType::U64TypeNode:
    case NodeType::I8TypeNode:
    case NodeType::I16TypeNode:
    case NodeType::I32TypeNode:
    case NodeType::I64TypeNode:
    case NodeType::F32TypeNode:
    case NodeType::F64TypeNode:
    case NodeType::BoolTypeNode:
    case NodeType::VoidTypeNode:
        return true;
    case NodeType::ConstUnaryExprNode:
        return reinterpret_cast<const ConstUnaryExprNode *>(this)->m_expr->is_primitive();
    case NodeType::ConstBinaryExprNode:
        return reinterpret_cast<const ConstBinaryExprNode *>(this)->m_lhs->is_primitive() &&
               reinterpret_cast<const ConstBinaryExprNode *>(this)->m_rhs->is_primitive();
    default:
        return false;
    };
}