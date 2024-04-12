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

#ifndef __QUIXCC_PARSE_NODES_TRAVERSAL_H__
#define __QUIXCC_PARSE_NODES_TRAVERSAL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <memory>
#include <functional>
#include <parse/NodeType.h>

namespace libquixcc
{
    typedef std::function<void(std::string _namespace, libquixcc::ParseNode *, std::shared_ptr<libquixcc::ParseNode> *)> ParseNodePreorderVisitorCallback;

    class ParseNodePreorderVisitor
    {
        ParseNodePreorderVisitorCallback m_callback;
        std::string m_prefix;

        void push_prefix(const std::string &name);

    public:
        ParseNodePreorderVisitor(ParseNodePreorderVisitorCallback callback, std::string _namespace) : m_callback(callback), m_prefix(_namespace) {}

        size_t visit(ASTNopNode *node);
        size_t visit(ExprStmtNode *node);
        size_t visit(NopStmtNode *node);
        size_t visit(BlockNode *node);
        size_t visit(StmtGroupNode *node);
        size_t visit(UnaryExprNode *node);
        size_t visit(BinaryExprNode *node);
        size_t visit(CallExprNode *node);
        size_t visit(ListExprNode *node);
        size_t visit(MemberAccessNode *node);
        size_t visit(ConstUnaryExprNode *node);
        size_t visit(ConstBinaryExprNode *node);
        size_t visit(IdentifierNode *node);
        size_t visit(U8TypeNode *node);
        size_t visit(U16TypeNode *node);
        size_t visit(U32TypeNode *node);
        size_t visit(U64TypeNode *node);
        size_t visit(I8TypeNode *node);
        size_t visit(I16TypeNode *node);
        size_t visit(I32TypeNode *node);
        size_t visit(I64TypeNode *node);
        size_t visit(F32TypeNode *node);
        size_t visit(F64TypeNode *node);
        size_t visit(BoolTypeNode *node);
        size_t visit(VoidTypeNode *node);
        size_t visit(PointerTypeNode *node);
        size_t visit(OpaqueTypeNode *node);
        size_t visit(StringTypeNode *node);
        size_t visit(EnumTypeNode *node);
        size_t visit(StructTypeNode *node);
        size_t visit(RegionTypeNode *node);
        size_t visit(UnionTypeNode *node);
        size_t visit(ArrayTypeNode *node);
        size_t visit(FunctionTypeNode *node);
        size_t visit(UserTypeNode *node);
        size_t visit(IntegerLiteralNode *node);
        size_t visit(FloatLiteralNode *node);
        size_t visit(StringLiteralNode *node);
        size_t visit(CharLiteralNode *node);
        size_t visit(BoolLiteralNode *node);
        size_t visit(NullLiteralNode *node);
        size_t visit(TypedefNode *node);
        size_t visit(VarDeclNode *node);
        size_t visit(LetDeclNode *node);
        size_t visit(FunctionDeclNode *node);
        size_t visit(StructDefNode *node);
        size_t visit(StructFieldNode *node);
        size_t visit(RegionDefNode *node);
        size_t visit(RegionFieldNode *node);
        size_t visit(GroupDefNode *node);
        size_t visit(GroupFieldNode *node);
        size_t visit(UnionDefNode *node);
        size_t visit(UnionFieldNode *node);
        size_t visit(EnumDefNode *node);
        size_t visit(EnumFieldNode *node);
        size_t visit(FunctionDefNode *node);
        size_t visit(FunctionParamNode *node);
        size_t visit(SubsystemNode *node);
        size_t visit(ExportNode *node);
        size_t visit(InlineAsmNode *node);
        size_t visit(ReturnStmtNode *node);
        size_t visit(RetifStmtNode *node);
        size_t visit(RetzStmtNode *node);
        size_t visit(RetvStmtNode *node);
        size_t visit(IfStmtNode *node);
        size_t visit(WhileStmtNode *node);
        size_t visit(ForStmtNode *node);
    };
}

#endif // __QUIXCC_PARSE_NODES_TRAVERSAL_H__