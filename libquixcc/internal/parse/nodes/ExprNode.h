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
#include <parse/nodes/BasicNodes.h>
#include <parse/nodes/LiteralNode.h>

namespace libquixcc
{
    class UnaryExprNode : public ExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        UnaryExprNode(Operator op, const std::shared_ptr<ExprNode> &expr) : m_op(op), m_expr(expr) { ntype = NodeType::UnaryExprNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual TypeNode *infer(TIState &state) const override;

        Operator m_op;
        std::shared_ptr<ExprNode> m_expr;
    };

    class BinaryExprNode : public ExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        BinaryExprNode(Operator op, const std::shared_ptr<ExprNode> &lhs, const std::shared_ptr<ExprNode> &rhs)
            : m_op(op), m_lhs(lhs), m_rhs(rhs) { ntype = NodeType::BinaryExprNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual TypeNode *infer(TIState &state) const override;

        Operator m_op;
        std::shared_ptr<ExprNode> m_lhs;
        std::shared_ptr<ExprNode> m_rhs;
    };

    class FunctionDeclNode;

    class CallExprNode : public ExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        CallExprNode() { ntype = NodeType::CallExprNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual TypeNode *infer(TIState &state) const override;

        std::string m_name;
        std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> m_named_args;
        std::vector<std::shared_ptr<ExprNode>> m_positional_args;
        std::shared_ptr<FunctionDeclNode> m_decl;
    };

    class ListExprNode : public ExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        ListExprNode(std::vector<std::shared_ptr<ExprNode>> elements) : m_elements(elements) { ntype = NodeType::ListExprNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual TypeNode *infer(TIState &state) const override;

        std::vector<std::shared_ptr<ExprNode>> m_elements;
    };

    class MemberAccessNode : public ExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        MemberAccessNode(const std::shared_ptr<ExprNode> &expr, const std::string &field) : m_expr(expr), m_field(field) { ntype = NodeType::MemberAccessNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual TypeNode *infer(TIState &state) const override;

        std::shared_ptr<ExprNode> m_expr;
        std::string m_field;
    };

    class CastExprNode : public ExprNode
    {
    public:
        CastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : m_expr(expr), m_type(type) { ntype = NodeType::CastExprNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual TypeNode *infer(TIState &state) const override;

        std::shared_ptr<ExprNode> m_expr;
        TypeNode *m_type;
    };

    class StaticCastExprNode : public CastExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        StaticCastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::StaticCastExprNode; }

        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class BitCastExprNode : public CastExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        BitCastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::BitCastExprNode; }

        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class SignedUpcastExprNode : public CastExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        SignedUpcastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::SignedUpcastExprNode; }

        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class UnsignedUpcastExprNode : public CastExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        UnsignedUpcastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::UnsignedUpcastExprNode; }

        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class DowncastExprNode : public CastExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        DowncastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::DowncastExprNode; }

        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class PtrToIntCastExprNode : public CastExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        PtrToIntCastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::PtrToIntCastExprNode; }

        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class IntToPtrCastExprNode : public CastExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override;
        virtual std::shared_ptr<ExprNode> promote_impl() const override;

    public:
        IntToPtrCastExprNode(const std::shared_ptr<ExprNode> &expr, TypeNode *type) : CastExprNode(expr, type) { ntype = NodeType::IntToPtrCastExprNode; }

        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

}

#endif // __QUIXCC_PARSE_NODES_EXPR_H__