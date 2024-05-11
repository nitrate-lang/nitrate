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

#include <parse/NodeType.h>

std::unordered_map<libquixcc::NodeType, std::string_view> libquixcc::NodeTypeNames = {
    {NodeType::ParseNode, "ParseNode"},
    {NodeType::ASTNopNode, "ASTNopNode"},
    {NodeType::ExprNode, "ExprNode"},
    {NodeType::ConstExprNode, "ConstExprNode"},
    {NodeType::StmtNode, "StmtNode"},
    {NodeType::ExprStmtNode, "ExprStmtNode"},
    {NodeType::NopStmtNode, "NopStmtNode"},
    {NodeType::TypeNode, "TypeNode"},
    {NodeType::DeclNode, "DeclNode"},
    {NodeType::DefNode, "DefNode"},
    {NodeType::BlockNode, "BlockNode"},
    {NodeType::StmtGroupNode, "StmtGroupNode"},
    {NodeType::CastExprNode, "CastExprNode"},
    {NodeType::StaticCastExprNode, "StaticCastExprNode"},
    {NodeType::BitCastExprNode, "BitCastExprNode"},
    {NodeType::SignedUpcastExprNode, "SignedUpcastExprNode"},
    {NodeType::UnsignedUpcastExprNode, "UnsignedUpcastExprNode"},
    {NodeType::DowncastExprNode, "DowncastExprNode"},
    {NodeType::PtrToIntCastExprNode, "PtrToIntCastExprNode"},
    {NodeType::IntToPtrCastExprNode, "IntToPtrCastExprNode"},
    {NodeType::UnaryExprNode, "UnaryExprNode"},
    {NodeType::BinaryExprNode, "BinaryExprNode"},
    {NodeType::CallExprNode, "CallExprNode"},
    {NodeType::ListExprNode, "ListExprNode"},
    {NodeType::MemberAccessNode, "MemberAccessNode"},
    {NodeType::ConstUnaryExprNode, "ConstUnaryExprNode"},
    {NodeType::ConstBinaryExprNode, "ConstBinaryExprNode"},
    {NodeType::IdentifierNode, "IdentifierNode"},
    {NodeType::MutTypeNode, "MutTypeNode"},
    {NodeType::U8TypeNode, "U8TypeNode"},
    {NodeType::U16TypeNode, "U16TypeNode"},
    {NodeType::U32TypeNode, "U32TypeNode"},
    {NodeType::U64TypeNode, "U64TypeNode"},
    {NodeType::U128TypeNode, "U128TypeNode"},
    {NodeType::I8TypeNode, "I8TypeNode"},
    {NodeType::I16TypeNode, "I16TypeNode"},
    {NodeType::I32TypeNode, "I32TypeNode"},
    {NodeType::I64TypeNode, "I64TypeNode"},
    {NodeType::I128TypeNode, "I128TypeNode"},
    {NodeType::F32TypeNode, "F32TypeNode"},
    {NodeType::F64TypeNode, "F64TypeNode"},
    {NodeType::BoolTypeNode, "BoolTypeNode"},
    {NodeType::VoidTypeNode, "VoidTypeNode"},
    {NodeType::PointerTypeNode, "PointerTypeNode"},
    {NodeType::OpaqueTypeNode, "OpaqueTypeNode"},
    {NodeType::StringTypeNode, "StringTypeNode"},
    {NodeType::EnumTypeNode, "EnumTypeNode"},
    {NodeType::StructTypeNode, "StructTypeNode"},
    {NodeType::RegionTypeNode, "RegionTypeNode"},
    {NodeType::UnionTypeNode, "UnionTypeNode"},
    {NodeType::ArrayTypeNode, "ArrayTypeNode"},
    {NodeType::FunctionTypeNode, "FunctionTypeNode"},
    {NodeType::UserTypeNode, "UserTypeNode"},
    {NodeType::LiteralNode, "LiteralNode"},
    {NodeType::IntegerNode, "IntegerNode"},
    {NodeType::FloatLiteralNode, "FloatLiteralNode"},
    {NodeType::StringNode, "StringNode"},
    {NodeType::CharNode, "CharNode"},
    {NodeType::BoolLiteralNode, "BoolLiteralNode"},
    {NodeType::NullLiteralNode, "NullLiteralNode"},
    {NodeType::TypedefNode, "TypedefNode"},
    {NodeType::VarDeclNode, "VarDeclNode"},
    {NodeType::LetDeclNode, "LetDeclNode"},
    {NodeType::FunctionDeclNode, "FunctionDeclNode"},
    {NodeType::StructDefNode, "StructDefNode"},
    {NodeType::StructFieldNode, "StructFieldNode"},
    {NodeType::RegionDefNode, "RegionDefNode"},
    {NodeType::RegionFieldNode, "RegionFieldNode"},
    {NodeType::GroupDefNode, "GroupDefNode"},
    {NodeType::GroupFieldNode, "GroupFieldNode"},
    {NodeType::UnionDefNode, "UnionDefNode"},
    {NodeType::UnionFieldNode, "UnionFieldNode"},
    {NodeType::EnumDefNode, "EnumDefNode"},
    {NodeType::EnumFieldNode, "EnumFieldNode"},
    {NodeType::FunctionDefNode, "FunctionDefNode"},
    {NodeType::FunctionParamNode, "FunctionParamNode"},
    {NodeType::SubsystemNode, "SubsystemNode"},
    {NodeType::ExportNode, "ExportNode"},
    {NodeType::InlineAsmNode, "InlineAsmNode"},
    {NodeType::ReturnStmtNode, "ReturnStmtNode"},
    {NodeType::RetifStmtNode, "RetifStmtNode"},
    {NodeType::RetzStmtNode, "RetzStmtNode"},
    {NodeType::RetvStmtNode, "RetvStmtNode"},
    {NodeType::IfStmtNode, "IfStmtNode"},
    {NodeType::WhileStmtNode, "WhileStmtNode"},
    {NodeType::ForStmtNode, "ForStmtNode"},
};