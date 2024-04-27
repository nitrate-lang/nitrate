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

#ifndef __QUIXCC_SERIALIZE_AST_JSON_H__
#define __QUIXCC_SERIALIZE_AST_JSON_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <set>
#include <memory>

#include <parse/NodeType.h>

namespace libquixcc
{
    namespace serialize
    {
        struct ASTJsonSerializerState
        {
            std::set<const TypeNode *> m_visited;
        };

        class ASTJsonSerializer
        {
            static std::string ASTNopNode_conv(ASTJsonSerializerState &state, const ASTNopNode *node);
            static std::string ExprStmtNode_conv(ASTJsonSerializerState &state, const ExprStmtNode *node);
            static std::string NopStmtNode_conv(ASTJsonSerializerState &state, const NopStmtNode *node);
            static std::string BlockNode_conv(ASTJsonSerializerState &state, const BlockNode *node);
            static std::string StmtGroupNode_conv(ASTJsonSerializerState &state, const StmtGroupNode *node);
            static std::string StaticCastExprNode_conv(ASTJsonSerializerState &state, const StaticCastExprNode *node);
            static std::string BitCastExprNode_conv(ASTJsonSerializerState &state, const BitCastExprNode *node);
            static std::string SignedUpcastExprNode_conv(ASTJsonSerializerState &state, const SignedUpcastExprNode *node);
            static std::string UnsignedUpcastExprNode_conv(ASTJsonSerializerState &state, const UnsignedUpcastExprNode *node);
            static std::string DowncastExprNode_conv(ASTJsonSerializerState &state, const DowncastExprNode *node);
            static std::string PtrToIntCastExprNode_conv(ASTJsonSerializerState &state, const PtrToIntCastExprNode *node);
            static std::string IntToPtrCastExprNode_conv(ASTJsonSerializerState &state, const IntToPtrCastExprNode *node);
            static std::string UnaryExprNode_conv(ASTJsonSerializerState &state, const UnaryExprNode *node);
            static std::string BinaryExprNode_conv(ASTJsonSerializerState &state, const BinaryExprNode *node);
            static std::string CallExprNode_conv(ASTJsonSerializerState &state, const CallExprNode *node);
            static std::string ListExprNode_conv(ASTJsonSerializerState &state, const ListExprNode *node);
            static std::string MemberAccessNode_conv(ASTJsonSerializerState &state, const MemberAccessNode *node);
            static std::string ConstUnaryExprNode_conv(ASTJsonSerializerState &state, const ConstUnaryExprNode *node);
            static std::string ConstBinaryExprNode_conv(ASTJsonSerializerState &state, const ConstBinaryExprNode *node);
            static std::string IdentifierNode_conv(ASTJsonSerializerState &state, const IdentifierNode *node);
            static std::string MutTypeNode_conv(ASTJsonSerializerState &state, const MutTypeNode *node);
            static std::string U8TypeNode_conv(ASTJsonSerializerState &state, const U8TypeNode *node);
            static std::string U16TypeNode_conv(ASTJsonSerializerState &state, const U16TypeNode *node);
            static std::string U32TypeNode_conv(ASTJsonSerializerState &state, const U32TypeNode *node);
            static std::string U64TypeNode_conv(ASTJsonSerializerState &state, const U64TypeNode *node);
            static std::string I8TypeNode_conv(ASTJsonSerializerState &state, const I8TypeNode *node);
            static std::string I16TypeNode_conv(ASTJsonSerializerState &state, const I16TypeNode *node);
            static std::string I32TypeNode_conv(ASTJsonSerializerState &state, const I32TypeNode *node);
            static std::string I64TypeNode_conv(ASTJsonSerializerState &state, const I64TypeNode *node);
            static std::string F32TypeNode_conv(ASTJsonSerializerState &state, const F32TypeNode *node);
            static std::string F64TypeNode_conv(ASTJsonSerializerState &state, const F64TypeNode *node);
            static std::string BoolTypeNode_conv(ASTJsonSerializerState &state, const BoolTypeNode *node);
            static std::string VoidTypeNode_conv(ASTJsonSerializerState &state, const VoidTypeNode *node);
            static std::string PointerTypeNode_conv(ASTJsonSerializerState &state, const PointerTypeNode *node);
            static std::string OpaqueTypeNode_conv(ASTJsonSerializerState &state, const OpaqueTypeNode *node);
            static std::string StringTypeNode_conv(ASTJsonSerializerState &state, const StringTypeNode *node);
            static std::string EnumTypeNode_conv(ASTJsonSerializerState &state, const EnumTypeNode *node);
            static std::string StructTypeNode_conv(ASTJsonSerializerState &state, const StructTypeNode *node);
            static std::string RegionTypeNode_conv(ASTJsonSerializerState &state, const RegionTypeNode *node);
            static std::string UnionTypeNode_conv(ASTJsonSerializerState &state, const UnionTypeNode *node);
            static std::string ArrayTypeNode_conv(ASTJsonSerializerState &state, const ArrayTypeNode *node);
            static std::string FunctionTypeNode_conv(ASTJsonSerializerState &state, const FunctionTypeNode *node);
            static std::string UserTypeNode_conv(ASTJsonSerializerState &state, const UserTypeNode *node);
            static std::string IntegerLiteralNode_conv(ASTJsonSerializerState &state, const IntegerLiteralNode *node);
            static std::string FloatLiteralNode_conv(ASTJsonSerializerState &state, const FloatLiteralNode *node);
            static std::string StringLiteralNode_conv(ASTJsonSerializerState &state, const StringLiteralNode *node);
            static std::string CharLiteralNode_conv(ASTJsonSerializerState &state, const CharLiteralNode *node);
            static std::string BoolLiteralNode_conv(ASTJsonSerializerState &state, const BoolLiteralNode *node);
            static std::string NullLiteralNode_conv(ASTJsonSerializerState &state, const NullLiteralNode *node);
            static std::string TypedefNode_conv(ASTJsonSerializerState &state, const TypedefNode *node);
            static std::string VarDeclNode_conv(ASTJsonSerializerState &state, const VarDeclNode *node);
            static std::string LetDeclNode_conv(ASTJsonSerializerState &state, const LetDeclNode *node);
            static std::string FunctionDeclNode_conv(ASTJsonSerializerState &state, const FunctionDeclNode *node);
            static std::string StructDefNode_conv(ASTJsonSerializerState &state, const StructDefNode *node);
            static std::string StructFieldNode_conv(ASTJsonSerializerState &state, const StructFieldNode *node);
            static std::string RegionDefNode_conv(ASTJsonSerializerState &state, const RegionDefNode *node);
            static std::string RegionFieldNode_conv(ASTJsonSerializerState &state, const RegionFieldNode *node);
            static std::string GroupDefNode_conv(ASTJsonSerializerState &state, const GroupDefNode *node);
            static std::string GroupFieldNode_conv(ASTJsonSerializerState &state, const GroupFieldNode *node);
            static std::string UnionDefNode_conv(ASTJsonSerializerState &state, const UnionDefNode *node);
            static std::string UnionFieldNode_conv(ASTJsonSerializerState &state, const UnionFieldNode *node);
            static std::string EnumDefNode_conv(ASTJsonSerializerState &state, const EnumDefNode *node);
            static std::string EnumFieldNode_conv(ASTJsonSerializerState &state, const EnumFieldNode *node);
            static std::string FunctionDefNode_conv(ASTJsonSerializerState &state, const FunctionDefNode *node);
            static std::string FunctionParamNode_conv(ASTJsonSerializerState &state, const FunctionParamNode *node);
            static std::string SubsystemNode_conv(ASTJsonSerializerState &state, const SubsystemNode *node);
            static std::string ExportNode_conv(ASTJsonSerializerState &state, const ExportNode *node);
            static std::string InlineAsmNode_conv(ASTJsonSerializerState &state, const InlineAsmNode *node);
            static std::string ReturnStmtNode_conv(ASTJsonSerializerState &state, const ReturnStmtNode *node);
            static std::string RetifStmtNode_conv(ASTJsonSerializerState &state, const RetifStmtNode *node);
            static std::string RetzStmtNode_conv(ASTJsonSerializerState &state, const RetzStmtNode *node);
            static std::string RetvStmtNode_conv(ASTJsonSerializerState &state, const RetvStmtNode *node);
            static std::string IfStmtNode_conv(ASTJsonSerializerState &state, const IfStmtNode *node);
            static std::string WhileStmtNode_conv(ASTJsonSerializerState &state, const WhileStmtNode *node);
            static std::string ForStmtNode_conv(ASTJsonSerializerState &state, const ForStmtNode *node);

            static std::string dispatch(ASTJsonSerializerState &state, const ParseNode *node);

        public:
            template <typename T>
            static std::string next(ASTJsonSerializerState &state, const T node)
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

#endif // __QUIXCC_SERIALIZE_AST_JSON_H__