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

#define QUIXCC_INTERNAL

#include <parsetree/Serialize.h>
#include <parse/nodes/AllNodes.h>
#include <functional>
#include <unordered_map>
#include <lexer/Lex.h>

std::string escape_json(const std::string &input)
{
    std::string output;
    output.reserve(input.length() * 2); // Reserving enough space for escaped characters

    for (char ch : input)
    {
        switch (ch)
        {
        case '"':
            output += "\\\"";
            break;
        case '\\':
            output += "\\\\";
            break;
        case '\b':
            output += "\\b";
            break;
        case '\f':
            output += "\\f";
            break;
        case '\n':
            output += "\\n";
            break;
        case '\r':
            output += "\\r";
            break;
        case '\t':
            output += "\\t";
            break;
        default:
            if (ch < ' ')
            {
                // Unicode escape sequence
                char hex[7];
                snprintf(hex, sizeof(hex), "\\u%04x", ch);
                output += hex;
            }
            else
            {
                output += ch;
            }
            break;
        }
    }

    return output;
}

std::string libquixcc::ParseNode::to_json(libquixcc::serialize::ParseTreeSerializerState state) const
{
    return serialize::ParseTreeSerializer::next(state, this);
}

std::string libquixcc::serialize::ParseTreeSerializer::dispatch(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ParseNode *node)
{
    typedef std::string (*Func)(libquixcc::serialize::ParseTreeSerializerState &, const libquixcc::ParseNode *);

    static std::unordered_map<NodeType, Func> node_map =
        {
            {NodeType::ExprStmtNode, (Func)ExprStmtNode_conv},
            {NodeType::NopStmtNode, (Func)NopStmtNode_conv},
            {NodeType::BlockNode, (Func)BlockNode_conv},
            {NodeType::StmtGroupNode, (Func)StmtGroupNode_conv},
            {NodeType::StaticCastExprNode, (Func)StaticCastExprNode_conv},
            {NodeType::BitCastExprNode, (Func)BitCastExprNode_conv},
            {NodeType::SignedUpcastExprNode, (Func)SignedUpcastExprNode_conv},
            {NodeType::UnsignedUpcastExprNode, (Func)UnsignedUpcastExprNode_conv},
            {NodeType::DowncastExprNode, (Func)DowncastExprNode_conv},
            {NodeType::PtrToIntCastExprNode, (Func)PtrToIntCastExprNode_conv},
            {NodeType::IntToPtrCastExprNode, (Func)IntToPtrCastExprNode_conv},
            {NodeType::UnaryExprNode, (Func)UnaryExprNode_conv},
            {NodeType::BinaryExprNode, (Func)BinaryExprNode_conv},
            {NodeType::CallExprNode, (Func)CallExprNode_conv},
            {NodeType::ListExprNode, (Func)ListExprNode_conv},
            {NodeType::MemberAccessNode, (Func)MemberAccessNode_conv},
            {NodeType::ConstUnaryExprNode, (Func)ConstUnaryExprNode_conv},
            {NodeType::ConstBinaryExprNode, (Func)ConstBinaryExprNode_conv},
            {NodeType::IdentifierNode, (Func)IdentifierNode_conv},
            {NodeType::MutTypeNode, (Func)MutTypeNode_conv},
            {NodeType::U8TypeNode, (Func)U8TypeNode_conv},
            {NodeType::U16TypeNode, (Func)U16TypeNode_conv},
            {NodeType::U32TypeNode, (Func)U32TypeNode_conv},
            {NodeType::U64TypeNode, (Func)U64TypeNode_conv},
            {NodeType::I8TypeNode, (Func)I8TypeNode_conv},
            {NodeType::I16TypeNode, (Func)I16TypeNode_conv},
            {NodeType::I32TypeNode, (Func)I32TypeNode_conv},
            {NodeType::I64TypeNode, (Func)I64TypeNode_conv},
            {NodeType::F32TypeNode, (Func)F32TypeNode_conv},
            {NodeType::F64TypeNode, (Func)F64TypeNode_conv},
            {NodeType::BoolTypeNode, (Func)BoolTypeNode_conv},
            {NodeType::VoidTypeNode, (Func)VoidTypeNode_conv},
            {NodeType::PointerTypeNode, (Func)PointerTypeNode_conv},
            {NodeType::OpaqueTypeNode, (Func)OpaqueTypeNode_conv},
            {NodeType::StringTypeNode, (Func)StringTypeNode_conv},
            {NodeType::EnumTypeNode, (Func)EnumTypeNode_conv},
            {NodeType::StructTypeNode, (Func)StructTypeNode_conv},
            {NodeType::RegionTypeNode, (Func)RegionTypeNode_conv},
            {NodeType::UnionTypeNode, (Func)UnionTypeNode_conv},
            {NodeType::ArrayTypeNode, (Func)ArrayTypeNode_conv},
            {NodeType::FunctionTypeNode, (Func)FunctionTypeNode_conv},
            {NodeType::UserTypeNode, (Func)UserTypeNode_conv},
            {NodeType::IntegerNode, (Func)IntegerNode_conv},
            {NodeType::FloatLiteralNode, (Func)FloatLiteralNode_conv},
            {NodeType::StringNode, (Func)StringNode_conv},
            {NodeType::CharNode, (Func)CharNode_conv},
            {NodeType::BoolLiteralNode, (Func)BoolLiteralNode_conv},
            {NodeType::NullLiteralNode, (Func)NullLiteralNode_conv},
            {NodeType::TypedefNode, (Func)TypedefNode_conv},
            {NodeType::VarDeclNode, (Func)VarDeclNode_conv},
            {NodeType::LetDeclNode, (Func)LetDeclNode_conv},
            {NodeType::ConstDeclNode, (Func)ConstDeclNode_conv},
            {NodeType::FunctionDeclNode, (Func)FunctionDeclNode_conv},
            {NodeType::StructDefNode, (Func)StructDefNode_conv},
            {NodeType::StructFieldNode, (Func)StructFieldNode_conv},
            {NodeType::RegionDefNode, (Func)RegionDefNode_conv},
            {NodeType::RegionFieldNode, (Func)RegionFieldNode_conv},
            {NodeType::GroupDefNode, (Func)GroupDefNode_conv},
            {NodeType::GroupFieldNode, (Func)GroupFieldNode_conv},
            {NodeType::UnionDefNode, (Func)UnionDefNode_conv},
            {NodeType::UnionFieldNode, (Func)UnionFieldNode_conv},
            {NodeType::EnumDefNode, (Func)EnumDefNode_conv},
            {NodeType::EnumFieldNode, (Func)EnumFieldNode_conv},
            {NodeType::FunctionDefNode, (Func)FunctionDefNode_conv},
            {NodeType::FunctionParamNode, (Func)FunctionParamNode_conv},
            {NodeType::SubsystemNode, (Func)SubsystemNode_conv},
            {NodeType::ExportNode, (Func)ExportNode_conv},
            {NodeType::InlineAsmNode, (Func)InlineAsmNode_conv},
            {NodeType::ReturnStmtNode, (Func)ReturnStmtNode_conv},
            {NodeType::RetifStmtNode, (Func)RetifStmtNode_conv},
            {NodeType::RetzStmtNode, (Func)RetzStmtNode_conv},
            {NodeType::RetvStmtNode, (Func)RetvStmtNode_conv},
            {NodeType::IfStmtNode, (Func)IfStmtNode_conv},
            {NodeType::WhileStmtNode, (Func)WhileStmtNode_conv},
            {NodeType::ForStmtNode, (Func)ForStmtNode_conv},
        };

    if (!node_map.contains(node->ntype))
        LOG(FATAL) << "No conversion function for node type " << (int)node->ntype << " found." << std::endl;

    return node_map[node->ntype](state, node);
}

std::string libquixcc::serialize::ParseTreeSerializer::ExprStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ExprStmtNode *node)
{
    return "{\"ntype\":\"ExprStmtNode\",\"expr\":" + next(state, node->m_expr) + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::NopStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::NopStmtNode *node)
{
    return "{\"ntype\":\"nop\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::BlockNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::BlockNode *node)
{
    std::string str = "{\"ntype\":\"BlockNode\",\"children\":[";
    for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_stmts.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::StmtGroupNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::StmtGroupNode *node)
{
    std::string str;

    for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_stmts.end() - 1)
        {
            str += ",";
        }
    }

    return str;
}

std::string libquixcc::serialize::ParseTreeSerializer::StaticCastExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::StaticCastExprNode *node)
{
    std::string str = "{\"ntype\":\"StaticCastExprNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"expr\":";
    str += next(state, node->m_expr);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::BitCastExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::BitCastExprNode *node)
{
    std::string str = "{\"ntype\":\"BitCastExprNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"expr\":";
    str += next(state, node->m_expr);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::SignedUpcastExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::SignedUpcastExprNode *node)
{
    std::string str = "{\"ntype\":\"SignedUpcastExprNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"expr\":";
    str += next(state, node->m_expr);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::UnsignedUpcastExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::UnsignedUpcastExprNode *node)
{
    std::string str = "{\"ntype\":\"UnsignedUpcastExprNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"expr\":";
    str += next(state, node->m_expr);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::DowncastExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::DowncastExprNode *node)
{
    std::string str = "{\"ntype\":\"DowncastExprNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"expr\":";
    str += next(state, node->m_expr);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::PtrToIntCastExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::PtrToIntCastExprNode *node)
{
    std::string str = "{\"ntype\":\"PtrToIntCastExprNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"expr\":";
    str += next(state, node->m_expr);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::IntToPtrCastExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::IntToPtrCastExprNode *node)
{
    std::string str = "{\"ntype\":\"IntToPtrCastExprNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"expr\":";
    str += next(state, node->m_expr);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::UnaryExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::UnaryExprNode *node)
{
    std::string str = "{\"ntype\":\"UnaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"operand\":";
    str += next(state, node->m_expr);

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::BinaryExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::BinaryExprNode *node)
{
    std::string str = "{\"ntype\":\"BinaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"lhs\":";
    str += next(state, node->m_lhs);
    str += ",\"rhs\":";
    str += next(state, node->m_rhs);

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::CallExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::CallExprNode *node)
{
    std::string str = "{\"ntype\":\"CallExprNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"named_args\":[";
    for (auto it = node->m_named_args.begin(); it != node->m_named_args.end(); ++it)
    {
        str += "{\"name\":\"" + escape_json(it->first) + "\",\"value\":" + next(state, it->second) + "}";
        if (it != node->m_named_args.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"positional_args\":[";
    for (auto it = node->m_positional_args.begin(); it != node->m_positional_args.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_positional_args.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::ListExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ListExprNode *node)
{
    std::string str = "{\"ntype\":\"ListExprNode\",\"elements\":[";
    for (auto it = node->m_elements.begin(); it != node->m_elements.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_elements.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::MemberAccessNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::MemberAccessNode *node)
{
    std::string str = "{\"ntype\":\"MemberAccessNode\",\"lhs\":";
    str += next(state, node->m_expr);
    str += ",\"field\":\"" + escape_json(node->m_field) + "\"";

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::ConstUnaryExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ConstUnaryExprNode *node)
{
    std::string str = "{\"ntype\":\"ConstUnaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"operand\":";
    str += next(state, node->m_expr);

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::ConstBinaryExprNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ConstBinaryExprNode *node)
{
    std::string str = "{\"ntype\":\"ConstBinaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"lhs\":";
    str += next(state, node->m_lhs);
    str += ",\"rhs\":";
    str += next(state, node->m_rhs);

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::IdentifierNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::IdentifierNode *node)
{
    return "{\"ntype\":\"IdentifierNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::MutTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::MutTypeNode *node)
{
    return "{\"ntype\":\"MutTypeNode\",\"type\":" + next(state, node->m_type) + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::U8TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::U8TypeNode *node)
{
    return "{\"ntype\":\"U8TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::U16TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::U16TypeNode *node)
{
    return "{\"ntype\":\"U16TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::U32TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::U32TypeNode *node)
{
    return "{\"ntype\":\"U32TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::U64TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::U64TypeNode *node)
{
    return "{\"ntype\":\"U64TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::I8TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::I8TypeNode *node)
{
    return "{\"ntype\":\"I8TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::I16TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::I16TypeNode *node)
{
    return "{\"ntype\":\"I16TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::I32TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::I32TypeNode *node)
{
    return "{\"ntype\":\"I32TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::I64TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::I64TypeNode *node)
{
    return "{\"ntype\":\"I64TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::F32TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::F32TypeNode *node)
{
    return "{\"ntype\":\"F32TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::F64TypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::F64TypeNode *node)
{
    return "{\"ntype\":\"F64TypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::BoolTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::BoolTypeNode *node)
{
    return "{\"ntype\":\"BoolTypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::VoidTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::VoidTypeNode *node)
{
    return "{\"ntype\":\"VoidTypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::PointerTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::PointerTypeNode *node)
{
    std::string str = "{\"ntype\":\"PointerTypeNode\",\"type\":";
    str += next(state, node->m_type);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::OpaqueTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::OpaqueTypeNode *node)
{
    return "{\"ntype\":\"OpaqueTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::StringTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::StringTypeNode *node)
{
    return "{\"ntype\":\"StringTypeNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::EnumTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::EnumTypeNode *node)
{
    std::string str = "{\"ntype\":\"EnumTypeNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"member_type\":";
    str += next(state, node->m_member_type);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::StructTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::StructTypeNode *node)
{
    if (state.m_visited.contains(node))
        return "{\"ntype\":\"StructTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";

    state.m_visited.insert(node); // Prevent infinite recursion

    std::string str = "{\"ntype\":\"StructTypeNode\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"name\":\"" + escape_json(node->m_name) + "\"";

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::RegionTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::RegionTypeNode *node)
{
    if (state.m_visited.contains(node))
        return "{\"ntype\":\"RegionTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";

    state.m_visited.insert(node); // Prevent infinite recursion

    std::string str = "{\"ntype\":\"RegionTypeNode\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::UnionTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::UnionTypeNode *node)
{
    if (state.m_visited.contains(node))
        return "{\"ntype\":\"UnionTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";

    state.m_visited.insert(node); // Prevent infinite recursion

    std::string str = "{\"ntype\":\"UnionTypeNode\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::ArrayTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ArrayTypeNode *node)
{
    std::string str = "{\"ntype\":\"ArrayTypeNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"size\":";
    str += next(state, node->m_size);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::FunctionTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::FunctionTypeNode *node)
{
    std::string str = "{\"ntype\":\"FunctionTypeNode\",\"params\":[";
    for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it)
    {
        str += next(state, (*it).second);
        if (it != node->m_params.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"return_type\":";
    str += next(state, node->m_return_type);
    str += ",\"variadic\":";
    str += std::string(node->m_variadic ? "true" : "false");
    str += ",\"pure\":";
    str += std::string(node->m_pure ? "true" : "false");
    str += ",\"thread_safe\":";
    str += std::string(node->m_thread_safe ? "true" : "false");
    str += ",\"foreign\":";
    str += std::string(node->m_foreign ? "true" : "false");
    str += ",\"nothrow\":";
    str += std::string(node->m_nothrow ? "true" : "false");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::UserTypeNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::UserTypeNode *node)
{
    return "{\"ntype\":\"UserTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::IntegerNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::IntegerNode *node)
{
    return "{\"ntype\":\"IntegerNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::FloatLiteralNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::FloatLiteralNode *node)
{
    return "{\"ntype\":\"FloatLiteralNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::StringNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::StringNode *node)
{
    return "{\"ntype\":\"StringNode\",\"value\":\"" + escape_json(node->m_val) + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::CharNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::CharNode *node)
{
    return "{\"ntype\":\"CharNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::BoolLiteralNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::BoolLiteralNode *node)
{
    return "{\"ntype\":\"BoolLiteralNode\",\"value\":\"" + std::string(node->m_val ? "true" : "false") + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::NullLiteralNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::NullLiteralNode *node)
{
    return "{\"ntype\":\"NullLiteralNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::TypedefNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::TypedefNode *node)
{
    std::string str = "{\"ntype\":\"TypedefNode\",\"base\":";
    str += next(state, node->m_orig);
    str += ",\"name\":\"" + escape_json(node->m_name);
    return str + "\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::VarDeclNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::VarDeclNode *node)
{
    state.m_visited.clear();

    std::string str = "{\"ntype\":\"VarDeclNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"value\":";
    if (node->m_init)
        str += next(state, node->m_init);
    else
        str += "null";
    str += ",\"deprecated\":";
    str += std::string(node->m_is_deprecated ? "true" : "false");
    str += ",\"mutable\":";
    str += std::string(node->m_is_mut ? "true" : "false");
    str += ",\"static\":";
    str += std::string(node->m_is_static ? "true" : "false");
    str += ",\"thread_local\":";
    str += std::string(node->m_is_thread_local ? "true" : "false");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::LetDeclNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::LetDeclNode *node)
{
    state.m_visited.clear();

    std::string str = "{\"ntype\":\"LetDeclNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"value\":";
    if (node->m_init)
        str += next(state, node->m_init);
    else
        str += "null";
    str += ",\"deprecated\":";
    str += std::string(node->m_is_deprecated ? "true" : "false");
    str += ",\"mutable\":";
    str += std::string(node->m_is_mut ? "true" : "false");
    str += ",\"static\":";
    str += std::string(node->m_is_static ? "true" : "false");
    str += ",\"thread_local\":";
    str += std::string(node->m_is_thread_local ? "true" : "false");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::ConstDeclNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ConstDeclNode *node)
{
    state.m_visited.clear();

    std::string str = "{\"ntype\":\"ConstDeclNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    if (node->m_type)
        str += next(state, node->m_type);
    else
        str += "null";
    str += ",\"value\":";
    if (node->m_init)
        str += next(state, node->m_init);
    else
        str += "null";
    str += ",\"deprecated\":";
    str += std::string(node->m_is_deprecated ? "true" : "false");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::FunctionDeclNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::FunctionDeclNode *node)
{
    state.m_visited.clear();

    std::string str = "{\"ntype\":\"FunctionDeclNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"params\":[";
    for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_params.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"return_type\":";
    str += next(state, node->m_type->m_return_type);
    str += ",\"variadic\":";
    str += std::string(node->m_type->m_variadic ? "true" : "false");
    str += ",\"pure\":";
    str += std::string(node->m_type->m_pure ? "true" : "false");
    str += ",\"thread_safe\":";
    str += std::string(node->m_type->m_thread_safe ? "true" : "false");
    str += ",\"foreign\":";
    str += std::string(node->m_type->m_foreign ? "true" : "false");
    str += ",\"nothrow\":";
    str += std::string(node->m_type->m_nothrow ? "true" : "false");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::StructDefNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::StructDefNode *node)
{
    std::string str = "{\"ntype\":\"StructDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"methods\":[";
    for (auto it = node->m_methods.begin(); it != node->m_methods.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_methods.end() - 1)
        {
            str += ",";
        }
    }
    str += "],\"static_methods\":[";
    for (auto it = node->m_static_methods.begin(); it != node->m_static_methods.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_static_methods.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::StructFieldNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::StructFieldNode *node)
{
    std::string str = "{\"ntype\":\"StructFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"value\":";
    str += (node->m_value ? next(state, node->m_value) : "null");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::RegionDefNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::RegionDefNode *node)
{
    std::string str = "{\"ntype\":\"RegionDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::RegionFieldNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::RegionFieldNode *node)
{
    std::string str = "{\"ntype\":\"RegionFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"value\":";
    str += (node->m_value ? next(state, node->m_value) : "null");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::GroupDefNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::GroupDefNode *node)
{
    std::string str = "{\"ntype\":\"GroupDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    auto fields = node->get_fields();
    str += "\",\"fields\":[";
    for (auto it = fields.begin(); it != fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::GroupFieldNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::GroupFieldNode *node)
{
    std::string str = "{\"ntype\":\"GroupFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"value\":";
    str += (node->m_value ? next(state, node->m_value) : "null");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::UnionDefNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::UnionDefNode *node)
{
    std::string str = "{\"ntype\":\"UnionDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::UnionFieldNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::UnionFieldNode *node)
{
    std::string str = "{\"ntype\":\"UnionFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"value\":";
    str += (node->m_value ? next(state, node->m_value) : "null");
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::EnumDefNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::EnumDefNode *node)
{
    std::string str = "{\"ntype\":\"EnumDefNode\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::EnumFieldNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::EnumFieldNode *node)
{
    std::string str = "{\"ntype\":\"EnumFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    if (node->m_value)
    {
        str += "\",\"value\":";
        str += next(state, node->m_value);
    }
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::FunctionDefNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::FunctionDefNode *node)
{
    std::string str = "{\"ntype\":\"FunctionDefNode\",\"decl\":";
    str += next(state, node->m_decl);
    str += ",\"body\":";
    str += next(state, node->m_body);
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::FunctionParamNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::FunctionParamNode *node)
{
    state.m_visited.clear();

    std::string str = "{\"ntype\":\"FunctionParamNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += next(state, node->m_type);
    str += ",\"value\":";
    if (node->m_value)
        str += next(state, node->m_value);
    else
        str += "null";
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::SubsystemNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::SubsystemNode *node)
{
    std::string str = "{\"ntype\":\"SubsystemNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"deps\":[";
    for (auto it = node->m_deps.begin(); it != node->m_deps.end(); ++it)
    {
        str += "\"" + escape_json(*it) + "\"";
        if (it != node->m_deps.end())
        {
            str += ",";
        }
    }

    str += "],\"block\":" + next(state, node->m_block) + "}";
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::ExportNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ExportNode *node)
{
    std::string str = "{\"ntype\":\"ExportNode\",\"block\":[";
    for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it)
    {
        str += next(state, *it);
        if (it != node->m_stmts.end() - 1)
        {
            str += ",";
        }
    }
    return str + "]}";
}

std::string libquixcc::serialize::ParseTreeSerializer::InlineAsmNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::InlineAsmNode *node)
{
    std::string str = "{\"ntype\":\"InlineAsmNode\",\"asm\":\"";
    str += escape_json(node->m_asm);
    str += "\",\"outputs\":[";
    for (auto it = node->m_outputs.begin(); it != node->m_outputs.end(); ++it)
    {
        str += next(state, (*it).second);

        if (it != node->m_outputs.end())
        {
            str += ",";
        }
    }

    str += "],\"inputs\":[";

    for (auto it = node->m_inputs.begin(); it != node->m_inputs.end(); ++it)
    {
        str += next(state, (*it).second);

        if (it != node->m_inputs.end())
        {
            str += ",";
        }
    }

    str += "],\"clobbers\":[";

    for (auto it = node->m_clobbers.begin(); it != node->m_clobbers.end(); ++it)
    {
        str += *it;

        if (it != node->m_clobbers.end() - 1)
        {
            str += ",";
        }
    }

    str += "]}";

    return str;
}

std::string libquixcc::serialize::ParseTreeSerializer::ReturnStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ReturnStmtNode *node)
{
    if (node->m_expr)
        return "{\"ntype\":\"ReturnStmtNode\",\"value\":" + next(state, node->m_expr) + "}";
    else
        return "{\"ntype\":\"ReturnStmtNode\"}";
}

std::string libquixcc::serialize::ParseTreeSerializer::RetifStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::RetifStmtNode *node)
{
    return "{\"ntype\":\"RetifStmtNode\",\"cond\":" + next(state, node->m_cond) + ",\"value\":" + next(state, node->m_return) + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::RetzStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::RetzStmtNode *node)
{
    return "{\"ntype\":\"RetzStmtNode\",\"cond\":" + next(state, node->m_cond) + ",\"value\":" + next(state, node->m_return) + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::RetvStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::RetvStmtNode *node)
{
    return "{\"ntype\":\"RetvStmtNode\",\"cond\":" + next(state, node->m_cond) + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::IfStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::IfStmtNode *node)
{
    std::string str = "{\"ntype\":\"IfStmtNode\",\"cond\":" + next(state, node->m_cond) + ",\"then\":" + next(state, node->m_then);
    if (node->m_else)
    {
        str += ",\"else\":" + next(state, node->m_else);
    }
    return str + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::WhileStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::WhileStmtNode *node)
{
    return "{\"ntype\":\"WhileStmtNode\",\"cond\":" + next(state, node->m_cond) + ",\"block\":" + next(state, node->m_stmt) + "}";
}

std::string libquixcc::serialize::ParseTreeSerializer::ForStmtNode_conv(libquixcc::serialize::ParseTreeSerializerState &state, const libquixcc::ForStmtNode *node)
{
    std::string str = "{\"ntype\":\"ForStmtNode\"";

    if (node->m_init)
        str += ",\"init\":" + next(state, node->m_init);
    else
        str += ",\"init\":null";

    if (node->m_cond)
        str += ",\"cond\":" + next(state, node->m_cond);
    else
        str += ",\"cond\":null";

    if (node->m_step)
        str += ",\"step\":" + next(state, node->m_step);
    else
        str += ",\"step\":null";

    str += ",\"block\":" + next(state, node->m_stmt);

    return str + "}";
}