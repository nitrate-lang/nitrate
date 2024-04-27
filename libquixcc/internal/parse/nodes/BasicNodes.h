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

#ifndef __QUIXCC_PARSE_NODES_BASIC_H__
#define __QUIXCC_PARSE_NODES_BASIC_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <core/Logger.h>
#include <parse/NodeType.h>
#include <traversal/AST.h>
#include <serialize/ASTJson.h>
#include <generate/CodeGen.h>

namespace libquixcc
{
    class TypeNode;
    class ParseNode;

    struct ReductionState
    {
        bool m_export = false;
        bool m_fn_def = false;
        std::shared_ptr<ParseNode> m_root;
        size_t m_ptr_size = 8;

        // ReductionState() = delete;
        /// TODO: Add a constructor that takes a shared_ptr<ParseNode> and a size_t ptr_size.
        ReductionState() = default;

        ReductionState(std::shared_ptr<ParseNode> root, size_t ptr_size) : m_root(root), m_ptr_size(ptr_size) {}
    };
    class ParseNode
    {
    public:
        ParseNode() = default;

        virtual size_t dfs_preorder(traversal::ASTTraversalState state);
        virtual std::string to_json(serialize::ASTJsonSerializerState state = serialize::ASTJsonSerializerState()) const;

        /// @brief Count the number of nodes in the tree.
        /// @return The number of nodes in the tree.
        size_t count();

        template <typename T>
        T as()
        {
            if (!is<T>())
                LOG(FATAL) << "Invalid cast from " << (int)ntype << std::endl;

            return static_cast<T>(this);
        }

        template <typename T>
        const T as() const
        {
            if (!is<T>())
                LOG(FATAL) << "Invalid cast from " << (int)ntype << std::endl;

            return static_cast<const T>(this);
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

            case NodeType::CastExprNode:
                return std::is_same_v<T, CastExprNode>;
            case NodeType::StaticCastExprNode:
                return std::is_same_v<T, StaticCastExprNode>;
            case NodeType::BitCastExprNode:
                return std::is_same_v<T, BitCastExprNode>;
            case NodeType::SignedUpcastExprNode:
                return std::is_same_v<T, SignedUpcastExprNode>;
            case NodeType::UnsignedUpcastExprNode:
                return std::is_same_v<T, UnsignedUpcastExprNode>;
            case NodeType::DowncastExprNode:
                return std::is_same_v<T, DowncastExprNode>;
            case NodeType::IntToPtrCastExprNode:
                return std::is_same_v<T, IntToPtrCastExprNode>;
            case NodeType::PtrToIntCastExprNode:
                return std::is_same_v<T, PtrToIntCastExprNode>;

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

            throw std::runtime_error("Invalid node type: " + std::to_string((int)ntype));
        }

        template <typename T>
        bool isof() const
        {
            if (is<T>())
                return true;

            if (std::is_same_v<T, ConstExprNode>)
                return is<ConstUnaryExprNode>() || is<ConstBinaryExprNode>();

            if (std::is_same_v<T, LiteralNode>)
                return is<IntegerLiteralNode>() || is<FloatLiteralNode>() || is<StringLiteralNode>() || is<CharLiteralNode>() || is<BoolLiteralNode>() || is<NullLiteralNode>();

            if (std::is_same_v<T, CastExprNode>)
                return is<StaticCastExprNode>() || is<BitCastExprNode>() || is<SignedUpcastExprNode>() || is<UnsignedUpcastExprNode>() || is<DowncastExprNode>() || is<IntToPtrCastExprNode>() || is<PtrToIntCastExprNode>();

            if (std::is_same_v<T, ExprNode>)
                return is<UnaryExprNode>() || is<BinaryExprNode>() || is<CallExprNode>() || is<ListExprNode>() || is<MemberAccessNode>() || isof<CastExprNode>() || isof<LiteralNode>();

            if (std::is_same_v<T, TypeNode>)
                return is<MutTypeNode>() || is<U8TypeNode>() || is<U16TypeNode>() || is<U32TypeNode>() || is<U64TypeNode>() || is<I8TypeNode>() || is<I16TypeNode>() || is<I32TypeNode>() || is<I64TypeNode>() || is<F32TypeNode>() || is<F64TypeNode>() || is<BoolTypeNode>() || is<VoidTypeNode>() || is<PointerTypeNode>() || is<OpaqueTypeNode>() || is<StringTypeNode>() || is<EnumTypeNode>() || is<StructTypeNode>() || is<RegionTypeNode>() || is<UnionTypeNode>() || is<ArrayTypeNode>() || is<FunctionTypeNode>() || is<UserTypeNode>();

            if (std::is_same_v<T, StmtNode>)
                return is<ExprStmtNode>() || is<NopStmtNode>() || is<DeclNode>() || is<DefNode>() || is<BlockNode>() || is<StmtGroupNode>() || is<ReturnStmtNode>() || is<RetifStmtNode>() || is<RetzStmtNode>() || is<RetvStmtNode>() || is<IfStmtNode>() || is<WhileStmtNode>() || is<ForStmtNode>() || is<SubsystemNode>() || is<ExportNode>() || is<InlineAsmNode>();

            if (std::is_same_v<T, DeclNode>)
                return is<VarDeclNode>() || is<LetDeclNode>() || is<FunctionDeclNode>() || is<FunctionParamNode>() || is<TypedefNode>();

            if (std::is_same_v<T, DefNode>)
                return is<EnumDefNode>() || is<FunctionDefNode>() || is<GroupDefNode>() || is<RegionDefNode>() || is<StructDefNode>() || is<UnionDefNode>();

            return false;
        }

        template <typename T>
        bool is_same(const T node) const
        {
            return ntype == node->ntype;
        }

        NodeType ntype = NodeType::ParseNode;
    };

    struct TIState
    {
        std::shared_ptr<ParseNode> m_root;

        TIState(std::shared_ptr<ParseNode> root) : m_root(root) {}
    };

#define PARSE_NODE_SIZE sizeof(ParseNode)

    class ASTNopNode : public ParseNode
    {
    public:
        ASTNopNode() { ntype = NodeType::ASTNopNode; }
    };

    class ExprNode : public ParseNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const = 0;
        virtual std::shared_ptr<ExprNode> promote_impl() const = 0;

    public:
        ExprNode() = default;

        virtual llvm::Value *codegen(CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(this); }

        /// @brief Do type promotion and insert casts if necessary.
        /// @return The promoted expression.
        template <typename T>
        std::shared_ptr<T> promote() const
        {
            return std::static_pointer_cast<T>(promote_impl());
        }

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

        virtual TypeNode *infer(TIState &state) const = 0;
        bool is_const_expr() const { return reduce<ExprNode>()->is<LiteralNode>(); }
    };

    class ConstExprNode : public ExprNode
    {
    protected:
        virtual std::shared_ptr<ExprNode> reduce_impl(ReductionState &state) const override = 0;
        virtual std::shared_ptr<ExprNode> promote_impl() const override = 0;

    public:
        ConstExprNode() { ntype = NodeType::ConstExprNode; }

        virtual llvm::Constant *codegen(CodegenVisitor &visitor) const { return static_cast<llvm::Constant *>(visitor.visit(static_cast<const ExprNode *>(this))); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(static_cast<const ExprNode *>(this)); }
        virtual bool is_negative() const;
        virtual TypeNode *infer(TIState &state) const override = 0;
    };

    class StmtNode : public ParseNode
    {
    public:
        StmtNode() { ntype = NodeType::StmtNode; }

        virtual llvm::Value *codegen(CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const = 0;
    };

    class ExprStmtNode : public StmtNode
    {
    public:
        ExprStmtNode(std::shared_ptr<ExprNode> expr) : m_expr(expr) { ntype = NodeType::ExprStmtNode; }

        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_expr;
    };

    class NopStmtNode : public StmtNode
    {
    public:
        NopStmtNode() { ntype = NodeType::NopStmtNode; }

        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;
    };

    class TypeNode : public ParseNode
    {
    public:
        TypeNode() { ntype = NodeType::TypeNode; }
        ~TypeNode() = default;
        TypeNode(const TypeNode &) = delete;

        virtual llvm::Type *codegen(CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual std::string codegen(C11CodegenVisitor &visitor) const { return visitor.visit(this); }
        bool is_composite() const;
        bool is_ptr() const;
        bool is_array() const;
        bool is_func() const;
        bool is_void() const;
        bool is_signed() const;
        bool is_integer() const;
        bool is_floating() const;
        bool is_bool() const;
        bool is_primitive() const;

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

        virtual size_t size(size_t ptr_size) const { return 0; }
        virtual std::string to_source() const override { return m_name; }
        virtual std::string name() const override { return m_name; }

        std::string m_name;
    };

    class DeclNode : public StmtNode
    {
    public:
        DeclNode() { ntype = NodeType::DeclNode; }

        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override = 0;
    };

    class DefNode : public StmtNode
    {
    public:
        DefNode() { ntype = NodeType::DefNode; }

        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override = 0;
    };

    class BlockNode : public StmtNode
    {
    public:
        BlockNode() { ntype = NodeType::BlockNode; }

        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };

    class StmtGroupNode : public StmtNode
    {
    public:
        StmtGroupNode(std::vector<std::shared_ptr<StmtNode>> stmts = {}) : m_stmts(stmts) { ntype = NodeType::StmtGroupNode; }

        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };
}

#endif // __QUIXCC_PARSE_NODES_BASIC_H__