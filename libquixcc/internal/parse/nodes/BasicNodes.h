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

#ifndef __QUIXCC_PARSE_NODES_BASIC_H__
#define __QUIXCC_PARSE_NODES_BASIC_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <parse/NodeTraversal.h>
#include <parse/JsonSerialize.h>
#include <parse/NodeType.h>
#include <generate/CodeGen.h>

namespace libquixcc
{
    class TypeNode;

    class ParseNode
    {
    public:
        ParseNode() = default;

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const { return visitor.visit(this); }

        /// @brief Replace a child node with a new node.
        /// @warning Careful, this function depends on wierd reinterpretation stuff.
        /// @param old_node **Reference** to the old node member in the parent node.
        /// @param new_node The new node to replace with.
        void replace_child(std::shared_ptr<libquixcc::ParseNode> &old_node, const std::shared_ptr<libquixcc::ParseNode> new_node);

        /// @brief Count the number of nodes in the tree.
        /// @return The number of nodes in the tree.
        size_t count();

        NodeType ntype = NodeType::ParseNode;
    };

#define PARSE_NODE_SIZE sizeof(ParseNode)

    class ASTNopNode : public ParseNode
    {
    public:
        ASTNopNode() { ntype = NodeType::ASTNopNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class ExprNode : public ParseNode
    {
    public:
        ExprNode() = default;

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const = 0;
    };

    class ConstExprNode : public ExprNode
    {
    public:
        ConstExprNode() { ntype = NodeType::ConstExprNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override = 0;
        virtual std::unique_ptr<LiteralNode> reduce() const = 0;

        virtual int64_t GetInt64() const { throw std::runtime_error("ConstExprNode::GetInt64() not implemented"); }
    };

    class StmtNode : public ParseNode
    {
    public:
        StmtNode() { ntype = NodeType::StmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const = 0;
        virtual std::unique_ptr<StmtNode> reduce() const = 0;
    };

    class ExprStmtNode : public StmtNode
    {
    public:
        ExprStmtNode(std::shared_ptr<ExprNode> expr) : m_expr(expr) { ntype = NodeType::ExprStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

        std::shared_ptr<ExprNode> m_expr;
    };

    class NopStmtNode : public StmtNode
    {
    public:
        NopStmtNode() { ntype = NodeType::NopStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const { throw std::runtime_error("NopStmtNode::codegen not implemented"); }
        virtual std::unique_ptr<StmtNode> reduce() const override;
    };

    class TypeNode : public ParseNode
    {
    public:
        TypeNode() { ntype = NodeType::TypeNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const = 0;
        virtual bool is_composite() const = 0;
        virtual size_t size(size_t ptr_size) const = 0;
        virtual std::string to_source() const = 0;

        bool m_mut = false;
    };

    class UserTypeNode : public TypeNode
    {
        UserTypeNode(const std::string &name) : m_name(name) { ntype = NodeType::UserTypeNode; }
        static std::unordered_map<std::string, UserTypeNode *> m_instances;

    public:
        static UserTypeNode *create(const std::string &name)
        {
            if (m_instances.contains(name))
            {
                return m_instances[name];
            }
            else
            {
                auto node = new UserTypeNode(name);
                m_instances[name] = node;
                return node;
            }
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const { return nullptr; } // never generated
        virtual bool is_composite() const override { throw std::runtime_error("UserTypeNode::is_composite() not implemented"); }
        virtual size_t size(size_t ptr_size) const { throw std::runtime_error("UserTypeNode::size() not implemented"); }
        virtual std::string to_source() const override { return m_name; }

        std::string m_name;
    };

    class DeclNode : public StmtNode
    {
    public:
        DeclNode() { ntype = NodeType::DeclNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override = 0;
    };

    class DefNode : public StmtNode
    {
    public:
        DefNode() { ntype = NodeType::DefNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override = 0;
    };

    class BlockNode : public StmtNode
    {
    public:
        BlockNode() { ntype = NodeType::BlockNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };

    class StmtGroupNode : public StmtNode
    {
    public:
        StmtGroupNode() { ntype = NodeType::StmtGroupNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };
}

#endif // __QUIXCC_PARSE_NODES_BASIC_H__