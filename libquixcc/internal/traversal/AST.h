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

#ifndef __QUIXCC_TRAVERSAL_AST_H__
#define __QUIXCC_TRAVERSAL_AST_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <set>
#include <vector>
#include <variant>
#include <memory>
#include <functional>
#include <parse/NodeType.h>

namespace libquixcc
{
    namespace traversal
    {
        enum class TraversePtrType
        {
            Smart,
            Raw
        };

        typedef std::pair<TraversePtrType, std::variant<std::shared_ptr<libquixcc::ParseNode> *, libquixcc::ParseNode **>> TraversePtr;

        typedef std::function<void(const std::vector<std::string> &_namespace, libquixcc::ParseNode *, TraversePtr)> ASTPreorderTraversalCallback;

        struct ASTTraversalState
        {
            std::vector<std::string> m_prefix;
            std::set<TypeNode *> m_visited;
            ASTPreorderTraversalCallback m_callback;

            ASTTraversalState(ASTPreorderTraversalCallback callback, const std::vector<std::string> &_namespace) : m_prefix(_namespace), m_visited(), m_callback(callback) {}
        };

        class ASTPreorderTraversal
        {
            static size_t ExprStmtNode_iter(ASTTraversalState &state, ExprStmtNode *node);
            static size_t NopStmtNode_iter(ASTTraversalState &state, NopStmtNode *node);
            static size_t BlockNode_iter(ASTTraversalState &state, BlockNode *node);
            static size_t StmtGroupNode_iter(ASTTraversalState &state, StmtGroupNode *node);
            static size_t CastExprNode_iter(ASTTraversalState &state, CastExprNode *node);
            static size_t StaticCastExprNode_iter(ASTTraversalState &state, StaticCastExprNode *node);
            static size_t BitCastExprNode_iter(ASTTraversalState &state, BitCastExprNode *node);
            static size_t SignedUpcastExprNode_iter(ASTTraversalState &state, SignedUpcastExprNode *node);
            static size_t UnsignedUpcastExprNode_iter(ASTTraversalState &state, UnsignedUpcastExprNode *node);
            static size_t DowncastExprNode_iter(ASTTraversalState &state, DowncastExprNode *node);
            static size_t PtrToIntCastExprNode_iter(ASTTraversalState &state, PtrToIntCastExprNode *node);
            static size_t IntToPtrCastExprNode_iter(ASTTraversalState &state, IntToPtrCastExprNode *node);
            static size_t UnaryExprNode_iter(ASTTraversalState &state, UnaryExprNode *node);
            static size_t BinaryExprNode_iter(ASTTraversalState &state, BinaryExprNode *node);
            static size_t CallExprNode_iter(ASTTraversalState &state, CallExprNode *node);
            static size_t ListExprNode_iter(ASTTraversalState &state, ListExprNode *node);
            static size_t MemberAccessNode_iter(ASTTraversalState &state, MemberAccessNode *node);
            static size_t ConstUnaryExprNode_iter(ASTTraversalState &state, ConstUnaryExprNode *node);
            static size_t ConstBinaryExprNode_iter(ASTTraversalState &state, ConstBinaryExprNode *node);
            static size_t IdentifierNode_iter(ASTTraversalState &state, IdentifierNode *node);
            static size_t MutTypeNode_iter(ASTTraversalState &state, MutTypeNode *node);
            static size_t U8TypeNode_iter(ASTTraversalState &state, U8TypeNode *node);
            static size_t U16TypeNode_iter(ASTTraversalState &state, U16TypeNode *node);
            static size_t U32TypeNode_iter(ASTTraversalState &state, U32TypeNode *node);
            static size_t U64TypeNode_iter(ASTTraversalState &state, U64TypeNode *node);
            static size_t I8TypeNode_iter(ASTTraversalState &state, I8TypeNode *node);
            static size_t I16TypeNode_iter(ASTTraversalState &state, I16TypeNode *node);
            static size_t I32TypeNode_iter(ASTTraversalState &state, I32TypeNode *node);
            static size_t I64TypeNode_iter(ASTTraversalState &state, I64TypeNode *node);
            static size_t F32TypeNode_iter(ASTTraversalState &state, F32TypeNode *node);
            static size_t F64TypeNode_iter(ASTTraversalState &state, F64TypeNode *node);
            static size_t BoolTypeNode_iter(ASTTraversalState &state, BoolTypeNode *node);
            static size_t VoidTypeNode_iter(ASTTraversalState &state, VoidTypeNode *node);
            static size_t PointerTypeNode_iter(ASTTraversalState &state, PointerTypeNode *node);
            static size_t OpaqueTypeNode_iter(ASTTraversalState &state, OpaqueTypeNode *node);
            static size_t StringTypeNode_iter(ASTTraversalState &state, StringTypeNode *node);
            static size_t EnumTypeNode_iter(ASTTraversalState &state, EnumTypeNode *node);
            static size_t StructTypeNode_iter(ASTTraversalState &state, StructTypeNode *node);
            static size_t RegionTypeNode_iter(ASTTraversalState &state, RegionTypeNode *node);
            static size_t UnionTypeNode_iter(ASTTraversalState &state, UnionTypeNode *node);
            static size_t ArrayTypeNode_iter(ASTTraversalState &state, ArrayTypeNode *node);
            static size_t FunctionTypeNode_iter(ASTTraversalState &state, FunctionTypeNode *node);
            static size_t UserTypeNode_iter(ASTTraversalState &state, UserTypeNode *node);
            static size_t IntegerNode_iter(ASTTraversalState &state, IntegerNode *node);
            static size_t FloatLiteralNode_iter(ASTTraversalState &state, FloatLiteralNode *node);
            static size_t StringNode_iter(ASTTraversalState &state, StringNode *node);
            static size_t CharNode_iter(ASTTraversalState &state, CharNode *node);
            static size_t BoolLiteralNode_iter(ASTTraversalState &state, BoolLiteralNode *node);
            static size_t NullLiteralNode_iter(ASTTraversalState &state, NullLiteralNode *node);
            static size_t TypedefNode_iter(ASTTraversalState &state, TypedefNode *node);
            static size_t VarDeclNode_iter(ASTTraversalState &state, VarDeclNode *node);
            static size_t LetDeclNode_iter(ASTTraversalState &state, LetDeclNode *node);
            static size_t FunctionDeclNode_iter(ASTTraversalState &state, FunctionDeclNode *node);
            static size_t StructDefNode_iter(ASTTraversalState &state, StructDefNode *node);
            static size_t StructFieldNode_iter(ASTTraversalState &state, StructFieldNode *node);
            static size_t RegionDefNode_iter(ASTTraversalState &state, RegionDefNode *node);
            static size_t RegionFieldNode_iter(ASTTraversalState &state, RegionFieldNode *node);
            static size_t GroupDefNode_iter(ASTTraversalState &state, GroupDefNode *node);
            static size_t GroupFieldNode_iter(ASTTraversalState &state, GroupFieldNode *node);
            static size_t UnionDefNode_iter(ASTTraversalState &state, UnionDefNode *node);
            static size_t UnionFieldNode_iter(ASTTraversalState &state, UnionFieldNode *node);
            static size_t EnumDefNode_iter(ASTTraversalState &state, EnumDefNode *node);
            static size_t EnumFieldNode_iter(ASTTraversalState &state, EnumFieldNode *node);
            static size_t FunctionDefNode_iter(ASTTraversalState &state, FunctionDefNode *node);
            static size_t FunctionParamNode_iter(ASTTraversalState &state, FunctionParamNode *node);
            static size_t SubsystemNode_iter(ASTTraversalState &state, SubsystemNode *node);
            static size_t ExportNode_iter(ASTTraversalState &state, ExportNode *node);
            static size_t InlineAsmNode_iter(ASTTraversalState &state, InlineAsmNode *node);
            static size_t ReturnStmtNode_iter(ASTTraversalState &state, ReturnStmtNode *node);
            static size_t RetifStmtNode_iter(ASTTraversalState &state, RetifStmtNode *node);
            static size_t RetzStmtNode_iter(ASTTraversalState &state, RetzStmtNode *node);
            static size_t RetvStmtNode_iter(ASTTraversalState &state, RetvStmtNode *node);
            static size_t IfStmtNode_iter(ASTTraversalState &state, IfStmtNode *node);
            static size_t WhileStmtNode_iter(ASTTraversalState &state, WhileStmtNode *node);
            static size_t ForStmtNode_iter(ASTTraversalState &state, ForStmtNode *node);

            static size_t dispatch(ASTTraversalState &state, libquixcc::ParseNode *node);

        public:
            template <typename T>
            static size_t next(ASTTraversalState &state, T node)
            {
                if constexpr (std::is_pointer<T>::value)
                {
                    return dispatch(state, node);
                }
                else
                {
                    return dispatch(state, node.get());
                }
            }
        };
    }
}

#endif // __QUIXCC_TRAVERSAL_AST_H__