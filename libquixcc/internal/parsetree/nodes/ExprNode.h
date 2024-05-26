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

#ifndef __QUIXCC_PARSE_NODES_EXPR_H__
#define __QUIXCC_PARSE_NODES_EXPR_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <llvm/LLVMWrapper.h>
#include <lexer/Token.h>
#include <parsetree/nodes/BasicNodes.h>
#include <parsetree/nodes/LiteralNode.h>

namespace libquixcc
{
    class UnaryExprNode : public ExprNode
    {
    public:
        UnaryExprNode(Operator op, const std::shared_ptr<ExprNode> &expr) : m_op(op), m_expr(expr) { ntype = NodeType::UnaryExprNode; }

        Operator m_op;
        std::shared_ptr<ExprNode> m_expr;
    };

    class BinaryExprNode : public ExprNode
    {
    public:
        BinaryExprNode(Operator op, const std::shared_ptr<ExprNode> &lhs, const std::shared_ptr<ExprNode> &rhs)
            : m_op(op), m_lhs(lhs), m_rhs(rhs) { ntype = NodeType::BinaryExprNode; }

        Operator m_op;
        std::shared_ptr<ExprNode> m_lhs;
        std::shared_ptr<ExprNode> m_rhs;
    };

    class FunctionDeclNode;

    class CallExprNode : public ExprNode
    {
    public:
        CallExprNode() { ntype = NodeType::CallExprNode; }

        std::string m_name;
        std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> m_named_args;
        std::vector<std::shared_ptr<ExprNode>> m_positional_args;
        std::shared_ptr<FunctionDeclNode> m_decl;
    };

    class ListExprNode : public ExprNode
    {
    public:
        ListExprNode(std::vector<std::shared_ptr<ExprNode>> elements) : m_elements(elements) { ntype = NodeType::ListExprNode; }

        std::vector<std::shared_ptr<ExprNode>> m_elements;
    };

    class MemberAccessNode : public ExprNode
    {
    public:
        MemberAccessNode(const std::shared_ptr<ExprNode> &expr, const std::string &field) : m_expr(expr), m_field(field) { ntype = NodeType::MemberAccessNode; }

        std::shared_ptr<ExprNode> m_expr;
        std::string m_field;
    };

    class IndexNode : public ExprNode
    {
    public:
        IndexNode(const std::shared_ptr<ExprNode> &expr, const std::shared_ptr<ExprNode> &index) : m_expr(expr), m_index(index) { ntype = NodeType::IndexNode; }

        std::shared_ptr<ExprNode> m_expr;
        std::shared_ptr<ExprNode> m_index;
    };

    class CastExprNode : public ExprNode
    {
    public:
        CastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : m_expr(expr), m_type(type) { ntype = NodeType::CastExprNode; }

        std::shared_ptr<ExprNode> m_expr;
        TypeNode *m_type;
    };

    class StaticCastExprNode : public CastExprNode
    {
    public:
        StaticCastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::StaticCastExprNode; }
    };

    class BitCastExprNode : public CastExprNode
    {
    public:
        BitCastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::BitCastExprNode; }
    };

    class SignedUpcastExprNode : public CastExprNode
    {
    public:
        SignedUpcastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::SignedUpcastExprNode; }
    };

    class UnsignedUpcastExprNode : public CastExprNode
    {
    public:
        UnsignedUpcastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::UnsignedUpcastExprNode; }
    };

    class DowncastExprNode : public CastExprNode
    {
    public:
        DowncastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::DowncastExprNode; }
    };

    class PtrToIntCastExprNode : public CastExprNode
    {
    public:
        PtrToIntCastExprNode(const std::shared_ptr<ExprNode> &expr) : CastExprNode(expr, nullptr) { ntype = NodeType::PtrToIntCastExprNode; }
    };

    class IntToPtrCastExprNode : public CastExprNode
    {
    public:
        IntToPtrCastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::IntToPtrCastExprNode; }
    };

}

#endif // __QUIXCC_PARSE_NODES_EXPR_H__