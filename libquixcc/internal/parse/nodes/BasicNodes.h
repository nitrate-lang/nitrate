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
#include <parsetree/Iterate.h>
#include <parsetree/Serialize.h>

namespace libquixcc
{
    class TypeNode;
    class ParseNode;

    class ParseNode
    {
    public:
        ParseNode() = default;

        virtual size_t dfs_preorder(traversal::ParseTreeTraversalState state);
        virtual std::string to_json(serialize::ParseTreeSerializerState state = serialize::ParseTreeSerializerState()) const;

        /// @brief Count the number of nodes in the tree.
        /// @return The number of nodes in the tree.
        size_t count();

        template <typename T>
        T *as()
        {
            auto p = dynamic_cast<T *>(this);
            if (!p)
                LOG(FATAL) << "Invalid cast from `" << typeid(*this).name() << "` to `" << typeid(T).name() << "`" << std::endl;

            return p;
        }

        template <typename T>
        const T *as() const
        {
            auto p = dynamic_cast<const T *>(this);
            if (!p)
                LOG(FATAL) << "Invalid cast from `" << typeid(*this).name() << "` to `" << typeid(T).name() << "`" << std::endl;

            return p;
        }

        template <typename T>
        bool is() const
        {
            return typeid(*this) == typeid(T);
        }

        template <typename T>
        bool isof() const
        {
            if (is<T>())
                return true;

            if (std::is_same_v<T, ConstExprNode>)
                return is<ConstUnaryExprNode>() || is<ConstBinaryExprNode>();

            if (std::is_same_v<T, LiteralNode>)
                return is<IntegerNode>() || is<FloatLiteralNode>() || is<StringNode>() || is<CharNode>() || is<BoolLiteralNode>() || is<NullLiteralNode>();

            if (std::is_same_v<T, CastExprNode>)
                return is<StaticCastExprNode>() || is<BitCastExprNode>() || is<SignedUpcastExprNode>() || is<UnsignedUpcastExprNode>() || is<DowncastExprNode>() || is<IntToPtrCastExprNode>() || is<PtrToIntCastExprNode>();

            if (std::is_same_v<T, ExprNode>)
                return is<UnaryExprNode>() || is<BinaryExprNode>() || is<CallExprNode>() || is<ListExprNode>() || is<MemberAccessNode>() || isof<CastExprNode>() || isof<LiteralNode>();

            if (std::is_same_v<T, TypeNode>)
                return is<ImmMutTypeNode>() || is<U8TypeNode>() || is<U16TypeNode>() || is<U32TypeNode>() || is<U64TypeNode>() || is<U128TypeNode>() || is<I8TypeNode>() || is<I16TypeNode>() || is<I32TypeNode>() || is<I64TypeNode>() || is<I128TypeNode>() || is<F32TypeNode>() || is<F64TypeNode>() || is<BoolTypeNode>() || is<VoidTypeNode>() || is<PointerTypeNode>() || is<OpaqueTypeNode>() || is<StringTypeNode>() || is<EnumTypeNode>() || is<StructTypeNode>() || is<RegionTypeNode>() || is<UnionTypeNode>() || is<ArrayTypeNode>() || is<FunctionTypeNode>() || is<UserTypeNode>();

            if (std::is_same_v<T, StmtNode>)
                return is<ExprStmtNode>() || is<NopStmtNode>() || is<DeclNode>() || is<DefNode>() || is<BlockNode>() || is<StmtGroupNode>() || is<ReturnStmtNode>() || is<RetifStmtNode>() || is<RetzStmtNode>() || is<RetvStmtNode>() || is<IfStmtNode>() || is<WhileStmtNode>() || is<ForStmtNode>() || is<SubsystemNode>() || is<ExportNode>() || is<InlineAsmNode>();

            if (std::is_same_v<T, DeclNode>)
                return is<VarDeclNode>() || is<LetDeclNode>() || is<FunctionDeclNode>() || is<FunctionParamNode>() || is<TypedefNode>() || is<ConstDeclNode>();

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

    class ExprNode : public ParseNode
    {
    public:
        ExprNode() = default;
    };

    class ConstExprNode : public ExprNode
    {
    public:
        ConstExprNode() { ntype = NodeType::ConstExprNode; }
    };

    class StmtNode : public ParseNode
    {
    public:
        StmtNode() { ntype = NodeType::StmtNode; }
    };

    class ExprStmtNode : public StmtNode
    {
    public:
        ExprStmtNode(std::shared_ptr<ExprNode> expr) : m_expr(expr) { ntype = NodeType::ExprStmtNode; }

        std::shared_ptr<ExprNode> m_expr;
    };

    class NopStmtNode : public StmtNode
    {
    public:
        NopStmtNode() { ntype = NodeType::NopStmtNode; }
    };

    class TypeNode : public ParseNode
    {
    public:
        TypeNode() { ntype = NodeType::TypeNode; }
        ~TypeNode() = default;
        TypeNode(const TypeNode &) = delete;
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

        std::string m_name;
    };

    class DeclNode : public StmtNode
    {
    public:
        DeclNode() { ntype = NodeType::DeclNode; }
    };

    class DefNode : public StmtNode
    {
    public:
        DefNode() { ntype = NodeType::DefNode; }
    };

    class BlockNode : public StmtNode
    {
    public:
        BlockNode() { ntype = NodeType::BlockNode; }

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };

    class StmtGroupNode : public StmtNode
    {
    public:
        StmtGroupNode(std::vector<std::shared_ptr<StmtNode>> stmts = {}) : m_stmts(stmts) { ntype = NodeType::StmtGroupNode; }

        std::vector<std::shared_ptr<StmtNode>> m_stmts;
    };
}

#endif // __QUIXCC_PARSE_NODES_BASIC_H__