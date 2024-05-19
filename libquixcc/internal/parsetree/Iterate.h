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

#ifndef __QUIXCC_PARSETREE_ITERATE_H__
#define __QUIXCC_PARSETREE_ITERATE_H__

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

        typedef std::function<void(const std::vector<std::string> &_namespace, libquixcc::ParseNode *, TraversePtr)> ParseTreePreorderCallback;

        struct ParseTreeTraversalState
        {
            std::vector<std::string> m_prefix;
            std::set<TypeNode *> m_visited;
            ParseTreePreorderCallback m_callback;

            ParseTreeTraversalState(ParseTreePreorderCallback callback, const std::vector<std::string> &_namespace) : m_prefix(_namespace), m_visited(), m_callback(callback) {}
        };

        class ParseTreePreorder
        {
            static size_t ExprStmtNode_iter(ParseTreeTraversalState &state, ExprStmtNode *node);
            static size_t NopStmtNode_iter(ParseTreeTraversalState &state, NopStmtNode *node);
            static size_t BlockNode_iter(ParseTreeTraversalState &state, BlockNode *node);
            static size_t StmtGroupNode_iter(ParseTreeTraversalState &state, StmtGroupNode *node);
            static size_t CastExprNode_iter(ParseTreeTraversalState &state, CastExprNode *node);
            static size_t StaticCastExprNode_iter(ParseTreeTraversalState &state, StaticCastExprNode *node);
            static size_t BitCastExprNode_iter(ParseTreeTraversalState &state, BitCastExprNode *node);
            static size_t SignedUpcastExprNode_iter(ParseTreeTraversalState &state, SignedUpcastExprNode *node);
            static size_t UnsignedUpcastExprNode_iter(ParseTreeTraversalState &state, UnsignedUpcastExprNode *node);
            static size_t DowncastExprNode_iter(ParseTreeTraversalState &state, DowncastExprNode *node);
            static size_t PtrToIntCastExprNode_iter(ParseTreeTraversalState &state, PtrToIntCastExprNode *node);
            static size_t IntToPtrCastExprNode_iter(ParseTreeTraversalState &state, IntToPtrCastExprNode *node);
            static size_t UnaryExprNode_iter(ParseTreeTraversalState &state, UnaryExprNode *node);
            static size_t BinaryExprNode_iter(ParseTreeTraversalState &state, BinaryExprNode *node);
            static size_t CallExprNode_iter(ParseTreeTraversalState &state, CallExprNode *node);
            static size_t ListExprNode_iter(ParseTreeTraversalState &state, ListExprNode *node);
            static size_t MemberAccessNode_iter(ParseTreeTraversalState &state, MemberAccessNode *node);
            static size_t ConstUnaryExprNode_iter(ParseTreeTraversalState &state, ConstUnaryExprNode *node);
            static size_t ConstBinaryExprNode_iter(ParseTreeTraversalState &state, ConstBinaryExprNode *node);
            static size_t IdentifierNode_iter(ParseTreeTraversalState &state, IdentifierNode *node);
            static size_t MutTypeNode_iter(ParseTreeTraversalState &state, MutTypeNode *node);
            static size_t U8TypeNode_iter(ParseTreeTraversalState &state, U8TypeNode *node);
            static size_t U16TypeNode_iter(ParseTreeTraversalState &state, U16TypeNode *node);
            static size_t U32TypeNode_iter(ParseTreeTraversalState &state, U32TypeNode *node);
            static size_t U64TypeNode_iter(ParseTreeTraversalState &state, U64TypeNode *node);
            static size_t I8TypeNode_iter(ParseTreeTraversalState &state, I8TypeNode *node);
            static size_t I16TypeNode_iter(ParseTreeTraversalState &state, I16TypeNode *node);
            static size_t I32TypeNode_iter(ParseTreeTraversalState &state, I32TypeNode *node);
            static size_t I64TypeNode_iter(ParseTreeTraversalState &state, I64TypeNode *node);
            static size_t F32TypeNode_iter(ParseTreeTraversalState &state, F32TypeNode *node);
            static size_t F64TypeNode_iter(ParseTreeTraversalState &state, F64TypeNode *node);
            static size_t BoolTypeNode_iter(ParseTreeTraversalState &state, BoolTypeNode *node);
            static size_t VoidTypeNode_iter(ParseTreeTraversalState &state, VoidTypeNode *node);
            static size_t PointerTypeNode_iter(ParseTreeTraversalState &state, PointerTypeNode *node);
            static size_t OpaqueTypeNode_iter(ParseTreeTraversalState &state, OpaqueTypeNode *node);
            static size_t StringTypeNode_iter(ParseTreeTraversalState &state, StringTypeNode *node);
            static size_t EnumTypeNode_iter(ParseTreeTraversalState &state, EnumTypeNode *node);
            static size_t StructTypeNode_iter(ParseTreeTraversalState &state, StructTypeNode *node);
            static size_t RegionTypeNode_iter(ParseTreeTraversalState &state, RegionTypeNode *node);
            static size_t UnionTypeNode_iter(ParseTreeTraversalState &state, UnionTypeNode *node);
            static size_t ArrayTypeNode_iter(ParseTreeTraversalState &state, ArrayTypeNode *node);
            static size_t FunctionTypeNode_iter(ParseTreeTraversalState &state, FunctionTypeNode *node);
            static size_t UserTypeNode_iter(ParseTreeTraversalState &state, UserTypeNode *node);
            static size_t IntegerNode_iter(ParseTreeTraversalState &state, IntegerNode *node);
            static size_t FloatLiteralNode_iter(ParseTreeTraversalState &state, FloatLiteralNode *node);
            static size_t StringNode_iter(ParseTreeTraversalState &state, StringNode *node);
            static size_t CharNode_iter(ParseTreeTraversalState &state, CharNode *node);
            static size_t BoolLiteralNode_iter(ParseTreeTraversalState &state, BoolLiteralNode *node);
            static size_t NullLiteralNode_iter(ParseTreeTraversalState &state, NullLiteralNode *node);
            static size_t TypedefNode_iter(ParseTreeTraversalState &state, TypedefNode *node);
            static size_t VarDeclNode_iter(ParseTreeTraversalState &state, VarDeclNode *node);
            static size_t LetDeclNode_iter(ParseTreeTraversalState &state, LetDeclNode *node);
            static size_t ConstDeclNode_iter(ParseTreeTraversalState &state, ConstDeclNode *node);
            static size_t FunctionDeclNode_iter(ParseTreeTraversalState &state, FunctionDeclNode *node);
            static size_t StructDefNode_iter(ParseTreeTraversalState &state, StructDefNode *node);
            static size_t StructFieldNode_iter(ParseTreeTraversalState &state, StructFieldNode *node);
            static size_t RegionDefNode_iter(ParseTreeTraversalState &state, RegionDefNode *node);
            static size_t RegionFieldNode_iter(ParseTreeTraversalState &state, RegionFieldNode *node);
            static size_t GroupDefNode_iter(ParseTreeTraversalState &state, GroupDefNode *node);
            static size_t GroupFieldNode_iter(ParseTreeTraversalState &state, GroupFieldNode *node);
            static size_t UnionDefNode_iter(ParseTreeTraversalState &state, UnionDefNode *node);
            static size_t UnionFieldNode_iter(ParseTreeTraversalState &state, UnionFieldNode *node);
            static size_t EnumDefNode_iter(ParseTreeTraversalState &state, EnumDefNode *node);
            static size_t EnumFieldNode_iter(ParseTreeTraversalState &state, EnumFieldNode *node);
            static size_t FunctionDefNode_iter(ParseTreeTraversalState &state, FunctionDefNode *node);
            static size_t FunctionParamNode_iter(ParseTreeTraversalState &state, FunctionParamNode *node);
            static size_t SubsystemNode_iter(ParseTreeTraversalState &state, SubsystemNode *node);
            static size_t ExportNode_iter(ParseTreeTraversalState &state, ExportNode *node);
            static size_t InlineAsmNode_iter(ParseTreeTraversalState &state, InlineAsmNode *node);
            static size_t ReturnStmtNode_iter(ParseTreeTraversalState &state, ReturnStmtNode *node);
            static size_t RetifStmtNode_iter(ParseTreeTraversalState &state, RetifStmtNode *node);
            static size_t RetzStmtNode_iter(ParseTreeTraversalState &state, RetzStmtNode *node);
            static size_t RetvStmtNode_iter(ParseTreeTraversalState &state, RetvStmtNode *node);
            static size_t IfStmtNode_iter(ParseTreeTraversalState &state, IfStmtNode *node);
            static size_t WhileStmtNode_iter(ParseTreeTraversalState &state, WhileStmtNode *node);
            static size_t ForStmtNode_iter(ParseTreeTraversalState &state, ForStmtNode *node);

            static size_t dispatch(ParseTreeTraversalState &state, libquixcc::ParseNode *node);

        public:
            template <typename T>
            static size_t next(ParseTreeTraversalState &state, T node)
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

#endif // __QUIXCC_PARSETREE_ITERATE_H__