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

#ifndef __QUIXCC_PARSE_NODES_JSON_SERIALIZE_H__
#define __QUIXCC_PARSE_NODES_JSON_SERIALIZE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <set>

#include <parse/NodeType.h>

namespace libquixcc
{
    class ParseNodeJsonSerializerVisitor
    {
        std::set<const TypeNode *> m_visited;

    public:
        std::string visit(const ASTNopNode *node);
        std::string visit(const ExprStmtNode *node);
        std::string visit(const NopStmtNode *node);
        std::string visit(const BlockNode *node);
        std::string visit(const StmtGroupNode *node);
        std::string visit(const StaticCastExprNode *node);
        std::string visit(const BitCastExprNode *node);
        std::string visit(const SignedUpcastExprNode *node);
        std::string visit(const UnsignedUpcastExprNode *node);
        std::string visit(const DowncastExprNode *node);
        std::string visit(const PtrToIntCastExprNode *node);
        std::string visit(const IntToPtrCastExprNode *node);
        std::string visit(const UnaryExprNode *node);
        std::string visit(const BinaryExprNode *node);
        std::string visit(const CallExprNode *node);
        std::string visit(const ListExprNode *node);
        std::string visit(const MemberAccessNode *node);
        std::string visit(const ConstUnaryExprNode *node);
        std::string visit(const ConstBinaryExprNode *node);
        std::string visit(const IdentifierNode *node);
        std::string visit(const MutTypeNode *node);
        std::string visit(const U8TypeNode *node);
        std::string visit(const U16TypeNode *node);
        std::string visit(const U32TypeNode *node);
        std::string visit(const U64TypeNode *node);
        std::string visit(const I8TypeNode *node);
        std::string visit(const I16TypeNode *node);
        std::string visit(const I32TypeNode *node);
        std::string visit(const I64TypeNode *node);
        std::string visit(const F32TypeNode *node);
        std::string visit(const F64TypeNode *node);
        std::string visit(const BoolTypeNode *node);
        std::string visit(const VoidTypeNode *node);
        std::string visit(const PointerTypeNode *node);
        std::string visit(const OpaqueTypeNode *node);
        std::string visit(const StringTypeNode *node);
        std::string visit(const EnumTypeNode *node);
        std::string visit(const StructTypeNode *node);
        std::string visit(const RegionTypeNode *node);
        std::string visit(const UnionTypeNode *node);
        std::string visit(const ArrayTypeNode *node);
        std::string visit(const FunctionTypeNode *node);
        std::string visit(const UserTypeNode *node);
        std::string visit(const IntegerLiteralNode *node);
        std::string visit(const FloatLiteralNode *node);
        std::string visit(const StringLiteralNode *node);
        std::string visit(const CharLiteralNode *node);
        std::string visit(const BoolLiteralNode *node);
        std::string visit(const NullLiteralNode *node);
        std::string visit(const TypedefNode *node);
        std::string visit(const VarDeclNode *node);
        std::string visit(const LetDeclNode *node);
        std::string visit(const FunctionDeclNode *node);
        std::string visit(const StructDefNode *node);
        std::string visit(const StructFieldNode *node);
        std::string visit(const RegionDefNode *node);
        std::string visit(const RegionFieldNode *node);
        std::string visit(const GroupDefNode *node);
        std::string visit(const GroupFieldNode *node);
        std::string visit(const UnionDefNode *node);
        std::string visit(const UnionFieldNode *node);
        std::string visit(const EnumDefNode *node);
        std::string visit(const EnumFieldNode *node);
        std::string visit(const FunctionDefNode *node);
        std::string visit(const FunctionParamNode *node);
        std::string visit(const SubsystemNode *node);
        std::string visit(const ExportNode *node);
        std::string visit(const InlineAsmNode *node);
        std::string visit(const ReturnStmtNode *node);
        std::string visit(const RetifStmtNode *node);
        std::string visit(const RetzStmtNode *node);
        std::string visit(const RetvStmtNode *node);
        std::string visit(const IfStmtNode *node);
        std::string visit(const WhileStmtNode *node);
        std::string visit(const ForStmtNode *node);
    };
}

#endif // __QUIXCC_PARSE_NODES_JSON_SERIALIZE_H__