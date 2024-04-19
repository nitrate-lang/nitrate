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

#define QUIXCC_INTERNAL

#include <parse/JsonSerialize.h>
#include <parse/nodes/AllNodes.h>
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

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ExprStmtNode *node)
{
    return "{\"ntype\":\"ExprStmtNode\",\"expr\":" + node->m_expr->to_json(*this) + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::NopStmtNode *node)
{
    return "{\"ntype\":\"nop\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::BlockNode *node)
{
    std::string str = "{\"ntype\":\"BlockNode\",\"children\":[";
    for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_stmts.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::StmtGroupNode *node)
{
    std::string str;

    for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_stmts.end() - 1)
        {
            str += ",";
        }
    }

    return str;
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::UnaryExprNode *node)
{
    std::string str = "{\"ntype\":\"UnaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"operand\":";
    str += node->m_expr->to_json(*this);

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::BinaryExprNode *node)
{
    std::string str = "{\"ntype\":\"BinaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"lhs\":";
    str += node->m_lhs->to_json(*this);
    str += ",\"rhs\":";
    str += node->m_rhs->to_json(*this);

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::CallExprNode *node)
{
    std::string str = "{\"ntype\":\"CallExprNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"named_args\":[";
    for (auto it = node->m_named_args.begin(); it != node->m_named_args.end(); ++it)
    {
        str += "{\"name\":\"" + escape_json(it->first) + "\",\"value\":" + it->second->to_json(*this) + "}";
        if (it != node->m_named_args.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"positional_args\":[";
    for (auto it = node->m_positional_args.begin(); it != node->m_positional_args.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_positional_args.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ListExprNode *node)
{
    std::string str = "{\"ntype\":\"ListExprNode\",\"elements\":[";
    for (auto it = node->m_elements.begin(); it != node->m_elements.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_elements.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::MemberAccessNode *node)
{
    std::string str = "{\"ntype\":\"MemberAccessNode\",\"lhs\":";
    str += node->m_expr->to_json(*this);
    str += ",\"field\":\"" + escape_json(node->m_field) + "\"";

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ConstUnaryExprNode *node)
{
    std::string str = "{\"ntype\":\"ConstUnaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"operand\":";
    str += node->m_expr->to_json(*this);

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ConstBinaryExprNode *node)
{
    std::string str = "{\"ntype\":\"ConstBinaryExprNode\",\"op\":\"";
    str += operator_map_inverse.at(node->m_op);
    str += "\",\"lhs\":";
    str += node->m_lhs->to_json(*this);
    str += ",\"rhs\":";
    str += node->m_rhs->to_json(*this);

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::IdentifierNode *node)
{
    return "{\"ntype\":\"IdentifierNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::MutTypeNode *node)
{
    return "{\"ntype\":\"MutTypeNode\",\"type\":" + node->m_type->to_json(*this) + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::U8TypeNode *node)
{
    return "{\"ntype\":\"U8TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::U16TypeNode *node)
{
    return "{\"ntype\":\"U16TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::U32TypeNode *node)
{
    return "{\"ntype\":\"U32TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::U64TypeNode *node)
{
    return "{\"ntype\":\"U64TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::I8TypeNode *node)
{
    return "{\"ntype\":\"I8TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::I16TypeNode *node)
{
    return "{\"ntype\":\"I16TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::I32TypeNode *node)
{
    return "{\"ntype\":\"I32TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::I64TypeNode *node)
{
    return "{\"ntype\":\"I64TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::F32TypeNode *node)
{
    return "{\"ntype\":\"F32TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::F64TypeNode *node)
{
    return "{\"ntype\":\"F64TypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::BoolTypeNode *node)
{
    return "{\"ntype\":\"BoolTypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::VoidTypeNode *node)
{
    return "{\"ntype\":\"VoidTypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::PointerTypeNode *node)
{
    std::string str = "{\"ntype\":\"PointerTypeNode\",\"type\":";
    str += node->m_type->to_json(*this);
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::OpaqueTypeNode *node)
{
    return "{\"ntype\":\"OpaqueTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::StringTypeNode *node)
{
    return "{\"ntype\":\"StringTypeNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::EnumTypeNode *node)
{
    std::string str = "{\"ntype\":\"EnumTypeNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"member_type\":";
    str += node->m_member_type->to_json(*this);
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::StructTypeNode *node)
{
    if (m_visited.contains(node))
        return "{\"ntype\":\"StructTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";

    m_visited.insert(node); // Prevent infinite recursion

    std::string str = "{\"ntype\":\"StructTypeNode\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"name\":\"" + escape_json(node->m_name) + "\"";

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::RegionTypeNode *node)
{
    if (m_visited.contains(node))
        return "{\"ntype\":\"RegionTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";
    
    m_visited.insert(node); // Prevent infinite recursion

    std::string str = "{\"ntype\":\"RegionTypeNode\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::UnionTypeNode *node)
{
    if (m_visited.contains(node))
        return "{\"ntype\":\"UnionTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";

    m_visited.insert(node); // Prevent infinite recursion
    
    std::string str = "{\"ntype\":\"UnionTypeNode\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ArrayTypeNode *node)
{
    std::string str = "{\"ntype\":\"ArrayTypeNode\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"size\":";
    str += node->m_size->to_json(*this);
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::FunctionTypeNode *node)
{
    std::string str = "{\"ntype\":\"FunctionTypeNode\",\"params\":[";
    for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it)
    {
        str += (*it).second->to_json(*this);
        if (it != node->m_params.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"return_type\":";
    str += node->m_return_type->to_json(*this);
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

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::UserTypeNode *node)
{
    return "{\"ntype\":\"UserTypeNode\",\"name\":\"" + escape_json(node->m_name) + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::IntegerLiteralNode *node)
{
    return "{\"ntype\":\"IntegerLiteralNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::FloatLiteralNode *node)
{
    return "{\"ntype\":\"FloatLiteralNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::StringLiteralNode *node)
{
    return "{\"ntype\":\"StringLiteralNode\",\"value\":\"" + escape_json(node->m_val) + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::CharLiteralNode *node)
{
    return "{\"ntype\":\"CharLiteralNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::BoolLiteralNode *node)
{
    return "{\"ntype\":\"BoolLiteralNode\",\"value\":\"" + std::string(node->m_val ? "true" : "false") + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::NullLiteralNode *node)
{
    return "{\"ntype\":\"NullLiteralNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::TypedefNode *node)
{
    std::string str = "{\"ntype\":\"TypedefNode\",\"base\":";
    str += node->m_orig->to_json(*this);
    str += ",\"name\":\"" + escape_json(node->m_name);
    return str + "\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::VarDeclNode *node)
{
    m_visited.clear();

    std::string str = "{\"ntype\":\"VarDeclNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"value\":";
    if (node->m_init)
        str += node->m_init->to_json(*this);
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

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::LetDeclNode *node)
{
    m_visited.clear();
    
    std::string str = "{\"ntype\":\"LetDeclNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"value\":";
    if (node->m_init)
        str += node->m_init->to_json(*this);
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

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::FunctionDeclNode *node)
{
    m_visited.clear();
    
    std::string str = "{\"ntype\":\"FunctionDeclNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"params\":[";
    for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_params.end() - 1)
        {
            str += ",";
        }
    }

    str += "],\"return_type\":";
    str += node->m_type->m_return_type->to_json(*this);
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

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::StructDefNode *node)
{
    std::string str = "{\"ntype\":\"StructDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::StructFieldNode *node)
{
    std::string str = "{\"ntype\":\"StructFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"value\":";
    str += (node->m_value ? node->m_value->to_json(*this) : "null");
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::RegionDefNode *node)
{
    std::string str = "{\"ntype\":\"RegionDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::RegionFieldNode *node)
{
    std::string str = "{\"ntype\":\"RegionFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"value\":";
    str += (node->m_value ? node->m_value->to_json(*this) : "null");
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::GroupDefNode *node)
{
    std::string str = "{\"ntype\":\"GroupDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    auto fields = node->get_fields();
    str += "\",\"fields\":[";
    for (auto it = fields.begin(); it != fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::GroupFieldNode *node)
{
    std::string str = "{\"ntype\":\"GroupFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"value\":";
    str += (node->m_value ? node->m_value->to_json(*this) : "null");
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::UnionDefNode *node)
{
    std::string str = "{\"ntype\":\"UnionDefNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::UnionFieldNode *node)
{
    std::string str = "{\"ntype\":\"UnionFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"value\":";
    str += (node->m_value ? node->m_value->to_json(*this) : "null");
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::EnumDefNode *node)
{
    std::string str = "{\"ntype\":\"EnumDefNode\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"fields\":[";
    for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_fields.end() - 1)
        {
            str += ",";
        }
    }

    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::EnumFieldNode *node)
{
    std::string str = "{\"ntype\":\"EnumFieldNode\",\"name\":\"";
    str += escape_json(node->m_name);
    if (node->m_value)
    {
        str += "\",\"value\":";
        str += node->m_value->to_json(*this);
    }
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::FunctionDefNode *node)
{
    std::string str = "{\"ntype\":\"FunctionDefNode\",\"decl\":";
    str += node->m_decl->to_json(*this);
    str += ",\"body\":";
    str += node->m_body->to_json(*this);
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::FunctionParamNode *node)
{
    m_visited.clear();

    std::string str = "{\"ntype\":\"FunctionParamNode\",\"name\":\"";
    str += escape_json(node->m_name);
    str += "\",\"type\":";
    str += node->m_type->to_json(*this);
    str += ",\"value\":";
    if (node->m_value)
        str += node->m_value->to_json(*this);
    else
        str += "null";
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::SubsystemNode *node)
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

    str += "],\"block\":" + node->m_block->to_json(*this);
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ExportNode *node)
{
    std::string str = "{\"ntype\":\"ExportNode\",\"block\":[";
    for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it)
    {
        str += (*it)->to_json(*this);
        if (it != node->m_stmts.end() - 1)
        {
            str += ",";
        }
    }
    return str + "]}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::InlineAsmNode *node)
{
    std::string str = "{\"ntype\":\"InlineAsmNode\",\"asm\":\"";
    str += escape_json(node->m_asm);
    str += "\",\"outputs\":[";
    for (auto it = node->m_outputs.begin(); it != node->m_outputs.end(); ++it)
    {
        str += (*it).second->to_json(*this);

        if (it != node->m_outputs.end())
        {
            str += ",";
        }
    }

    str += "],\"inputs\":[";

    for (auto it = node->m_inputs.begin(); it != node->m_inputs.end(); ++it)
    {
        str += (*it).second->to_json(*this);

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

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ReturnStmtNode *node)
{
    if (node->m_expr)
        return "{\"ntype\":\"ReturnStmtNode\",\"value\":" + node->m_expr->to_json(*this) + "}";
    else
        return "{\"ntype\":\"ReturnStmtNode\"}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::RetifStmtNode *node)
{
    return "{\"ntype\":\"RetifStmtNode\",\"cond\":" + node->m_cond->to_json(*this) + ",\"value\":" + node->m_return->to_json(*this) + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::RetzStmtNode *node)
{
    return "{\"ntype\":\"RetzStmtNode\",\"cond\":" + node->m_cond->to_json(*this) + ",\"value\":" + node->m_return->to_json(*this) + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::RetvStmtNode *node)
{
    return "{\"ntype\":\"RetvStmtNode\",\"cond\":" + node->m_cond->to_json(*this) + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::IfStmtNode *node)
{
    std::string str = "{\"ntype\":\"IfStmtNode\",\"cond\":" + node->m_cond->to_json(*this) + ",\"then\":" + node->m_then->to_json(*this);
    if (node->m_else)
    {
        str += ",\"else\":" + node->m_else->to_json(*this);
    }
    return str + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::WhileStmtNode *node)
{
    return "{\"ntype\":\"WhileStmtNode\",\"cond\":" + node->m_cond->to_json(*this) + ",\"block\":" + node->m_stmt->to_json(*this) + "}";
}

std::string libquixcc::ParseNodeJsonSerializerVisitor::visit(const libquixcc::ForStmtNode *node)
{
    std::string str = "{\"ntype\":\"ForStmtNode\"";

    if (node->m_init)
        str += ",\"init\":" + node->m_init->to_json(*this);
    else
        str += ",\"init\":null";

    if (node->m_cond)
        str += ",\"cond\":" + node->m_cond->to_json(*this);
    else
        str += ",\"cond\":null";

    if (node->m_step)
        str += ",\"step\":" + node->m_step->to_json(*this);
    else
        str += ",\"step\":null";

    str += ",\"block\":" + node->m_stmt->to_json(*this);

    return str + "}";
}