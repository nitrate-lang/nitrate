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

#include <error/Logger.h>
#include <parse/NodeTraversal.h>
#include <parse/JsonSerialize.h>
#include <parse/NodeType.h>
#include <generate/CodeGen.h>

namespace libquixcc
{
    class TypeNode;

    struct ReductionState
    {
        bool m_export = false;
        bool m_fn_def = false;
    };
    class ParseNode
    {
    public:
        ParseNode() = default;

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const = 0;

        /// @brief Replace a child node with a new node.
        /// @warning Careful, this function depends on wierd reinterpretation stuff.
        /// @param old_node **Reference** to the old node member in the parent node.
        /// @param new_node The new node to replace with.
        void replace_child(std::shared_ptr<libquixcc::ParseNode> &old_node, const std::shared_ptr<libquixcc::ParseNode> new_node);

        /// @brief Count the number of nodes in the tree.
        /// @return The number of nodes in the tree.
        size_t count();

        template <typename T>
        T *as()
        {
            if (!is<T>())
                LOG(FATAL) << "Invalid cast from " << (int)ntype << std::endl;

            return static_cast<T *>(this);
        }

        template <typename T>
        const T *as() const
        {
            if (!is<T>())
                LOG(FATAL) << "Invalid cast from " << (int)ntype << std::endl;

            return static_cast<const T *>(this);
        }

        template <typename T>
        bool is() const
        {
            switch (ntype)
            {
            case NodeType::ParseNode:
                return std::is_same_v<T, ParseNode>;
            case NodeType::ASTNopNode:
                return std::is_same_v<T, ASTNopNode>;
            case NodeType::ExprNode:
                return std::is_same_v<T, ExprNode>;
            case NodeType::ConstExprNode:
                return std::is_same_v<T, ConstExprNode>;
            case NodeType::StmtNode:
                return std::is_same_v<T, StmtNode>;
            case NodeType::ExprStmtNode:
                return std::is_same_v<T, ExprStmtNode>;
            case NodeType::NopStmtNode:
                return std::is_same_v<T, NopStmtNode>;
            case NodeType::TypeNode:
                return std::is_same_v<T, TypeNode>;
            case NodeType::DeclNode:
                return std::is_same_v<T, DeclNode>;
            case NodeType::DefNode:
                return std::is_same_v<T, DefNode>;
            case NodeType::BlockNode:
                return std::is_same_v<T, BlockNode>;
            case NodeType::StmtGroupNode:
                return std::is_same_v<T, StmtGroupNode>;

            case NodeType::ConstUnaryExprNode:
                return std::is_same_v<T, ConstUnaryExprNode>;
            case NodeType::ConstBinaryExprNode:
                return std::is_same_v<T, ConstBinaryExprNode>;

            case NodeType::UnaryExprNode:
                return std::is_same_v<T, UnaryExprNode>;
            case NodeType::BinaryExprNode:
                return std::is_same_v<T, BinaryExprNode>;
            case NodeType::CallExprNode:
                return std::is_same_v<T, CallExprNode>;
            case NodeType::ListExprNode:
                return std::is_same_v<T, ListExprNode>;
            case NodeType::MemberAccessNode:
                return std::is_same_v<T, MemberAccessNode>;

            case NodeType::IdentifierNode:
                return std::is_same_v<T, IdentifierNode>;

            case NodeType::MutTypeNode:
                return std::is_same_v<T, MutTypeNode>;
            case NodeType::U8TypeNode:
                return std::is_same_v<T, U8TypeNode>;
            case NodeType::U16TypeNode:
                return std::is_same_v<T, U16TypeNode>;
            case NodeType::U32TypeNode:
                return std::is_same_v<T, U32TypeNode>;
            case NodeType::U64TypeNode:
                return std::is_same_v<T, U64TypeNode>;
            case NodeType::I8TypeNode:
                return std::is_same_v<T, I8TypeNode>;
            case NodeType::I16TypeNode:
                return std::is_same_v<T, I16TypeNode>;
            case NodeType::I32TypeNode:
                return std::is_same_v<T, I32TypeNode>;
            case NodeType::I64TypeNode:
                return std::is_same_v<T, I64TypeNode>;
            case NodeType::F32TypeNode:
                return std::is_same_v<T, F32TypeNode>;
            case NodeType::F64TypeNode:
                return std::is_same_v<T, F64TypeNode>;
            case NodeType::BoolTypeNode:
                return std::is_same_v<T, BoolTypeNode>;
            case NodeType::VoidTypeNode:
                return std::is_same_v<T, VoidTypeNode>;
            case NodeType::PointerTypeNode:
                return std::is_same_v<T, PointerTypeNode>;
            case NodeType::OpaqueTypeNode:
                return std::is_same_v<T, OpaqueTypeNode>;
            case NodeType::StringTypeNode:
                return std::is_same_v<T, StringTypeNode>;

            case NodeType::EnumTypeNode:
                return std::is_same_v<T, EnumTypeNode>;
            case NodeType::StructTypeNode:
                return std::is_same_v<T, StructTypeNode>;
            case NodeType::RegionTypeNode:
                return std::is_same_v<T, RegionTypeNode>;
            case NodeType::UnionTypeNode:
                return std::is_same_v<T, UnionTypeNode>;
            case NodeType::ArrayTypeNode:
                return std::is_same_v<T, ArrayTypeNode>;
            case NodeType::FunctionTypeNode:
                return std::is_same_v<T, FunctionTypeNode>;
            case NodeType::UserTypeNode:
                return std::is_same_v<T, UserTypeNode>;

            case NodeType::LiteralNode:
                return std::is_same_v<T, LiteralNode>;
            case NodeType::IntegerLiteralNode:
                return std::is_same_v<T, IntegerLiteralNode>;
            case NodeType::FloatLiteralNode:
                return std::is_same_v<T, FloatLiteralNode>;
            case NodeType::StringLiteralNode:
                return std::is_same_v<T, StringLiteralNode>;
            case NodeType::CharLiteralNode:
                return std::is_same_v<T, CharLiteralNode>;
            case NodeType::BoolLiteralNode:
                return std::is_same_v<T, BoolLiteralNode>;
            case NodeType::NullLiteralNode:
                return std::is_same_v<T, NullLiteralNode>;

            case NodeType::TypedefNode:
                return std::is_same_v<T, TypedefNode>;

            case NodeType::VarDeclNode:
                return std::is_same_v<T, VarDeclNode>;
            case NodeType::LetDeclNode:
                return std::is_same_v<T, LetDeclNode>;

            case NodeType::FunctionDeclNode:
                return std::is_same_v<T, FunctionDeclNode>;

            case NodeType::StructDefNode:
                return std::is_same_v<T, StructDefNode>;
            case NodeType::StructFieldNode:
                return std::is_same_v<T, StructFieldNode>;
            case NodeType::RegionDefNode:
                return std::is_same_v<T, RegionDefNode>;
            case NodeType::RegionFieldNode:
                return std::is_same_v<T, RegionFieldNode>;
            case NodeType::GroupDefNode:
                return std::is_same_v<T, GroupDefNode>;
            case NodeType::GroupFieldNode:
                return std::is_same_v<T, GroupFieldNode>;
            case NodeType::UnionDefNode:
                return std::is_same_v<T, UnionDefNode>;
            case NodeType::UnionFieldNode:
                return std::is_same_v<T, UnionFieldNode>;
            case NodeType::EnumDefNode:
                return std::is_same_v<T, EnumDefNode>;
            case NodeType::EnumFieldNode:
                return std::is_same_v<T, EnumFieldNode>;
            case NodeType::FunctionDefNode:
                return std::is_same_v<T, FunctionDefNode>;
            case NodeType::FunctionParamNode:
                return std::is_same_v<T, FunctionParamNode>;

            case NodeType::SubsystemNode:
                return std::is_same_v<T, SubsystemNode>;
            case NodeType::ExportNode:
                return std::is_same_v<T, ExportNode>;
            case NodeType::InlineAsmNode:
                return std::is_same_v<T, InlineAsmNode>;

            case NodeType::ReturnStmtNode:
                return std::is_same_v<T, ReturnStmtNode>;
            case NodeType::RetifStmtNode:
                return std::is_same_v<T, RetifStmtNode>;
            case NodeType::RetzStmtNode:
                return std::is_same_v<T, RetzStmtNode>;
            case NodeType::RetvStmtNode:
                return std::is_same_v<T, RetvStmtNode>;
            case NodeType::IfStmtNode:
                return std::is_same_v<T, IfStmtNode>;
            case NodeType::WhileStmtNode:
                return std::is_same_v<T, WhileStmtNode>;
            case NodeType::ForStmtNode:
                return std::is_same_v<T, ForStmtNode>;
            }

            assert(false);
            return false;
        }

        NodeType ntype = NodeType::ParseNode;
    };

    struct TypeInferenceState
    {
        std::shared_ptr<ParseNode> m_root;
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
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const = 0;

    public:
        ExprNode() = default;

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(this); }

        template <typename T>
        std::shared_ptr<T> reduce(ReductionState &state) const
        {
            return std::static_pointer_cast<T>(reduce_impl(state));
        }

        template <typename T>
        std::shared_ptr<T> reduce() const
        {
            ReductionState state;
            return std::static_pointer_cast<T>(reduce_impl(state));
        }

        virtual TypeNode *infer(TypeInferenceState &state) const = 0;
        bool is_const_expr() const { return reduce<ExprNode>()->ntype == NodeType::LiteralNode; }
    };

    class ConstExprNode : public ExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override = 0;

    public:
        ConstExprNode() { ntype = NodeType::ConstExprNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual llvm::Constant *codegen(CodegenVisitor &visitor) const { return static_cast<llvm::Constant *>(visitor.visit(static_cast<const ExprNode *>(this))); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(static_cast<const ExprNode *>(this)); }
        virtual bool is_negative() const;
        virtual TypeNode *infer(TypeInferenceState &state) const override = 0;
    };

    class StmtNode : public ParseNode
    {
    public:
        StmtNode() { ntype = NodeType::StmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const = 0;
    };

    class ExprStmtNode : public StmtNode
    {
    public:
        ExprStmtNode(std::shared_ptr<ExprNode> expr) : m_expr(expr) { ntype = NodeType::ExprStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_expr;
    };

    class NopStmtNode : public StmtNode
    {
    public:
        NopStmtNode() { ntype = NodeType::NopStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;
    };

    class TypeNode : public ParseNode
    {
    public:
        TypeNode() { ntype = NodeType::TypeNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual llvm::Type *codegen(CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual bool is_composite() const = 0;
        bool is_ptr() const;
        bool is_array() const;
        bool is_func() const;
        bool is_void() const;
        bool is_signed() const;
        bool is_integer() const;
        bool is_floating() const;
        bool is_bool() const;

        virtual size_t size(size_t ptr_size) const = 0;
        virtual std::string to_source() const = 0;
        virtual std::string name() const { return ""; }
    };

    class UserTypeNode : public TypeNode
    {
        UserTypeNode(const std::string &name) : m_name(name) { ntype = NodeType::UserTypeNode; }
        static thread_local std::unordered_map<std::string, std::shared_ptr<UserTypeNode>> m_instances;

    public:
        static UserTypeNode *create(const std::string &name)
        {
            if (!m_instances.contains(name))
                m_instances[name] = std::shared_ptr<UserTypeNode>(new UserTypeNode(name));

            return m_instances[name].get();
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { throw std::runtime_error("UserTypeNode::is_composite() not implemented"); }
        virtual size_t size(size_t ptr_size) const { return 0; }
        virtual std::string to_source() const override { return m_name; }
        virtual std::string name() const override { return m_name; }

        std::string m_name;
    };

    class DeclNode : public StmtNode
    {
    public:
        DeclNode() { ntype = NodeType::DeclNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override = 0;
    };

    class DefNode : public StmtNode
    {
    public:
        DefNode() { ntype = NodeType::DefNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override = 0;
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override = 0;
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override = 0;
    };

    class BlockNode : public StmtNode
    {
    public:
        BlockNode() { ntype = NodeType::BlockNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };

    class StmtGroupNode : public StmtNode
    {
    public:
        StmtGroupNode(std::vector<std::shared_ptr<StmtNode>> stmts = {}) : m_stmts(stmts) { ntype = NodeType::StmtGroupNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };
}

#endif // __QUIXCC_PARSE_NODES_BASIC_H__