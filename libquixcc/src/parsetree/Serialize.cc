////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <lexer/Lex.h>
#include <parsetree/Serialize.h>
#include <parsetree/nodes/AllNodes.h>

#include <functional>
#include <unordered_map>

using namespace libquixcc;

std::string escape_json(const std::string &input) {
  std::string output;
  output.reserve(input.length() *
                 2);  // Reserving enough space for escaped characters

  for (char ch : input) {
    switch (ch) {
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
        if (ch < ' ') {
          // Unicode escape sequence
          char hex[7];
          snprintf(hex, sizeof(hex), "\\u%04x", ch);
          output += hex;
        } else {
          output += ch;
        }
        break;
    }
  }

  return output;
}

std::string ParseNode::to_json(
    serialize::ParseTreeSerializerState state) const {
  return serialize::ParseTreeSerializer::next(state, this);
}

std::string serialize::ParseTreeSerializer::dispatch(
    serialize::ParseTreeSerializerState &state, const ParseNode *node) {
  typedef std::string (*Func)(serialize::ParseTreeSerializerState &,
                              const ParseNode *);

  static std::unordered_map<NodeType, Func> node_map = {
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
      {NodeType::PostUnaryExprNode, (Func)PostUnaryExprNode_conv},
      {NodeType::BinaryExprNode, (Func)BinaryExprNode_conv},
      {NodeType::CallExprNode, (Func)CallExprNode_conv},
      {NodeType::ListExprNode, (Func)ListExprNode_conv},
      {NodeType::MemberAccessNode, (Func)MemberAccessNode_conv},
      {NodeType::IndexNode, (Func)IndexNode_conv},
      {NodeType::ConstUnaryExprNode, (Func)ConstUnaryExprNode_conv},
      {NodeType::ConstPostUnaryExprNode, (Func)ConstPostUnaryExprNode_conv},
      {NodeType::ConstBinaryExprNode, (Func)ConstBinaryExprNode_conv},
      {NodeType::IdentifierNode, (Func)IdentifierNode_conv},
      {NodeType::MutTypeNode, (Func)MutTypeNode_conv},
      {NodeType::U8TypeNode, (Func)U8TypeNode_conv},
      {NodeType::U16TypeNode, (Func)U16TypeNode_conv},
      {NodeType::U32TypeNode, (Func)U32TypeNode_conv},
      {NodeType::U64TypeNode, (Func)U64TypeNode_conv},
      {NodeType::U128TypeNode, (Func)U128TypeNode_conv},
      {NodeType::I8TypeNode, (Func)I8TypeNode_conv},
      {NodeType::I16TypeNode, (Func)I16TypeNode_conv},
      {NodeType::I32TypeNode, (Func)I32TypeNode_conv},
      {NodeType::I64TypeNode, (Func)I64TypeNode_conv},
      {NodeType::I128TypeNode, (Func)I128TypeNode_conv},
      {NodeType::F32TypeNode, (Func)F32TypeNode_conv},
      {NodeType::F64TypeNode, (Func)F64TypeNode_conv},
      {NodeType::BoolTypeNode, (Func)BoolTypeNode_conv},
      {NodeType::VoidTypeNode, (Func)VoidTypeNode_conv},
      {NodeType::NullTypeNode, (Func)NullTypeNode_conv},
      {NodeType::PointerTypeNode, (Func)PointerTypeNode_conv},
      {NodeType::OpaqueTypeNode, (Func)OpaqueTypeNode_conv},
      {NodeType::StringTypeNode, (Func)StringTypeNode_conv},
      {NodeType::EnumTypeNode, (Func)EnumTypeNode_conv},
      {NodeType::StructTypeNode, (Func)StructTypeNode_conv},
      {NodeType::GroupTypeNode, (Func)GroupTypeNode_conv},
      {NodeType::RegionTypeNode, (Func)RegionTypeNode_conv},
      {NodeType::UnionTypeNode, (Func)UnionTypeNode_conv},
      {NodeType::ArrayTypeNode, (Func)ArrayTypeNode_conv},
      {NodeType::VectorTypeNode, (Func)VectorTypeNode_conv},
      {NodeType::ResultTypeNode, (Func)ResultTypeNode_conv},
      {NodeType::GeneratorTypeNode, (Func)GeneratorTypeNode_conv},
      {NodeType::FunctionTypeNode, (Func)FunctionTypeNode_conv},
      {NodeType::UserTypeNode, (Func)UserTypeNode_conv},
      {NodeType::IntegerNode, (Func)IntegerNode_conv},
      {NodeType::FloatLiteralNode, (Func)FloatLiteralNode_conv},
      {NodeType::StringNode, (Func)StringNode_conv},
      {NodeType::CharNode, (Func)CharNode_conv},
      {NodeType::BoolLiteralNode, (Func)BoolLiteralNode_conv},
      {NodeType::NullLiteralNode, (Func)NullLiteralNode_conv},
      {NodeType::UndefLiteralNode, (Func)UndefLiteralNode_conv},
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
    LOG(FATAL) << "No conversion function for node type " << (int)node->ntype
               << " found." << std::endl;

  return node_map[node->ntype](state, node);
}

std::string serialize::ParseTreeSerializer::ExprStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const ExprStmtNode *node) {
  return "{\"ntype\":\"ExprStmtNode\",\"expr\":" + next(state, node->m_expr) +
         "}";
}

std::string serialize::ParseTreeSerializer::NopStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const NopStmtNode *node) {
  return "{\"ntype\":\"nop\"}";
}

std::string serialize::ParseTreeSerializer::BlockNode_conv(
    serialize::ParseTreeSerializerState &state, const BlockNode *node) {
  std::string str = "{\"ntype\":\"BlockNode\",\"children\":[";
  for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_stmts.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::StmtGroupNode_conv(
    serialize::ParseTreeSerializerState &state, const StmtGroupNode *node) {
  std::string str;

  for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_stmts.end() - 1) {
      str += ",";
    }
  }

  return str;
}

std::string serialize::ParseTreeSerializer::StaticCastExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const StaticCastExprNode *node) {
  std::string str = "{\"ntype\":\"StaticCastExprNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"expr\":";
  str += next(state, node->m_expr);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::BitCastExprNode_conv(
    serialize::ParseTreeSerializerState &state, const BitCastExprNode *node) {
  std::string str = "{\"ntype\":\"BitCastExprNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"expr\":";
  str += next(state, node->m_expr);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::SignedUpcastExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const SignedUpcastExprNode *node) {
  std::string str = "{\"ntype\":\"SignedUpcastExprNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"expr\":";
  str += next(state, node->m_expr);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::UnsignedUpcastExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const UnsignedUpcastExprNode *node) {
  std::string str = "{\"ntype\":\"UnsignedUpcastExprNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"expr\":";
  str += next(state, node->m_expr);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::DowncastExprNode_conv(
    serialize::ParseTreeSerializerState &state, const DowncastExprNode *node) {
  std::string str = "{\"ntype\":\"DowncastExprNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"expr\":";
  str += next(state, node->m_expr);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::PtrToIntCastExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const PtrToIntCastExprNode *node) {
  std::string str = "{\"ntype\":\"PtrToIntCastExprNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"expr\":";
  str += next(state, node->m_expr);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::IntToPtrCastExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const IntToPtrCastExprNode *node) {
  std::string str = "{\"ntype\":\"IntToPtrCastExprNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"expr\":";
  str += next(state, node->m_expr);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::UnaryExprNode_conv(
    serialize::ParseTreeSerializerState &state, const UnaryExprNode *node) {
  std::string str = "{\"ntype\":\"UnaryExprNode\",\"op\":\"";
  str += operator_map_inverse.at(node->m_op);
  str += "\",\"operand\":";
  str += next(state, node->m_expr);

  return str + "}";
}

std::string serialize::ParseTreeSerializer::PostUnaryExprNode_conv(
    serialize::ParseTreeSerializerState &state, const PostUnaryExprNode *node) {
  std::string str = "{\"ntype\":\"PostUnaryExprNode\",\"op\":\"";
  str += operator_map_inverse.at(node->m_op);
  str += "\",\"operand\":";
  str += next(state, node->m_expr);

  return str + "}";
}

std::string serialize::ParseTreeSerializer::BinaryExprNode_conv(
    serialize::ParseTreeSerializerState &state, const BinaryExprNode *node) {
  std::string str = "{\"ntype\":\"BinaryExprNode\",\"op\":\"";
  str += operator_map_inverse.at(node->m_op);
  str += "\",\"lhs\":";
  str += next(state, node->m_lhs);
  str += ",\"rhs\":";
  str += next(state, node->m_rhs);

  return str + "}";
}

std::string serialize::ParseTreeSerializer::CallExprNode_conv(
    serialize::ParseTreeSerializerState &state, const CallExprNode *node) {
  std::string str = "{\"ntype\":\"CallExprNode\",\"calle\":";
  str += next(state, node->m_callee);
  str += ",\"named_args\":[";
  for (auto it = node->m_named_args.begin(); it != node->m_named_args.end();
       ++it) {
    str += "{\"name\":\"" + escape_json(it->first) +
           "\",\"value\":" + next(state, it->second) + "}";
    if (it != node->m_named_args.end() - 1) {
      str += ",";
    }
  }

  str += "],\"positional_args\":[";
  for (auto it = node->m_positional_args.begin();
       it != node->m_positional_args.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_positional_args.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::ListExprNode_conv(
    serialize::ParseTreeSerializerState &state, const ListExprNode *node) {
  std::string str = "{\"ntype\":\"ListExprNode\",\"elements\":[";
  for (auto it = node->m_elements.begin(); it != node->m_elements.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_elements.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::MemberAccessNode_conv(
    serialize::ParseTreeSerializerState &state, const MemberAccessNode *node) {
  std::string str = "{\"ntype\":\"MemberAccessNode\",\"lhs\":";
  str += next(state, node->m_expr);
  str += ",\"field\":\"" + escape_json(node->m_field) + "\"";

  return str + "}";
}

std::string serialize::ParseTreeSerializer::IndexNode_conv(
    serialize::ParseTreeSerializerState &state, const IndexNode *node) {
  std::string str = "{\"ntype\":\"IndexNode\",\"lhs\":";
  str += next(state, node->m_expr);
  str += ",\"index\":";
  str += next(state, node->m_index);

  return str + "}";
}

std::string serialize::ParseTreeSerializer::ConstUnaryExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const ConstUnaryExprNode *node) {
  std::string str = "{\"ntype\":\"ConstUnaryExprNode\",\"op\":\"";
  str += operator_map_inverse.at(node->m_op);
  str += "\",\"operand\":";
  str += next(state, node->m_expr);

  return str + "}";
}

std::string serialize::ParseTreeSerializer::ConstPostUnaryExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const ConstPostUnaryExprNode *node) {
  std::string str = "{\"ntype\":\"ConstPostUnaryExprNode\",\"op\":\"";
  str += operator_map_inverse.at(node->m_op);
  str += "\",\"operand\":";
  str += next(state, node->m_expr);

  return str + "}";
}

std::string serialize::ParseTreeSerializer::ConstBinaryExprNode_conv(
    serialize::ParseTreeSerializerState &state,
    const ConstBinaryExprNode *node) {
  std::string str = "{\"ntype\":\"ConstBinaryExprNode\",\"op\":\"";
  str += operator_map_inverse.at(node->m_op);
  str += "\",\"lhs\":";
  str += next(state, node->m_lhs);
  str += ",\"rhs\":";
  str += next(state, node->m_rhs);

  return str + "}";
}

std::string serialize::ParseTreeSerializer::IdentifierNode_conv(
    serialize::ParseTreeSerializerState &state, const IdentifierNode *node) {
  return "{\"ntype\":\"IdentifierNode\",\"name\":\"" +
         escape_json(node->m_name) + "\"}";
}

std::string serialize::ParseTreeSerializer::MutTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const MutTypeNode *node) {
  return "{\"ntype\":\"MutTypeNode\",\"type\":" + next(state, node->m_type) +
         "}";
}

std::string serialize::ParseTreeSerializer::U8TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const U8TypeNode *node) {
  return "{\"ntype\":\"U8TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::U16TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const U16TypeNode *node) {
  return "{\"ntype\":\"U16TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::U32TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const U32TypeNode *node) {
  return "{\"ntype\":\"U32TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::U64TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const U64TypeNode *node) {
  return "{\"ntype\":\"U64TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::U128TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const U128TypeNode *node) {
  return "{\"ntype\":\"U128TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::I8TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const I8TypeNode *node) {
  return "{\"ntype\":\"I8TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::I16TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const I16TypeNode *node) {
  return "{\"ntype\":\"I16TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::I32TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const I32TypeNode *node) {
  return "{\"ntype\":\"I32TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::I64TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const I64TypeNode *node) {
  return "{\"ntype\":\"I64TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::I128TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const I128TypeNode *node) {
  return "{\"ntype\":\"I128TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::F32TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const F32TypeNode *node) {
  return "{\"ntype\":\"F32TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::F64TypeNode_conv(
    serialize::ParseTreeSerializerState &state, const F64TypeNode *node) {
  return "{\"ntype\":\"F64TypeNode\"}";
}

std::string serialize::ParseTreeSerializer::BoolTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const BoolTypeNode *node) {
  return "{\"ntype\":\"BoolTypeNode\"}";
}

std::string serialize::ParseTreeSerializer::VoidTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const VoidTypeNode *node) {
  return "{\"ntype\":\"VoidTypeNode\"}";
}

std::string serialize::ParseTreeSerializer::NullTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const NullTypeNode *node) {
  return "{\"ntype\":\"NullTypeNode\"}";
}

std::string serialize::ParseTreeSerializer::PointerTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const PointerTypeNode *node) {
  std::string str = "{\"ntype\":\"PointerTypeNode\",\"type\":";
  str += next(state, node->m_type);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::OpaqueTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const OpaqueTypeNode *node) {
  return "{\"ntype\":\"OpaqueTypeNode\",\"name\":\"" +
         escape_json(node->m_name) + "\"}";
}

std::string serialize::ParseTreeSerializer::StringTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const StringTypeNode *node) {
  return "{\"ntype\":\"StringTypeNode\"}";
}

std::string serialize::ParseTreeSerializer::EnumTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const EnumTypeNode *node) {
  std::string str = "{\"ntype\":\"EnumTypeNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"member_type\":";
  if (node->m_member_type)
    str += next(state, node->m_member_type);
  else
    str += "null";
  return str + "}";
}

std::string serialize::ParseTreeSerializer::StructTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const StructTypeNode *node) {
  if (state.m_visited.contains(node))
    return "{\"ntype\":\"StructTypeNode\",\"name\":\"" +
           escape_json(node->m_name) + "\"}";

  state.m_visited.insert(node);  // Prevent infinite recursion

  std::string str = "{\"ntype\":\"StructTypeNode\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  str += "],\"name\":\"" + escape_json(node->m_name) + "\"";

  return str + "}";
}

std::string serialize::ParseTreeSerializer::GroupTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const GroupTypeNode *node) {
  if (state.m_visited.contains(node))
    return "{\"ntype\":\"GroupTypeNode\",\"name\":\"" +
           escape_json(node->m_name) + "\"}";

  state.m_visited.insert(node);  // Prevent infinite recursion

  std::string str = "{\"ntype\":\"GroupTypeNode\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::RegionTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const RegionTypeNode *node) {
  if (state.m_visited.contains(node))
    return "{\"ntype\":\"RegionTypeNode\",\"name\":\"" +
           escape_json(node->m_name) + "\"}";

  state.m_visited.insert(node);  // Prevent infinite recursion

  std::string str = "{\"ntype\":\"RegionTypeNode\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::UnionTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const UnionTypeNode *node) {
  if (state.m_visited.contains(node))
    return "{\"ntype\":\"UnionTypeNode\",\"name\":\"" +
           escape_json(node->m_name) + "\"}";

  state.m_visited.insert(node);  // Prevent infinite recursion

  std::string str = "{\"ntype\":\"UnionTypeNode\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::ArrayTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const ArrayTypeNode *node) {
  std::string str = "{\"ntype\":\"ArrayTypeNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"size\":";
  str += next(state, node->m_size);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::VectorTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const VectorTypeNode *node) {
  std::string str = "{\"ntype\":\"VectorTypeNode\",\"type\":";
  str += next(state, node->m_type);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::ResultTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const ResultTypeNode *node) {
  std::string str = "{\"ntype\":\"ResultTypeNode\",\"type\":";
  str += next(state, node->m_type);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::GeneratorTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const GeneratorTypeNode *node) {
  std::string str = "{\"ntype\":\"GeneratorTypeNode\",\"type\":";
  str += next(state, node->m_type);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::FunctionTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const FunctionTypeNode *node) {
  std::string str = "{\"ntype\":\"FunctionTypeNode\",\"params\":[";
  for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it) {
    str += next(state, (*it).second);
    if (it != node->m_params.end() - 1) {
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
  str += ",\"noexcept\":";
  str += std::string(node->m_noexcept ? "true" : "false");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::UserTypeNode_conv(
    serialize::ParseTreeSerializerState &state, const UserTypeNode *node) {
  return "{\"ntype\":\"UserTypeNode\",\"name\":\"" + escape_json(node->m_name) +
         "\"}";
}

std::string serialize::ParseTreeSerializer::IntegerNode_conv(
    serialize::ParseTreeSerializerState &state, const IntegerNode *node) {
  return "{\"ntype\":\"IntegerNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string serialize::ParseTreeSerializer::FloatLiteralNode_conv(
    serialize::ParseTreeSerializerState &state, const FloatLiteralNode *node) {
  return "{\"ntype\":\"FloatLiteralNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string serialize::ParseTreeSerializer::StringNode_conv(
    serialize::ParseTreeSerializerState &state, const StringNode *node) {
  return "{\"ntype\":\"StringNode\",\"value\":\"" + escape_json(node->m_val) +
         "\"}";
}

std::string serialize::ParseTreeSerializer::CharNode_conv(
    serialize::ParseTreeSerializerState &state, const CharNode *node) {
  return "{\"ntype\":\"CharNode\",\"value\":\"" + node->m_val + "\"}";
}

std::string serialize::ParseTreeSerializer::BoolLiteralNode_conv(
    serialize::ParseTreeSerializerState &state, const BoolLiteralNode *node) {
  return "{\"ntype\":\"BoolLiteralNode\",\"value\":\"" +
         std::string(node->m_val ? "true" : "false") + "\"}";
}

std::string serialize::ParseTreeSerializer::NullLiteralNode_conv(
    serialize::ParseTreeSerializerState &state, const NullLiteralNode *node) {
  return "{\"ntype\":\"NullLiteralNode\"}";
}

std::string serialize::ParseTreeSerializer::UndefLiteralNode_conv(
    serialize::ParseTreeSerializerState &state, const UndefLiteralNode *node) {
  return "{\"ntype\":\"UndefLiteralNode\"}";
}

std::string serialize::ParseTreeSerializer::TypedefNode_conv(
    serialize::ParseTreeSerializerState &state, const TypedefNode *node) {
  std::string str = "{\"ntype\":\"TypedefNode\",\"base\":";
  str += next(state, node->m_orig);
  str += ",\"name\":\"" + escape_json(node->m_name);
  return str + "\"}";
}

std::string serialize::ParseTreeSerializer::VarDeclNode_conv(
    serialize::ParseTreeSerializerState &state, const VarDeclNode *node) {
  state.m_visited.clear();

  std::string str = "{\"ntype\":\"VarDeclNode\",\"name\":\"";
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
  str += ",\"mutable\":";
  str += std::string(node->m_is_mut ? "true" : "false");
  str += ",\"static\":";
  str += std::string(node->m_is_static ? "true" : "false");
  str += ",\"thread_local\":";
  str += std::string(node->m_is_thread_local ? "true" : "false");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::LetDeclNode_conv(
    serialize::ParseTreeSerializerState &state, const LetDeclNode *node) {
  state.m_visited.clear();

  std::string str = "{\"ntype\":\"LetDeclNode\",\"name\":\"";
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
  str += ",\"mutable\":";
  str += std::string(node->m_is_mut ? "true" : "false");
  str += ",\"static\":";
  str += std::string(node->m_is_static ? "true" : "false");
  str += ",\"thread_local\":";
  str += std::string(node->m_is_thread_local ? "true" : "false");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::ConstDeclNode_conv(
    serialize::ParseTreeSerializerState &state, const ConstDeclNode *node) {
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

std::string serialize::ParseTreeSerializer::FunctionDeclNode_conv(
    serialize::ParseTreeSerializerState &state, const FunctionDeclNode *node) {
  state.m_visited.clear();

  std::string str = "{\"ntype\":\"FunctionDeclNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"params\":[";
  for (auto it = node->m_params.begin(); it != node->m_params.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_params.end() - 1) {
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
  str += ",\"noexcept\":";
  str += std::string(node->m_type->m_noexcept ? "true" : "false");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::StructDefNode_conv(
    serialize::ParseTreeSerializerState &state, const StructDefNode *node) {
  std::string str = "{\"ntype\":\"StructDefNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  str += "],\"methods\":[";
  for (auto it = node->m_methods.begin(); it != node->m_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_methods.end() - 1) {
      str += ",";
    }
  }
  str += "],\"static_methods\":[";
  for (auto it = node->m_static_methods.begin();
       it != node->m_static_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_static_methods.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::StructFieldNode_conv(
    serialize::ParseTreeSerializerState &state, const StructFieldNode *node) {
  std::string str = "{\"ntype\":\"StructFieldNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"value\":";
  str += (node->m_value ? next(state, node->m_value) : "null");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::RegionDefNode_conv(
    serialize::ParseTreeSerializerState &state, const RegionDefNode *node) {
  std::string str = "{\"ntype\":\"RegionDefNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  str += "],\"methods\":[";
  for (auto it = node->m_methods.begin(); it != node->m_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_methods.end() - 1) {
      str += ",";
    }
  }
  str += "],\"static_methods\":[";
  for (auto it = node->m_static_methods.begin();
       it != node->m_static_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_static_methods.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::RegionFieldNode_conv(
    serialize::ParseTreeSerializerState &state, const RegionFieldNode *node) {
  std::string str = "{\"ntype\":\"RegionFieldNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"value\":";
  str += (node->m_value ? next(state, node->m_value) : "null");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::GroupDefNode_conv(
    serialize::ParseTreeSerializerState &state, const GroupDefNode *node) {
  std::string str = "{\"ntype\":\"GroupDefNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  str += "],\"methods\":[";
  for (auto it = node->m_methods.begin(); it != node->m_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_methods.end() - 1) {
      str += ",";
    }
  }
  str += "],\"static_methods\":[";
  for (auto it = node->m_static_methods.begin();
       it != node->m_static_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_static_methods.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::GroupFieldNode_conv(
    serialize::ParseTreeSerializerState &state, const GroupFieldNode *node) {
  std::string str = "{\"ntype\":\"GroupFieldNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"value\":";
  str += (node->m_value ? next(state, node->m_value) : "null");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::UnionDefNode_conv(
    serialize::ParseTreeSerializerState &state, const UnionDefNode *node) {
  std::string str = "{\"ntype\":\"UnionDefNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  str += "],\"methods\":[";
  for (auto it = node->m_methods.begin(); it != node->m_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_methods.end() - 1) {
      str += ",";
    }
  }
  str += "],\"static_methods\":[";
  for (auto it = node->m_static_methods.begin();
       it != node->m_static_methods.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_static_methods.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::UnionFieldNode_conv(
    serialize::ParseTreeSerializerState &state, const UnionFieldNode *node) {
  std::string str = "{\"ntype\":\"UnionFieldNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"value\":";
  str += (node->m_value ? next(state, node->m_value) : "null");
  return str + "}";
}

std::string serialize::ParseTreeSerializer::EnumDefNode_conv(
    serialize::ParseTreeSerializerState &state, const EnumDefNode *node) {
  std::string str = "{\"ntype\":\"EnumDefNode\",\"type\":";
  str += next(state, node->m_type);
  str += ",\"fields\":[";
  for (auto it = node->m_fields.begin(); it != node->m_fields.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_fields.end() - 1) {
      str += ",";
    }
  }

  return str + "]}";
}

std::string serialize::ParseTreeSerializer::EnumFieldNode_conv(
    serialize::ParseTreeSerializerState &state, const EnumFieldNode *node) {
  std::string str = "{\"ntype\":\"EnumFieldNode\",\"name\":\"";
  str += escape_json(node->m_name);
  if (node->m_value) {
    str += "\",\"value\":";
    str += next(state, node->m_value);
  }
  return str + "}";
}

std::string serialize::ParseTreeSerializer::FunctionDefNode_conv(
    serialize::ParseTreeSerializerState &state, const FunctionDefNode *node) {
  std::string str = "{\"ntype\":\"FunctionDefNode\",\"decl\":";
  str += next(state, node->m_decl);
  str += ",\"body\":";
  str += next(state, node->m_body);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::FunctionParamNode_conv(
    serialize::ParseTreeSerializerState &state, const FunctionParamNode *node) {
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

std::string serialize::ParseTreeSerializer::SubsystemNode_conv(
    serialize::ParseTreeSerializerState &state, const SubsystemNode *node) {
  std::string str = "{\"ntype\":\"SubsystemNode\",\"name\":\"";
  str += escape_json(node->m_name);
  str += "\",\"deps\":[";
  for (auto it = node->m_deps.begin(); it != node->m_deps.end(); ++it) {
    str += "\"" + escape_json(*it) + "\"";
    if (it != node->m_deps.end()) {
      str += ",";
    }
  }

  str += "],\"block\":" + next(state, node->m_block);
  return str + "}";
}

std::string serialize::ParseTreeSerializer::ExportNode_conv(
    serialize::ParseTreeSerializerState &state, const ExportNode *node) {
  std::string str = "{\"ntype\":\"ExportNode\",\"block\":[";
  for (auto it = node->m_stmts.begin(); it != node->m_stmts.end(); ++it) {
    str += next(state, *it);
    if (it != node->m_stmts.end() - 1) {
      str += ",";
    }
  }
  return str + "]}";
}

std::string serialize::ParseTreeSerializer::InlineAsmNode_conv(
    serialize::ParseTreeSerializerState &state, const InlineAsmNode *node) {
  std::string str = "{\"ntype\":\"InlineAsmNode\",\"asm\":\"";
  str += escape_json(node->m_asm);
  str += "\",\"outputs\":[";
  for (auto it = node->m_outputs.begin(); it != node->m_outputs.end(); ++it) {
    str += next(state, (*it).second);

    if (it != node->m_outputs.end()) {
      str += ",";
    }
  }

  str += "],\"inputs\":[";

  for (auto it = node->m_inputs.begin(); it != node->m_inputs.end(); ++it) {
    str += next(state, (*it).second);

    if (it != node->m_inputs.end()) {
      str += ",";
    }
  }

  str += "],\"clobbers\":[";

  for (auto it = node->m_clobbers.begin(); it != node->m_clobbers.end(); ++it) {
    str += *it;

    if (it != node->m_clobbers.end() - 1) {
      str += ",";
    }
  }

  str += "]}";

  return str;
}

std::string serialize::ParseTreeSerializer::ReturnStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const ReturnStmtNode *node) {
  if (node->m_expr)
    return "{\"ntype\":\"ReturnStmtNode\",\"value\":" +
           next(state, node->m_expr) + "}";
  else
    return "{\"ntype\":\"ReturnStmtNode\"}";
}

std::string serialize::ParseTreeSerializer::RetifStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const RetifStmtNode *node) {
  return "{\"ntype\":\"RetifStmtNode\",\"cond\":" + next(state, node->m_cond) +
         ",\"value\":" + next(state, node->m_return) + "}";
}

std::string serialize::ParseTreeSerializer::RetzStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const RetzStmtNode *node) {
  return "{\"ntype\":\"RetzStmtNode\",\"cond\":" + next(state, node->m_cond) +
         ",\"value\":" + next(state, node->m_return) + "}";
}

std::string serialize::ParseTreeSerializer::RetvStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const RetvStmtNode *node) {
  return "{\"ntype\":\"RetvStmtNode\",\"cond\":" + next(state, node->m_cond) +
         "}";
}

std::string serialize::ParseTreeSerializer::IfStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const IfStmtNode *node) {
  std::string str =
      "{\"ntype\":\"IfStmtNode\",\"cond\":" + next(state, node->m_cond) +
      ",\"then\":" + next(state, node->m_then);
  if (node->m_else) {
    str += ",\"else\":" + next(state, node->m_else);
  }
  return str + "}";
}

std::string serialize::ParseTreeSerializer::WhileStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const WhileStmtNode *node) {
  return "{\"ntype\":\"WhileStmtNode\",\"cond\":" + next(state, node->m_cond) +
         ",\"block\":" + next(state, node->m_stmt) + "}";
}

std::string serialize::ParseTreeSerializer::ForStmtNode_conv(
    serialize::ParseTreeSerializerState &state, const ForStmtNode *node) {
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