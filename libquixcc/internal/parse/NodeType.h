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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_PARSE_NODES_NODETYPES_H__
#define __QUIXCC_PARSE_NODES_NODETYPES_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <cstdint>

namespace libquixcc
{
    enum class NodeType : uint8_t
    {
        ParseNode,
        ASTNopNode,
        ExprNode,
        ConstExprNode,
        StmtNode,
        ExprStmtNode,
        NopStmtNode,
        TypeNode,
        DeclNode,
        DefNode,
        BlockNode,
        BasicTypeNode,

        ConstUnaryExprNode,
        ConstBinaryExprNode,

        UnaryExprNode,
        BinaryExprNode,
        CallExprNode,

        IdentifierNode,

        U8TypeNode,
        U16TypeNode,
        U32TypeNode,
        U64TypeNode,
        I8TypeNode,
        I16TypeNode,
        I32TypeNode,
        I64TypeNode,
        F32TypeNode,
        F64TypeNode,
        BoolTypeNode,
        VoidTypeNode,
        PointerTypeNode,
        StringTypeNode,

        EnumTypeNode,
        StructTypeNode,
        UnionTypeNode,
        ArrayTypeNode,
        FunctionTypeNode,
        UserTypeNode,

        LiteralNode,
        IntegerLiteralNode,
        FloatLiteralNode,
        StringLiteralNode,
        CharLiteralNode,
        BoolLiteralNode,
        NullLiteralNode,

        TypedefNode,

        VarDeclNode,
        LetDeclNode,

        StructDeclNode,
        UnionDeclNode,
        EnumDeclNode,
        FunctionDeclNode,

        StructDefNode,
        StructFieldNode,
        UnionDefNode,
        UnionFieldNode,
        EnumDefNode,
        EnumFieldNode,
        FunctionDefNode,
        FunctionParamNode,

        SubsystemNode,
        ExportNode,

        ReturnStmtNode,
        RetifStmtNode,
        RetzStmtNode,
        RetvStmtNode,
        IfStmtNode,
        WhileStmtNode,
        ForStmtNode,
    };

    class ParseNode;
    class ASTNopNode;
    class ExprNode;
    class ConstExprNode;
    class StmtNode;
    class ExprStmtNode;
    class NopStmtNode;
    class TypeNode;
    class DeclNode;
    class DefNode;
    class BlockNode;
    class BasicTypeNode;
    class UnaryExprNode;
    class BinaryExprNode;
    class CallExprNode;
    class ConstUnaryExprNode;
    class ConstBinaryExprNode;
    class IdentifierNode;
    class U8TypeNode;
    class U16TypeNode;
    class U32TypeNode;
    class U64TypeNode;
    class I8TypeNode;
    class I16TypeNode;
    class I32TypeNode;
    class I64TypeNode;
    class F32TypeNode;
    class F64TypeNode;
    class BoolTypeNode;
    class VoidTypeNode;
    class PointerTypeNode;
    class StringTypeNode;
    class EnumTypeNode;
    class StructTypeNode;
    class UnionTypeNode;
    class ArrayTypeNode;
    class FunctionTypeNode;
    class UserTypeNode;
    class LiteralNode;
    class IntegerLiteralNode;
    class FloatLiteralNode;
    class StringLiteralNode;
    class CharLiteralNode;
    class BoolLiteralNode;
    class NullLiteralNode;
    class TypedefNode;
    class VarDeclNode;
    class LetDeclNode;
    class StructDeclNode;
    class UnionDeclNode;
    class EnumDeclNode;
    class FunctionDeclNode;
    class StructDefNode;
    class StructFieldNode;
    class UnionDefNode;
    class UnionFieldNode;
    class EnumDefNode;
    class EnumFieldNode;
    class FunctionDefNode;
    class FunctionParamNode;
    class SubsystemNode;
    class ExportNode;
    class ReturnStmtNode;
    class RetifStmtNode;
    class RetzStmtNode;
    class RetvStmtNode;
    class IfStmtNode;
    class WhileStmtNode;
    class ForStmtNode;
}

#endif // __QUIXCC_PARSE_NODES_NODETYPES_H__