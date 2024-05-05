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

/*
    TODO: make this code less shitty
*/

#define QUIXCC_INTERNAL

#include <generate/CodeGen.h>
#include <mangle/Symbol.h>
#include <core/Logger.h>

bool libquixcc::C11Codegen::codegen(const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir, std::ostream &os)
{
    return false;
}

// static std::string getind(size_t ind)
// {
//     return std::string(ind, ' ');
// }

// static std::string c_escape_string_literal(const std::string &str)
// {
//     std::string out;

//     for (char c : str)
//     {
//         switch (c)
//         {
//         case '\n':
//             out += "\\n";
//             break;
//         case '\r':
//             out += "\\r";
//             break;
//         case '\t':
//             out += "\\t";
//             break;
//         case '\v':
//             out += "\\v";
//             break;
//         case '\b':
//             out += "\\b";
//             break;
//         case '\f':
//             out += "\\f";
//             break;
//         case '\a':
//             out += "\\a";
//             break;
//         case '\\':
//             out += "\\\\";
//             break;
//         case '\"':
//             out += "\\\"";
//             break;
//         default:
//             out += c;
//             break;
//         }
//     }

//     return out;
// }

// static bool is_pointer(const libquixcc::TypeNode *node)
// {
//     if (node->ntype == libquixcc::NodeType::PointerTypeNode)
//         return true;

//     if (node->ntype != libquixcc::NodeType::MutTypeNode)
//         return false;

//     libquixcc::MutTypeNode *mut = (libquixcc::MutTypeNode *)node;

//     return is_pointer(mut->m_type);
// }

// #define INDENT getind(m_state.indent)

// #define STEP 2

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::BlockNode *node)
// {
//     std::string code = "{\n";

//     m_state.indent += STEP;

//     for (auto &stmt : node->m_stmts)
//     {
//         code += INDENT + stmt->codegen(*this) + "\n";

//         if (!m_state.function_def)
//             code += "\n";
//     }

//     m_state.indent -= STEP;

//     code += INDENT + "}";

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::StmtGroupNode *node)
// {
//     std::string code;

//     for (auto &stmt : node->m_stmts)
//     {
//         code += INDENT + stmt->codegen(*this) + "\n";

//         if (!m_state.function_def)
//             code += "\n";
//     }

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ExprStmtNode *node)
// {
//     return node->m_expr->codegen(*this) + ";";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::BitCastExprNode *node)
// {
//     // Create a union with the same size as the source type
//     // Assign the source value to the union
//     // Read the destination value from the union

//     // Syntax?: uint64_t x0 = union { double a, uint64_t b; }

//     throw CodegenException("BitCastExprNode not implemented for C");
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::SignedUpcastExprNode *node)
// {
//     return "(" + node->m_type->codegen(*this) + ")" + node->m_expr->codegen(*this);
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::UnsignedUpcastExprNode *node)
// {
//     return "(" + node->m_type->codegen(*this) + ")" + node->m_expr->codegen(*this);
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::DowncastExprNode *node)
// {
//     return "(" + node->m_type->codegen(*this) + ")" + node->m_expr->codegen(*this);
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::IntToPtrCastExprNode *node)
// {
//     return "(" + node->m_type->codegen(*this) + ")" + node->m_expr->codegen(*this);
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::PtrToIntCastExprNode *node)
// {
//     return "(" + node->m_type->codegen(*this) + ")" + node->m_expr->codegen(*this);
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::UnaryExprNode *node)
// {
//     switch (node->m_op)
//     {
//     case Operator::Minus:
//         return "-" + node->m_expr->codegen(*this);
//     case Operator::Plus:
//         return "+" + node->m_expr->codegen(*this);
//     case Operator::BitwiseNot:
//         return "~" + node->m_expr->codegen(*this);
//     case Operator::LogicalNot:
//         return "!" + node->m_expr->codegen(*this);
//     case Operator::Increment:
//         return node->m_expr->codegen(*this) + "++";
//     case Operator::Decrement:
//         return node->m_expr->codegen(*this) + "--";
//     case Operator::BitwiseAnd:
//         return "&" + node->m_expr->codegen(*this);
//     case Operator::Multiply:
//         return "*" + node->m_expr->codegen(*this);
//     default:
//         throw CodegenException("Unary operator not codegenable");
//     }
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::BinaryExprNode *node)
// {
//     switch (node->m_op)
//     {
//     case Operator::LessThan:
//         return node->m_lhs->codegen(*this) + " < " + node->m_rhs->codegen(*this);
//     case Operator::GreaterThan:
//         return node->m_lhs->codegen(*this) + " > " + node->m_rhs->codegen(*this);
//     case Operator::LessThanEqual:
//         return node->m_lhs->codegen(*this) + " <= " + node->m_rhs->codegen(*this);
//     case Operator::GreaterThanEqual:
//         return node->m_lhs->codegen(*this) + " >= " + node->m_rhs->codegen(*this);
//     case Operator::Equal:
//         return node->m_lhs->codegen(*this) + " == " + node->m_rhs->codegen(*this);
//     case Operator::NotEqual:
//         return node->m_lhs->codegen(*this) + " != " + node->m_rhs->codegen(*this);
//     case Operator::Plus:
//         return node->m_lhs->codegen(*this) + " + " + node->m_rhs->codegen(*this);
//     case Operator::Minus:
//         return node->m_lhs->codegen(*this) + " - " + node->m_rhs->codegen(*this);
//     case Operator::Multiply:
//         return node->m_lhs->codegen(*this) + " * " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseAnd:
//         return node->m_lhs->codegen(*this) + " & " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseOr:
//         return node->m_lhs->codegen(*this) + " | " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseXor:
//         return node->m_lhs->codegen(*this) + " ^ " + node->m_rhs->codegen(*this);
//     case Operator::LeftShift:
//         return node->m_lhs->codegen(*this) + " << " + node->m_rhs->codegen(*this);
//     case Operator::RightShift:
//         return node->m_lhs->codegen(*this) + " >> " + node->m_rhs->codegen(*this);
//     case Operator::LogicalAnd:
//         return node->m_lhs->codegen(*this) + " && " + node->m_rhs->codegen(*this);
//     case Operator::LogicalOr:
//         return node->m_lhs->codegen(*this) + " || " + node->m_rhs->codegen(*this);
//     case Operator::LogicalXor:
//         return node->m_lhs->codegen(*this) + " ^^ " + node->m_rhs->codegen(*this);
//     case Operator::Divide:
//         return node->m_lhs->codegen(*this) + " / " + node->m_rhs->codegen(*this);
//     case Operator::Modulo:
//         return node->m_lhs->codegen(*this) + " % " + node->m_rhs->codegen(*this);
//     case Operator::Assign:
//         return node->m_lhs->codegen(*this) + " = " + node->m_rhs->codegen(*this);
//     case Operator::PlusAssign:
//         return node->m_lhs->codegen(*this) + " += " + node->m_rhs->codegen(*this);
//     case Operator::MinusAssign:
//         return node->m_lhs->codegen(*this) + " -= " + node->m_rhs->codegen(*this);
//     case Operator::MultiplyAssign:
//         return node->m_lhs->codegen(*this) + " *= " + node->m_rhs->codegen(*this);
//     case Operator::DivideAssign:
//         return node->m_lhs->codegen(*this) + " /= " + node->m_rhs->codegen(*this);
//     case Operator::ModuloAssign:
//         return node->m_lhs->codegen(*this) + " %= " + node->m_rhs->codegen(*this);
//     case Operator::LeftShiftAssign:
//         return node->m_lhs->codegen(*this) + " <<= " + node->m_rhs->codegen(*this);
//     case Operator::RightShiftAssign:
//         return node->m_lhs->codegen(*this) + " >>= " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseAndAssign:
//         return node->m_lhs->codegen(*this) + " &= " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseOrAssign:
//         return node->m_lhs->codegen(*this) + " |= " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseXorAssign:
//         return node->m_lhs->codegen(*this) + " ^= " + node->m_rhs->codegen(*this);
//     default:
//         throw CodegenException("Binary operator not codegenable");
//     }
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::CallExprNode *node)
// {
//     std::string code;

//     if (m_state.variables.contains(node->m_name))
//         code += m_state.variables[node->m_name];
//     else
//         code += node->m_name;

//     code += "(";
//     for (auto it = node->m_positional_args.begin(); it != node->m_positional_args.end(); ++it)
//     {
//         code += (*it)->codegen(*this);
//         if (it != node->m_positional_args.end() - 1)
//             code += ", ";
//     }

//     if (node->m_named_args.size() != 0)
//         throw CodegenException("CallExprNode not implemented for C");

//     code += ")";

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ListExprNode *node)
// {
//     throw CodegenException("ListExprNode not implemented for C");
//     return "";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::MemberAccessNode *node)
// {
//     auto expr = node->m_expr->codegen(*this);

//     if (m_state.is_pointers.contains(expr) && m_state.is_pointers[expr])
//         return expr + "->" + node->m_field;

//     return expr + "." + node->m_field;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ConstUnaryExprNode *node)
// {
//     switch (node->m_op)
//     {
//     case Operator::Minus:
//         return "-" + node->m_expr->codegen(*this);
//     case Operator::Plus:
//         return "+" + node->m_expr->codegen(*this);
//     case Operator::BitwiseNot:
//         return "~" + node->m_expr->codegen(*this);
//     case Operator::LogicalNot:
//         return "!" + node->m_expr->codegen(*this);
//     case Operator::Increment:
//         return node->m_expr->codegen(*this) + "++";
//     case Operator::Decrement:
//         return node->m_expr->codegen(*this) + "--";
//     case Operator::BitwiseAnd:
//         return "&" + node->m_expr->codegen(*this);
//     case Operator::Multiply:
//         return "*" + node->m_expr->codegen(*this);
//     default:
//         throw CodegenException("Unary operator not codegenable");
//     }
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ConstBinaryExprNode *node)
// {
//     switch (node->m_op)
//     {
//     case Operator::LessThan:
//         return node->m_lhs->codegen(*this) + " < " + node->m_rhs->codegen(*this);
//     case Operator::GreaterThan:
//         return node->m_lhs->codegen(*this) + " > " + node->m_rhs->codegen(*this);
//     case Operator::LessThanEqual:
//         return node->m_lhs->codegen(*this) + " <= " + node->m_rhs->codegen(*this);
//     case Operator::GreaterThanEqual:
//         return node->m_lhs->codegen(*this) + " >= " + node->m_rhs->codegen(*this);
//     case Operator::Equal:
//         return node->m_lhs->codegen(*this) + " == " + node->m_rhs->codegen(*this);
//     case Operator::NotEqual:
//         return node->m_lhs->codegen(*this) + " != " + node->m_rhs->codegen(*this);
//     case Operator::Plus:
//         return node->m_lhs->codegen(*this) + " + " + node->m_rhs->codegen(*this);
//     case Operator::Minus:
//         return node->m_lhs->codegen(*this) + " - " + node->m_rhs->codegen(*this);
//     case Operator::Multiply:
//         return node->m_lhs->codegen(*this) + " * " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseAnd:
//         return node->m_lhs->codegen(*this) + " & " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseOr:
//         return node->m_lhs->codegen(*this) + " | " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseXor:
//         return node->m_lhs->codegen(*this) + " ^ " + node->m_rhs->codegen(*this);
//     case Operator::LeftShift:
//         return node->m_lhs->codegen(*this) + " << " + node->m_rhs->codegen(*this);
//     case Operator::RightShift:
//         return node->m_lhs->codegen(*this) + " >> " + node->m_rhs->codegen(*this);
//     case Operator::LogicalAnd:
//         return node->m_lhs->codegen(*this) + " && " + node->m_rhs->codegen(*this);
//     case Operator::LogicalOr:
//         return node->m_lhs->codegen(*this) + " || " + node->m_rhs->codegen(*this);
//     case Operator::LogicalXor:
//         return node->m_lhs->codegen(*this) + " ^^ " + node->m_rhs->codegen(*this);
//     case Operator::Divide:
//         return node->m_lhs->codegen(*this) + " / " + node->m_rhs->codegen(*this);
//     case Operator::Modulo:
//         return node->m_lhs->codegen(*this) + " % " + node->m_rhs->codegen(*this);
//     case Operator::Assign:
//         return node->m_lhs->codegen(*this) + " = " + node->m_rhs->codegen(*this);
//     case Operator::PlusAssign:
//         return node->m_lhs->codegen(*this) + " += " + node->m_rhs->codegen(*this);
//     case Operator::MinusAssign:
//         return node->m_lhs->codegen(*this) + " -= " + node->m_rhs->codegen(*this);
//     case Operator::MultiplyAssign:
//         return node->m_lhs->codegen(*this) + " *= " + node->m_rhs->codegen(*this);
//     case Operator::DivideAssign:
//         return node->m_lhs->codegen(*this) + " /= " + node->m_rhs->codegen(*this);
//     case Operator::ModuloAssign:
//         return node->m_lhs->codegen(*this) + " %= " + node->m_rhs->codegen(*this);
//     case Operator::LeftShiftAssign:
//         return node->m_lhs->codegen(*this) + " <<= " + node->m_rhs->codegen(*this);
//     case Operator::RightShiftAssign:
//         return node->m_lhs->codegen(*this) + " >>= " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseAndAssign:
//         return node->m_lhs->codegen(*this) + " &= " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseOrAssign:
//         return node->m_lhs->codegen(*this) + " |= " + node->m_rhs->codegen(*this);
//     case Operator::BitwiseXorAssign:
//         return node->m_lhs->codegen(*this) + " ^= " + node->m_rhs->codegen(*this);
//     default:
//         throw CodegenException("Binary operator not codegenable");
//     }
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::IdentifierNode *node)
// {
//     if (m_state.variables.contains(node->m_name))
//         return m_state.variables[node->m_name];

//     return node->m_name;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::MutTypeNode *node)
// {
//     bool state = m_state.mut;
//     m_state.mut = true;
//     std::string res = node->m_type->codegen(*this);

//     m_state.mut = state;

//     return res;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::U8TypeNode *node)
// {
//     if (m_state.mut)
//         return "uint8_t";
//     else
//         return "const uint8_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::U16TypeNode *node)
// {
//     if (m_state.mut)
//         return "uint16_t";
//     else
//         return "const uint16_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::U32TypeNode *node)
// {
//     if (m_state.mut)
//         return "uint32_t";
//     else
//         return "const uint32_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::U64TypeNode *node)
// {
//     if (m_state.mut)
//         return "uint64_t";
//     else
//         return "const uint64_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::I8TypeNode *node)
// {
//     if (m_state.mut)
//         return "int8_t";
//     else
//         return "const int8_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::I16TypeNode *node)
// {
//     if (m_state.mut)
//         return "int16_t";
//     else
//         return "const int16_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::I32TypeNode *node)
// {
//     if (m_state.mut)
//         return "int32_t";
//     else
//         return "const int32_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::I64TypeNode *node)
// {
//     if (m_state.mut)
//         return "int64_t";
//     else
//         return "const int64_t";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::F32TypeNode *node)
// {
//     if (m_state.mut)
//         return "float";
//     else
//         return "const float";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::F64TypeNode *node)
// {
//     if (m_state.mut)
//         return "double";
//     else
//         return "const double";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::BoolTypeNode *node)
// {
//     if (m_state.mut)
//         return "bool";
//     else
//         return "const bool";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::VoidTypeNode *node)
// {
//     if (m_state.mut)
//         return "void";
//     else
//         return "const void";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::PointerTypeNode *node)
// {
//     if (m_state.mut)
//         return node->m_type->codegen(*this) + "*";
//     else
//         return node->m_type->codegen(*this) + "* const";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::StringTypeNode *node)
// {
//     if (m_state.mut)
//         return "char*";
//     else
//         return "const char*";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::EnumTypeNode *node)
// {
//     if (m_state.mut)
//         return "enum " + node->m_name;
//     else
//         return "const enum " + node->m_name;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::StructTypeNode *node)
// {
//     if (m_state.mut)
//         return "struct " + node->m_name;
//     else
//         return "const struct " + node->m_name;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::RegionTypeNode *node)
// {
//     if (m_state.mut)
//         return "struct " + node->m_name;
//     else
//         return "const struct " + node->m_name;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::UnionTypeNode *node)
// {
//     if (m_state.mut)
//         return "union " + node->m_name;
//     else
//         return "const union " + node->m_name;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ArrayTypeNode *node)
// {
//     if (m_state.mut)
//         return node->m_type->codegen(*this) + "[" + node->m_size->codegen(*this) + "]";
//     else
//         return "const " + node->m_type->codegen(*this) + "[" + node->m_size->codegen(*this) + "]";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::FunctionTypeNode *node)
// {
//     std::string code;

//     if (!m_state.mut)
//         code += "const ";

//     code += node->m_return_type->codegen(*this) + " (*)(";
//     for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it)
//     {
//         code += (*it).second->codegen(*this);
//         if (it != node->m_params.end() - 1)
//             code += ", ";
//     }

//     if (node->m_variadic)
//         code += ", ...";

//     if (node->m_params.size() != 0)
//         throw CodegenException("FunctionTypeNode not implemented for C");

//     code += ")";

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::IntegerNode *node)
// {
//     return node->m_val;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::FloatLiteralNode *node)
// {
//     return node->m_val;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::StringNode *node)
// {
//     return "\"" + c_escape_string_literal(node->m_val) + "\"";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::CharNode *node)
// {
//     return "'" + node->m_val + "'";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::BoolLiteralNode *node)
// {
//     return node->m_val ? "true" : "false";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::NullLiteralNode *node)
// {
//     return "NULL";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::LetDeclNode *node)
// {
//     std::string code;

//     if (node->m_is_thread_local)
//         code += "__thread ";
//     if (node->m_is_static)
//         code += "static ";

//     code += node->m_type->codegen(*this) + " ";

//     m_state.is_pointers[node->m_name] = is_pointer(node->m_type);

//     std::string name;

//     if (!m_state.function_def)
//         name = Symbol::mangle(node, m_state.subsystem, m_state.export_lang);
//     else
//         name = node->m_name;
//     m_state.variables[node->m_name] = name;

//     code += name;

//     if (node->m_init)
//         code += " = " + node->m_init->codegen(*this);

//     return code + ";";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::FunctionDeclNode *node)
// {
//     std::string code = node->m_type->m_return_type->codegen(*this) + " ";

//     std::string name = Symbol::mangle(node, m_state.subsystem, m_state.export_lang);

//     m_state.variables[node->m_name] = name;

//     code += name + "(";

//     for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it)
//     {
//         code += (*it)->codegen(*this);
//         if (it != node->m_params.end() - 1)
//             code += ", ";
//     }

//     if (node->m_type->m_variadic)
//         code += ", ...";

//     if (m_state.function_def)
//         code += ")";
//     else
//         code += ");";

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::StructDefNode *node)
// {
//     std::string code = "struct " + node->m_name + " {\n";
//     m_state.indent += STEP;

//     for (auto &field : node->m_fields)
//     {
//         code += INDENT + field->m_type->codegen(*this) + " " + field->m_name + ";\n";
//     }

//     m_state.indent -= STEP;

//     return code + INDENT + "};";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::RegionDefNode *node)
// {
//     std::string code = "struct " + node->m_name + " {\n";
//     m_state.indent += STEP;

//     for (auto &field : node->m_fields)
//     {
//         code += INDENT + field->m_type->codegen(*this) + " " + field->m_name + ";\n";
//     }

//     m_state.indent -= STEP;

//     return code + INDENT + "} __attribute__((packed));";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::UnionDefNode *node)
// {
//     std::string code = "union " + node->m_name + " {\n";
//     m_state.indent += STEP;

//     for (auto &field : node->m_fields)
//     {
//         code += INDENT + field->m_type->codegen(*this) + " " + field->m_name + ";\n";
//     }

//     m_state.indent -= STEP;

//     return code + INDENT + "};";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::FunctionDefNode *node)
// {
//     bool state = m_state.function_def;
//     m_state.function_def = true;
//     std::string code;

//     ExportLangType lang = m_state.export_lang;
//     bool pub = m_state.pub;

//     if (node->m_decl->m_name == "main" && m_state.subsystem.empty())
//     {
//         m_state.export_lang = ExportLangType::C;
//         m_state.pub = true;
//     }

//     if (!m_state.pub)
//         code += "static ";
//     else
//         code += "__attribute__((visibility(\"default\"))) ";

//     code += node->m_decl->codegen(*this);

//     code += " " + node->m_body->codegen(*this);

//     m_state.export_lang = lang;
//     m_state.pub = pub;
//     m_state.function_def = state;

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::FunctionParamNode *node)
// {
//     m_state.is_pointers[node->m_name] = is_pointer(node->m_type);

//     return node->m_type->codegen(*this) + " " + node->m_name;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ExportNode *node)
// {
//     std::string code;

//     bool pub = m_state.pub;
//     m_state.pub = true;
//     ExportLangType lang = m_state.export_lang;
//     m_state.export_lang = node->m_lang_type;

//     for (auto &stmt : node->m_stmts)
//     {
//         if (stmt->is<FunctionDefNode>())
//             code += stmt->codegen(*this) + "\n";
//         else if (stmt->is<LetDeclNode>())
//             code += "extern " + stmt->codegen(*this) + ";\n";
//         else
//             code += stmt->codegen(*this) + "\n";
//     }

//     m_state.pub = pub;
//     m_state.export_lang = lang;

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::InlineAsmNode *node)
// {
//     std::string code = "__asm__ __volatile__ (";

//     code += "\"" + c_escape_string_literal(node->m_asm) + "\"";

//     code += ":";

//     for (auto it = node->m_outputs.begin(); it != node->m_outputs.end(); ++it)
//     {
//         code += "\"" + it->first + "\"(" + it->second->codegen(*this) + ")";
//         if (it != node->m_outputs.end() - 1)
//             code += ",";
//     }

//     code += ":";

//     for (auto it = node->m_inputs.begin(); it != node->m_inputs.end(); ++it)
//     {
//         code += "\"" + it->first + "\"(" + it->second->codegen(*this) + ")";
//         if (it != node->m_inputs.end() - 1)
//             code += ",";
//     }

//     code += ":";

//     for (auto it = node->m_clobbers.begin(); it != node->m_clobbers.end(); ++it)
//     {
//         code += "\"" + *it + "\"";
//         if (it != node->m_clobbers.end() - 1)
//             code += ",";
//     }

//     code += ");";

//     return code;

//     throw CodegenException("InlineAsmNode not implemented for C");
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ReturnStmtNode *node)
// {
//     if (!node->m_expr)
//         return "return;";

//     return "return " + node->m_expr->codegen(*this) + ";";
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::IfStmtNode *node)
// {
//     std::string code = "if (" + node->m_cond->codegen(*this) + ") ";
//     code += node->m_then->codegen(*this);
//     if (node->m_else)
//         code += " else\n" + node->m_else->codegen(*this);

//     return code;
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::WhileStmtNode *node)
// {
//     return "while (" + node->m_cond->codegen(*this) + ") " + node->m_stmt->codegen(*this);
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::ExprNode *node)
// {
//     switch (node->ntype)
//     {
//     case NodeType::BitCastExprNode:
//         return visit(static_cast<const BitCastExprNode *>(node));
//     case NodeType::SignedUpcastExprNode:
//         return visit(static_cast<const SignedUpcastExprNode *>(node));
//     case NodeType::UnsignedUpcastExprNode:
//         return visit(static_cast<const UnsignedUpcastExprNode *>(node));
//     case NodeType::DowncastExprNode:
//         return visit(static_cast<const DowncastExprNode *>(node));
//     case NodeType::IntToPtrCastExprNode:
//         return visit(static_cast<const IntToPtrCastExprNode *>(node));
//     case NodeType::PtrToIntCastExprNode:
//         return visit(static_cast<const PtrToIntCastExprNode *>(node));
//     case NodeType::UnaryExprNode:
//         return visit(static_cast<const UnaryExprNode *>(node));
//     case NodeType::BinaryExprNode:
//         return visit(static_cast<const BinaryExprNode *>(node));
//     case NodeType::CallExprNode:
//         return visit(static_cast<const CallExprNode *>(node));
//     case NodeType::ListExprNode:
//         return visit(static_cast<const ListExprNode *>(node));
//     case NodeType::MemberAccessNode:
//         return visit(static_cast<const MemberAccessNode *>(node));
//     case NodeType::ConstUnaryExprNode:
//         return visit(static_cast<const ConstUnaryExprNode *>(node));
//     case NodeType::ConstBinaryExprNode:
//         return visit(static_cast<const ConstBinaryExprNode *>(node));
//     case NodeType::IdentifierNode:
//         return visit(static_cast<const IdentifierNode *>(node));
//     case NodeType::IntegerNode:
//         return visit(static_cast<const IntegerNode *>(node));
//     case NodeType::FloatLiteralNode:
//         return visit(static_cast<const FloatLiteralNode *>(node));
//     case NodeType::StringNode:
//         return visit(static_cast<const StringNode *>(node));
//     case NodeType::CharNode:
//         return visit(static_cast<const CharNode *>(node));
//     case NodeType::BoolLiteralNode:
//         return visit(static_cast<const BoolLiteralNode *>(node));
//     case NodeType::NullLiteralNode:
//         return visit(static_cast<const NullLiteralNode *>(node));
//     default:
//         throw CodegenException("NodeType: " + std::to_string(static_cast<int>(node->ntype)) + " is not codegenable");
//     }
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::StmtNode *node)
// {
//     switch (node->ntype)
//     {
//     case NodeType::BlockNode:
//         return visit(static_cast<const BlockNode *>(node));
//     case NodeType::StmtGroupNode:
//         return visit(static_cast<const StmtGroupNode *>(node));
//     case NodeType::ExprStmtNode:
//         return visit(static_cast<const ExprStmtNode *>(node));
//     case NodeType::LetDeclNode:
//         return visit(static_cast<const LetDeclNode *>(node));
//     case NodeType::FunctionDeclNode:
//         return visit(static_cast<const FunctionDeclNode *>(node));
//     case NodeType::StructDefNode:
//         return visit(static_cast<const StructDefNode *>(node));
//     case NodeType::RegionDefNode:
//         return visit(static_cast<const RegionDefNode *>(node));
//     case NodeType::UnionDefNode:
//         return visit(static_cast<const UnionDefNode *>(node));
//     case NodeType::FunctionDefNode:
//         return visit(static_cast<const FunctionDefNode *>(node));
//     case NodeType::FunctionParamNode:
//         return visit(static_cast<const FunctionParamNode *>(node));
//     case NodeType::ExportNode:
//         return visit(static_cast<const ExportNode *>(node));
//     case NodeType::InlineAsmNode:
//         return visit(static_cast<const InlineAsmNode *>(node));
//     case NodeType::ReturnStmtNode:
//         return visit(static_cast<const ReturnStmtNode *>(node));
//     case NodeType::IfStmtNode:
//         return visit(static_cast<const IfStmtNode *>(node));
//     case NodeType::WhileStmtNode:
//         return visit(static_cast<const WhileStmtNode *>(node));
//     default:
//         throw CodegenException("NodeType: " + std::to_string(static_cast<int>(node->ntype)) + " is not codegenable");
//     }
// }

// std::string libquixcc::C11CodegenVisitor::visit(const libquixcc::TypeNode *node)
// {
//     switch (node->ntype)
//     {
//     case NodeType::MutTypeNode:
//         return visit(static_cast<const MutTypeNode *>(node));
//     case NodeType::U8TypeNode:
//         return visit(static_cast<const U8TypeNode *>(node));
//     case NodeType::U16TypeNode:
//         return visit(static_cast<const U16TypeNode *>(node));
//     case NodeType::U32TypeNode:
//         return visit(static_cast<const U32TypeNode *>(node));
//     case NodeType::U64TypeNode:
//         return visit(static_cast<const U64TypeNode *>(node));
//     case NodeType::I8TypeNode:
//         return visit(static_cast<const I8TypeNode *>(node));
//     case NodeType::I16TypeNode:
//         return visit(static_cast<const I16TypeNode *>(node));
//     case NodeType::I32TypeNode:
//         return visit(static_cast<const I32TypeNode *>(node));
//     case NodeType::I64TypeNode:
//         return visit(static_cast<const I64TypeNode *>(node));
//     case NodeType::F32TypeNode:
//         return visit(static_cast<const F32TypeNode *>(node));
//     case NodeType::F64TypeNode:
//         return visit(static_cast<const F64TypeNode *>(node));
//     case NodeType::BoolTypeNode:
//         return visit(static_cast<const BoolTypeNode *>(node));
//     case NodeType::VoidTypeNode:
//         return visit(static_cast<const VoidTypeNode *>(node));
//     case NodeType::PointerTypeNode:
//         return visit(static_cast<const PointerTypeNode *>(node));
//     case NodeType::StringTypeNode:
//         return visit(static_cast<const StringTypeNode *>(node));
//     case NodeType::EnumTypeNode:
//         return visit(static_cast<const EnumTypeNode *>(node));
//     case NodeType::StructTypeNode:
//         return visit(static_cast<const StructTypeNode *>(node));
//     case NodeType::RegionTypeNode:
//         return visit(static_cast<const RegionTypeNode *>(node));
//     case NodeType::UnionTypeNode:
//         return visit(static_cast<const UnionTypeNode *>(node));
//     case NodeType::ArrayTypeNode:
//         return visit(static_cast<const ArrayTypeNode *>(node));
//     case NodeType::FunctionTypeNode:
//         return visit(static_cast<const FunctionTypeNode *>(node));
//     default:
//         throw CodegenException("NodeType: " + std::to_string(static_cast<int>(node->ntype)) + " is not codegenable");
//     }
// }