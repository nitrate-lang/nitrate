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

#include <core/Exception.h>
#include <lexer/Lex.h>
#include <parsetree/Serialize.h>
#include <parsetree/nodes/AllNodes.h>

#include <functional>
#include <unordered_map>

using namespace libquixcc;
using namespace serialize;

#define DO_EXCEPT 1

#if DO_EXCEPT
#define DEV_THROWING(x) throw std::runtime_error("Not implemented: " x)
#else
#define DEV_THROWING(x)
#endif

#define INDENT_WIDTH 1

std::string escape_string(const std::string &input) {
  std::string output;
  output.reserve(input.length() * 2);

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
      case '\0':
        output += "\\0";
        break;
      default:
        if (ch >= 32 && ch < 127) {
          output += ch;
        } else {
          char hex[5];
          snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
          output += hex;
        }
        break;
    }
  }

  return output;
}

std::string ParseNode::to_string(bool minified) const {
  return ParseTreeSerializer().serialize(this, minified);
}

std::string ParseTreeSerializer::serialize(const ParseNode *node,
                                           bool minified) {
  this->minified = minified;

  o << "(Program \"v1.0\" \"quix:1.0\"";
  dispatch(node);
  o << ')';
  return o.str();
}

void serialize::ParseTreeSerializer::dispatch(const ParseNode *n) {
  switch (n->ntype) {
    case libquixcc::NodeType::ExprStmtNode:
      conv(n->as<ExprStmtNode>());
      break;
    case libquixcc::NodeType::StmtExprNode:
      conv(n->as<StmtExprNode>());
      break;
    case libquixcc::NodeType::NopStmtNode:
      conv(n->as<NopStmtNode>());
      break;
    case libquixcc::NodeType::BlockNode:
      conv(n->as<BlockNode>());
      break;
    case libquixcc::NodeType::StmtGroupNode:
      conv(n->as<StmtGroupNode>());
      break;
    case libquixcc::NodeType::StaticCastExprNode:
      conv(n->as<StaticCastExprNode>());
      break;
    case libquixcc::NodeType::BitCastExprNode:
      conv(n->as<BitCastExprNode>());
      break;
    case libquixcc::NodeType::SignedUpcastExprNode:
      conv(n->as<SignedUpcastExprNode>());
      break;
    case libquixcc::NodeType::UnsignedUpcastExprNode:
      conv(n->as<UnsignedUpcastExprNode>());
      break;
    case libquixcc::NodeType::DowncastExprNode:
      conv(n->as<DowncastExprNode>());
      break;
    case libquixcc::NodeType::UnaryExprNode:
      conv(n->as<UnaryExprNode>());
      break;
    case libquixcc::NodeType::PostUnaryExprNode:
      conv(n->as<PostUnaryExprNode>());
      break;
    case libquixcc::NodeType::BinaryExprNode:
      conv(n->as<BinaryExprNode>());
      break;
    case libquixcc::NodeType::SeqExprNode:
      conv(n->as<SeqExprNode>());
      break;
    case libquixcc::NodeType::CallExprNode:
      conv(n->as<CallExprNode>());
      break;
    case libquixcc::NodeType::ListExprNode:
      conv(n->as<ListExprNode>());
      break;
    case libquixcc::NodeType::AssocExprNode:
      conv(n->as<AssocExprNode>());
      break;
    case libquixcc::NodeType::MemberAccessNode:
      conv(n->as<MemberAccessNode>());
      break;
    case libquixcc::NodeType::IndexNode:
      conv(n->as<IndexNode>());
      break;
    case libquixcc::NodeType::SliceNode:
      conv(n->as<SliceNode>());
      break;
    case libquixcc::NodeType::FStringNode:
      conv(n->as<FStringNode>());
      break;
    case libquixcc::NodeType::ConstUnaryExprNode:
      conv(n->as<ConstUnaryExprNode>());
      break;
    case libquixcc::NodeType::ConstPostUnaryExprNode:
      conv(n->as<ConstPostUnaryExprNode>());
      break;
    case libquixcc::NodeType::ConstBinaryExprNode:
      conv(n->as<ConstBinaryExprNode>());
      break;
    case libquixcc::NodeType::IdentifierNode:
      conv(n->as<IdentifierNode>());
      break;
    case libquixcc::NodeType::MutTypeNode:
      conv(n->as<MutTypeNode>());
      break;
    case libquixcc::NodeType::U8TypeNode:
      conv(n->as<U8TypeNode>());
      break;
    case libquixcc::NodeType::U16TypeNode:
      conv(n->as<U16TypeNode>());
      break;
    case libquixcc::NodeType::U32TypeNode:
      conv(n->as<U32TypeNode>());
      break;
    case libquixcc::NodeType::U64TypeNode:
      conv(n->as<U64TypeNode>());
      break;
    case libquixcc::NodeType::U128TypeNode:
      conv(n->as<U128TypeNode>());
      break;
    case libquixcc::NodeType::I8TypeNode:
      conv(n->as<I8TypeNode>());
      break;
    case libquixcc::NodeType::I16TypeNode:
      conv(n->as<I16TypeNode>());
      break;
    case libquixcc::NodeType::I32TypeNode:
      conv(n->as<I32TypeNode>());
      break;
    case libquixcc::NodeType::I64TypeNode:
      conv(n->as<I64TypeNode>());
      break;
    case libquixcc::NodeType::I128TypeNode:
      conv(n->as<I128TypeNode>());
      break;
    case libquixcc::NodeType::F32TypeNode:
      conv(n->as<F32TypeNode>());
      break;
    case libquixcc::NodeType::F64TypeNode:
      conv(n->as<F64TypeNode>());
      break;
    case libquixcc::NodeType::BoolTypeNode:
      conv(n->as<BoolTypeNode>());
      break;
    case libquixcc::NodeType::VoidTypeNode:
      conv(n->as<VoidTypeNode>());
      break;
    case libquixcc::NodeType::NullTypeNode:
      conv(n->as<NullTypeNode>());
      break;
    case libquixcc::NodeType::PointerTypeNode:
      conv(n->as<PointerTypeNode>());
      break;
    case libquixcc::NodeType::OpaqueTypeNode:
      conv(n->as<OpaqueTypeNode>());
      break;
    case libquixcc::NodeType::StringTypeNode:
      conv(n->as<StringTypeNode>());
      break;
    case libquixcc::NodeType::EnumTypeNode:
      conv(n->as<EnumTypeNode>());
      break;
    case libquixcc::NodeType::StructTypeNode:
      conv(n->as<StructTypeNode>());
      break;
    case libquixcc::NodeType::GroupTypeNode:
      conv(n->as<GroupTypeNode>());
      break;
    case libquixcc::NodeType::RegionTypeNode:
      conv(n->as<RegionTypeNode>());
      break;
    case libquixcc::NodeType::UnionTypeNode:
      conv(n->as<UnionTypeNode>());
      break;
    case libquixcc::NodeType::ArrayTypeNode:
      conv(n->as<ArrayTypeNode>());
      break;
    case libquixcc::NodeType::VectorTypeNode:
      conv(n->as<VectorTypeNode>());
      break;
    case libquixcc::NodeType::MapTypeNode:
      conv(n->as<MapTypeNode>());
      break;
    case libquixcc::NodeType::ResultTypeNode:
      conv(n->as<ResultTypeNode>());
      break;
    case libquixcc::NodeType::FunctionTypeNode:
      conv(n->as<FunctionTypeNode>());
      break;
    case libquixcc::NodeType::UserTypeNode:
      conv(n->as<UserTypeNode>());
      break;
    case libquixcc::NodeType::IntegerNode:
      conv(n->as<IntegerNode>());
      break;
    case libquixcc::NodeType::FloatLiteralNode:
      conv(n->as<FloatLiteralNode>());
      break;
    case libquixcc::NodeType::StringNode:
      conv(n->as<StringNode>());
      break;
    case libquixcc::NodeType::CharNode:
      conv(n->as<CharNode>());
      break;
    case libquixcc::NodeType::BoolLiteralNode:
      conv(n->as<BoolLiteralNode>());
      break;
    case libquixcc::NodeType::NullLiteralNode:
      conv(n->as<NullLiteralNode>());
      break;
    case libquixcc::NodeType::UndefLiteralNode:
      conv(n->as<UndefLiteralNode>());
      break;
    case libquixcc::NodeType::TypedefNode:
      conv(n->as<TypedefNode>());
      break;
    case libquixcc::NodeType::VarDeclNode:
      conv(n->as<VarDeclNode>());
      break;
    case libquixcc::NodeType::LetDeclNode:
      conv(n->as<LetDeclNode>());
      break;
    case libquixcc::NodeType::ConstDeclNode:
      conv(n->as<ConstDeclNode>());
      break;
    case libquixcc::NodeType::FunctionDeclNode:
      conv(n->as<FunctionDeclNode>());
      break;
    case libquixcc::NodeType::StructDefNode:
      conv(n->as<StructDefNode>());
      break;
    case libquixcc::NodeType::StructFieldNode:
      conv(n->as<StructFieldNode>());
      break;
    case libquixcc::NodeType::RegionDefNode:
      conv(n->as<RegionDefNode>());
      break;
    case libquixcc::NodeType::RegionFieldNode:
      conv(n->as<RegionFieldNode>());
      break;
    case libquixcc::NodeType::GroupDefNode:
      conv(n->as<GroupDefNode>());
      break;
    case libquixcc::NodeType::GroupFieldNode:
      conv(n->as<GroupFieldNode>());
      break;
    case libquixcc::NodeType::UnionDefNode:
      conv(n->as<UnionDefNode>());
      break;
    case libquixcc::NodeType::UnionFieldNode:
      conv(n->as<UnionFieldNode>());
      break;
    case libquixcc::NodeType::EnumDefNode:
      conv(n->as<EnumDefNode>());
      break;
    case libquixcc::NodeType::EnumFieldNode:
      conv(n->as<EnumFieldNode>());
      break;
    case libquixcc::NodeType::FunctionDefNode:
      conv(n->as<FunctionDefNode>());
      break;
    case libquixcc::NodeType::FunctionParamNode:
      conv(n->as<FunctionParamNode>());
      break;
    case libquixcc::NodeType::SubsystemNode:
      conv(n->as<SubsystemNode>());
      break;
    case libquixcc::NodeType::ExportNode:
      conv(n->as<ExportNode>());
      break;
    case libquixcc::NodeType::InlineAsmNode:
      conv(n->as<InlineAsmNode>());
      break;
    case libquixcc::NodeType::ReturnStmtNode:
      conv(n->as<ReturnStmtNode>());
      break;
    case libquixcc::NodeType::RetifStmtNode:
      conv(n->as<RetifStmtNode>());
      break;
    case libquixcc::NodeType::RetzStmtNode:
      conv(n->as<RetzStmtNode>());
      break;
    case libquixcc::NodeType::RetvStmtNode:
      conv(n->as<RetvStmtNode>());
      break;
    case libquixcc::NodeType::BreakStmtNode:
      conv(n->as<BreakStmtNode>());
      break;
    case libquixcc::NodeType::ContinueStmtNode:
      conv(n->as<ContinueStmtNode>());
      break;
    case libquixcc::NodeType::IfStmtNode:
      conv(n->as<IfStmtNode>());
      break;
    case libquixcc::NodeType::WhileStmtNode:
      conv(n->as<WhileStmtNode>());
      break;
    case libquixcc::NodeType::ForStmtNode:
      conv(n->as<ForStmtNode>());
      break;
    case libquixcc::NodeType::FormStmtNode:
      conv(n->as<FormStmtNode>());
      break;
    case libquixcc::NodeType::ForeachStmtNode:
      conv(n->as<ForeachStmtNode>());
      break;
    case libquixcc::NodeType::CaseStmtNode:
      conv(n->as<CaseStmtNode>());
      break;
    case libquixcc::NodeType::SwitchStmtNode:
      conv(n->as<SwitchStmtNode>());
      break;

    default:
      LOG(FATAL) << "No conversion function for node type " << (int)n->ntype
                 << " found." << std::endl;
      throw core::Exception();
  }
}

void ParseTreeSerializer::ind() {
  if (minified) return;

  o << '\n' + std::string(indent * INDENT_WIDTH, ' ');
}

void ParseTreeSerializer::conv(const ExprStmtNode *n) {
  indent++;
  ind();

  o << "(EStmt";
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const StmtExprNode *n) {
  indent++;
  ind();

  o << "(SExpr";
  next(n->m_stmt);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const NopStmtNode *n) { (void)n; }

void ParseTreeSerializer::conv(const BlockNode *n) {
  indent++;

  ind();

  o << "(Blk [";

  for (const auto &stmt : n->m_stmts) {
    next(stmt);
  }

  o << "]";

  if (n->m_unsafe) {
    o << " 1";
  } else {
    o << " 0";
  }

  o << ')';

  indent--;
}

void ParseTreeSerializer::conv(const StmtGroupNode *n) {
  for (const auto &stmt : n->m_stmts) {
    next(stmt);
  }
}

void ParseTreeSerializer::conv(const StaticCastExprNode *n) {
  indent++;
  ind();

  o << "(SCast";
  next(n->m_type);
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const BitCastExprNode *n) {
  indent++;
  ind();

  o << "(BitCast";
  next(n->m_type);
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const SignedUpcastExprNode *n) {
  indent++;
  ind();

  o << "(SUpcast";
  next(n->m_type);
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const UnsignedUpcastExprNode *n) {
  indent++;
  ind();

  o << "(UUpcast";
  next(n->m_type);
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const DowncastExprNode *n) {
  indent++;
  ind();

  o << "(Trunc";
  next(n->m_type);
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const UnaryExprNode *n) {
  indent++;
  ind();

  o << "(UExpr \"" << operator_map_inverse.at(n->m_op) << "\"";
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const PostUnaryExprNode *n) {
  indent++;
  ind();

  o << "(PExpr \"" << operator_map_inverse.at(n->m_op) << "\"";
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const BinaryExprNode *n) {
  indent++;
  ind();

  o << "(BExpr \"" << operator_map_inverse.at(n->m_op) << "\"";
  next(n->m_lhs);
  next(n->m_rhs);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const SeqExprNode *n) {
  indent++;
  ind();

  o << "(Seq";
  next(n->m_lhs);
  next(n->m_rhs);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const CallExprNode *n) {
  indent++;
  ind();

  o << "(Call";
  next(n->m_callee);

  o << " [";

  for (const auto &arg : n->m_positional_args) {
    next(arg);
  }

  o << "] [";

  for (const auto &arg : n->m_named_args) {
    indent++;
    ind();

    o << "(Narg \"" << escape_string(arg.first) << "\"";
    next(arg.second);
    o << ')';
    indent--;
  }

  o << "])";

  indent--;
}

void ParseTreeSerializer::conv(const ListExprNode *n) {
  indent++;
  ind();

  o << "(List [";
  for (const auto &elem : n->m_elements) {
    next(elem);
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const AssocExprNode *n) {
  indent++;
  ind();

  o << "(Assoc [";
  for (const auto &elem : n->m_elements) {
    indent++;
    ind();

    o << "(AssocE";
    next(elem.first);
    next(elem.second);
    o << ')';
    indent--;
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const MemberAccessNode *n) {
  indent++;
  ind();

  o << "(Get \"" << escape_string(n->m_field) << "\"";
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const IndexNode *n) {
  indent++;
  ind();

  o << "(Idx";
  next(n->m_expr);
  next(n->m_index);
  o << ')';
}

void ParseTreeSerializer::conv(const SliceNode *n) {
  indent++;
  ind();

  o << "(Slice";
  next(n->m_expr);
  next(n->m_start);
  next(n->m_end);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FStringNode *n) {
  indent++;
  ind();

  o << "(FStr \"" << escape_string(n->template_string) << "\" [";

  for (const auto &arg : n->args) {
    next(arg);
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const ConstUnaryExprNode *n) {
  indent++;
  ind();

  o << "(CUExpr \"" << operator_map_inverse.at(n->m_op) << "\"";
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const ConstPostUnaryExprNode *n) {
  indent++;
  ind();

  o << "(CPExpr \"" << operator_map_inverse.at(n->m_op) << "\"";
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const ConstBinaryExprNode *n) {
  indent++;
  ind();

  o << "(CBExpr \"" << operator_map_inverse.at(n->m_op) << "\"";
  next(n->m_lhs);
  next(n->m_rhs);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const IdentifierNode *n) {
  indent++;
  ind();
  o << "(Id \"" << escape_string(n->m_name) << "\")";
  indent--;
}

void ParseTreeSerializer::conv(const MutTypeNode *n) {
  indent++;
  ind();
  o << "(Mut";
  next(n->m_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const U8TypeNode *n) {
  indent++;
  ind();
  o << "(U8)";
  indent--;
}

void ParseTreeSerializer::conv(const U16TypeNode *n) {
  indent++;
  ind();
  o << "(U16)";
  indent--;
}

void ParseTreeSerializer::conv(const U32TypeNode *n) {
  indent++;
  ind();
  o << "(U32)";
  indent--;
}

void ParseTreeSerializer::conv(const U64TypeNode *n) {
  indent++;
  ind();
  o << "(U64)";
  indent--;
}

void ParseTreeSerializer::conv(const U128TypeNode *n) {
  indent++;
  ind();
  o << "(U128)";
  indent--;
}

void ParseTreeSerializer::conv(const I8TypeNode *n) {
  indent++;
  ind();
  o << "(I8)";
  indent--;
}

void ParseTreeSerializer::conv(const I16TypeNode *n) {
  indent++;
  ind();
  o << "(I16)";
  indent--;
}

void ParseTreeSerializer::conv(const I32TypeNode *n) {
  indent++;
  ind();
  o << "(I32)";
  indent--;
}

void ParseTreeSerializer::conv(const I64TypeNode *n) {
  indent++;
  ind();
  o << "(I64)";
  indent--;
}

void ParseTreeSerializer::conv(const I128TypeNode *n) {
  indent++;
  ind();
  o << "(I128)";
  indent--;
}

void ParseTreeSerializer::conv(const F32TypeNode *n) {
  indent++;
  ind();
  o << "(f32)";
  indent--;
}

void ParseTreeSerializer::conv(const F64TypeNode *n) {
  indent++;
  ind();
  o << "(f64)";
  indent--;
}

void ParseTreeSerializer::conv(const BoolTypeNode *n) {
  indent++;
  ind();
  o << "(I1)";
  indent--;
}

void ParseTreeSerializer::conv(const VoidTypeNode *n) {
  indent++;
  ind();
  o << "(Void)";
  indent--;
}

void ParseTreeSerializer::conv(const NullTypeNode *n) {
  indent++;
  ind();
  o << "(Null)";
  indent--;
}

void ParseTreeSerializer::conv(const PointerTypeNode *n) {
  indent++;
  ind();

  o << "(Ptr";
  next(n->m_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const OpaqueTypeNode *n) {
  indent++;
  ind();
  o << "(Opaque \"" << escape_string(n->m_name) << "\")";
  indent--;
}

void ParseTreeSerializer::conv(const StringTypeNode *n) {
  indent++;
  ind();
  o << "(String)";
  indent--;
}

void ParseTreeSerializer::conv(const EnumTypeNode *n) {
  indent++;
  ind();

  o << "(Enum \"" << escape_string(n->m_name) << "\"";
  next(n->m_member_type);

  o << ')';
}

void ParseTreeSerializer::conv(const StructTypeNode *n) {
  indent++;
  ind();

  o << "(Struct \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const GroupTypeNode *n) {
  indent++;
  ind();

  o << "(Group \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const RegionTypeNode *n) {
  indent++;
  ind();

  o << "(Region \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const UnionTypeNode *n) {
  indent++;
  ind();

  o << "(Union \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const ArrayTypeNode *n) {
  indent++;
  ind();

  o << "(Arr";
  next(n->m_type);
  next(n->m_size);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const VectorTypeNode *n) {
  indent++;
  ind();

  o << "(Vec";
  next(n->m_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const MapTypeNode *n) {
  indent++;
  ind();

  o << "(Map";
  next(n->m_key_type);
  next(n->m_value_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const ResultTypeNode *n) {
  indent++;
  ind();

  o << "(Result";
  next(n->m_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FunctionTypeNode *n) {
  indent++;
  ind();

  o << "(Func ";
  if (n->m_variadic) {
    o << "1 ";
  } else {
    o << "0 ";
  }
  if (n->m_pure) {
    o << "1 ";
  } else {
    o << "0 ";
  }
  if (n->m_thread_safe) {
    o << "1 ";
  } else {
    o << "0 ";
  }
  if (n->m_foreign) {
    o << "1 ";
  } else {
    o << "0 ";
  }
  if (n->m_noexcept) {
    o << "1 ";
  } else {
    o << "0 ";
  }

  next(n->m_return_type);

  o << " [";

  for (const auto &param : n->m_params) {
    indent++;
    ind();

    o << "(Param \"" << escape_string(param.first) << "\" ";
    next(param.second);
    o << ')';
    indent--;
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const UserTypeNode *n) {
  indent++;
  ind();

  o << "(Name \"" << escape_string(n->m_name) << "\")";
  indent--;
}

void ParseTreeSerializer::conv(const IntegerNode *n) {
  indent++;
  ind();

  o << "(ILit \"" << escape_string(n->m_val) << "\"";
  next(n->m_val_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FloatLiteralNode *n) {
  indent++;
  ind();

  o << "(FLit \"" << escape_string(n->m_val) << "\"";
  next(n->m_val_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const StringNode *n) {
  indent++;
  ind();

  o << "(SLit \"" << escape_string(n->m_val) << "\")";
  indent--;
}

void ParseTreeSerializer::conv(const CharNode *n) {
  indent++;
  ind();

  o << "(CLit \"" << escape_string(n->m_val) << "\")";
  indent--;
}

void ParseTreeSerializer::conv(const BoolLiteralNode *n) {
  indent++;
  ind();

  o << "(BLit ";
  if (n->m_val) {
    o << "1";
  } else {
    o << "0";
  }

  o << ')';

  indent--;
}

void ParseTreeSerializer::conv(const NullLiteralNode *n) {
  indent++;
  ind();

  o << "(Null)";
  indent--;
}

void ParseTreeSerializer::conv(const UndefLiteralNode *n) {
  indent++;
  ind();

  o << "(Undef)";
  indent--;
}

void ParseTreeSerializer::conv(const TypedefNode *n) {
  indent++;
  ind();

  o << "(TypeDef \"" << escape_string(n->m_name) << "\"";
  next(n->m_orig);

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const VarDeclNode *n) {
  indent++;
  ind();

  o << "(Var \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_init);

  if (n->m_is_deprecated) {
    o << " 1";
  } else {
    o << " 0";
  }

  if (n->m_is_mut) {
    o << " 1";
  } else {
    o << " 0";
  }

  if (n->m_is_static) {
    o << " 1";
  } else {
    o << " 0";
  }

  if (n->m_is_thread_local) {
    o << " 1";
  } else {
    o << " 0";
  }

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const LetDeclNode *n) {
  indent++;
  ind();

  o << "(Let \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_init);

  if (n->m_is_deprecated) {
    o << " 1";
  } else {
    o << " 0";
  }

  if (n->m_is_mut) {
    o << " 1";
  } else {
    o << " 0";
  }

  if (n->m_is_static) {
    o << " 1";
  } else {
    o << " 0";
  }

  if (n->m_is_thread_local) {
    o << " 1";
  } else {
    o << " 0";
  }

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const ConstDeclNode *n) {
  indent++;
  ind();

  o << "(Const \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_init);

  if (n->m_is_deprecated) {
    o << " 1";
  } else {
    o << " 0";
  }

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FunctionDeclNode *n) {
  indent++;
  ind();

  o << "(FuncDecl \"" << escape_string(n->m_name) << "\" ";

  if (n->m_type->m_variadic) {
    o << "1 ";
  } else {
    o << "0 ";
  }

  if (n->m_type->m_pure) {
    o << "1 ";
  } else {
    o << "0 ";
  }

  if (n->m_type->m_thread_safe) {
    o << "1 ";
  } else {
    o << "0 ";
  }

  if (n->m_type->m_foreign) {
    o << "1 ";
  } else {
    o << "0 ";
  }

  if (n->m_type->m_noexcept) {
    o << "1";
  } else {
    o << "0";
  }

  next(n->m_type->m_return_type);

  o << " [";

  for (const auto &param : n->m_params) {
    next(param);
  }

  o << "])";

  indent--;
}

void ParseTreeSerializer::conv(const StructDefNode *n) {
  indent++;
  ind();

  o << "(StructDef \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "] [";

  for (const auto &method : n->m_methods) {
    next(method);
  }

  o << "] [";

  for (const auto &static_method : n->m_static_methods) {
    next(static_method);
  }

  o << "] [";

  for (size_t i = 0; i < n->m_implements.size(); i++) {
    o << "\"" << escape_string(n->m_implements[i]) << "\"";
    if (i != n->m_implements.size() - 1) {
      o << " ";
    }
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const StructFieldNode *n) {
  indent++;
  ind();

  o << "(SField \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_value);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const RegionDefNode *n) {
  indent++;
  ind();

  o << "(RegionDef \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "] [";

  for (const auto &method : n->m_methods) {
    next(method);
  }

  o << "] [";

  for (const auto &static_method : n->m_static_methods) {
    next(static_method);
  }

  o << "] [";

  for (size_t i = 0; i < n->m_implements.size(); i++) {
    o << "\"" << escape_string(n->m_implements[i]) << "\"";
    if (i != n->m_implements.size() - 1) {
      o << " ";
    }
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const RegionFieldNode *n) {
  indent++;
  ind();

  o << "(RField \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_value);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const GroupDefNode *n) {
  indent++;
  ind();

  o << "(GroupDef \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "] [";

  for (const auto &method : n->m_methods) {
    next(method);
  }

  o << "] [";

  for (const auto &static_method : n->m_static_methods) {
    next(static_method);
  }

  o << "] [";

  for (size_t i = 0; i < n->m_implements.size(); i++) {
    o << "\"" << escape_string(n->m_implements[i]) << "\"";
    if (i != n->m_implements.size() - 1) {
      o << " ";
    }
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const GroupFieldNode *n) {
  indent++;
  ind();

  o << "(GField \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_value);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const UnionDefNode *n) {
  indent++;
  ind();

  o << "(UnionDef \"" << escape_string(n->m_name) << "\" [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "] [";

  for (const auto &method : n->m_methods) {
    next(method);
  }

  o << "] [";

  for (const auto &static_method : n->m_static_methods) {
    next(static_method);
  }

  o << "] [";

  for (size_t i = 0; i < n->m_implements.size(); i++) {
    o << "\"" << escape_string(n->m_implements[i]) << "\"";
    if (i != n->m_implements.size() - 1) {
      o << " ";
    }
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const UnionFieldNode *n) {
  indent++;
  ind();

  o << "(UField \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_value);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const EnumDefNode *n) {
  indent++;
  ind();

  o << "(EnumDef \"" << escape_string(n->m_type->m_name) << "\"";

  next(n->m_type->m_member_type);

  o << " [";

  for (const auto &field : n->m_fields) {
    next(field);
  }

  o << "]";

  o << ')';

  indent--;
}

void ParseTreeSerializer::conv(const EnumFieldNode *n) {
  indent++;
  ind();

  o << "(EField \"" << escape_string(n->m_name) << "\"";
  next(n->m_value);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FunctionDefNode *n) {
  indent++;
  ind();

  o << "(FuncDef";
  next(n->m_decl);
  next(n->m_body);
  next(n->m_req_in);
  next(n->m_req_out);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FunctionParamNode *n) {
  indent++;
  ind();

  o << "(FParam \"" << escape_string(n->m_name) << "\"";
  next(n->m_type);
  next(n->m_value);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const SubsystemNode *n) {
  indent++;
  ind();

  o << "(Subsystem \"" << escape_string(n->m_name) << "\" [";
  for (auto it = n->m_deps.begin(); it != n->m_deps.end(); ++it) {
    o << "\"" << escape_string(*it) << "\"";
    if (std::next(it) != n->m_deps.end()) {
      o << ' ';
    }
  }

  o << "]";
  next(n->m_block);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const ExportNode *n) {
  indent++;
  ind();

  o << "(Export ";
  switch (n->m_lang_type) {
    case ExportLangType::C:
      o << "\"C\"";
      break;
    case ExportLangType::CXX:
      o << "\"C++\"";
      break;
    case ExportLangType::Default:
      o << "\"Default\"";
      break;
    case ExportLangType::DLang:
      o << "\"D\"";
      break;
    case ExportLangType::None:
      o << "\"None\"";
      break;
  }

  o << " [";

  for (const auto &item : n->m_stmts) {
    next(item);
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const InlineAsmNode *n) {
  indent++;
  ind();

  o << "(Asm ";
  o << "\"" << escape_string(n->m_asm) << "\" [";

  for (const auto &item : n->m_outputs) {
    indent++;
    ind();

    o << "(Out \"" << escape_string(item.first) << "\"";
    next(item.second);
    o << ')';
    indent--;
  }

  o << "] [";

  for (const auto &item : n->m_inputs) {
    indent++;
    ind();

    o << "(In \"" << escape_string(item.first) << "\"";
    next(item.second);
    o << ')';
    indent--;
  }

  o << "] [";

  for (auto it = n->m_clobbers.begin(); it != n->m_clobbers.end(); ++it) {
    o << "\"" << escape_string(*it) << "\"";
    if (std::next(it) != n->m_clobbers.end()) {
      o << ' ';
    }
  }

  o << "])";
  indent--;
}

void ParseTreeSerializer::conv(const ReturnStmtNode *n) {
  indent++;
  ind();

  o << "(Ret";
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const RetifStmtNode *n) {
  indent++;
  ind();

  o << "(Retif";
  next(n->m_cond);
  next(n->m_return);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const RetzStmtNode *n) {
  indent++;
  ind();

  o << "(Retz";
  next(n->m_cond);
  next(n->m_return);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const RetvStmtNode *n) {
  indent++;
  ind();

  o << "(Retv";
  next(n->m_cond);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const BreakStmtNode *n) {
  indent++;
  ind();

  o << "(Break)";
  indent--;
}

void ParseTreeSerializer::conv(const ContinueStmtNode *n) {
  indent++;
  ind();

  o << "(Continue)";
  indent--;
}

void ParseTreeSerializer::conv(const IfStmtNode *n) {
  indent++;
  ind();

  o << "(If";
  next(n->m_cond);
  next(n->m_then);
  next(n->m_else);

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const WhileStmtNode *n) {
  indent++;
  ind();

  o << "(While";
  next(n->m_cond);
  next(n->m_stmt);

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const ForStmtNode *n) {
  indent++;
  ind();

  o << "(For";
  next(n->m_init);
  next(n->m_cond);
  next(n->m_step);
  next(n->m_stmt);

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FormStmtNode *n) {
  indent++;
  ind();

  o << "(Form \"" + escape_string(n->m_var) + "\"";
  next(n->m_range);
  next(n->m_block);

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const ForeachStmtNode *n) {
  indent++;
  ind();

  o << "(Foreach \"" + escape_string(n->m_var) + "\"";
  next(n->m_range);
  next(n->m_block);

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const CaseStmtNode *n) {
  indent++;
  ind();

  o << "(Case";
  next(n->m_expr);
  next(n->m_block);

  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const SwitchStmtNode *n) {
  indent++;
  ind();

  o << "(Switch";
  next(n->m_expr);

  o << " [";

  for (const auto &case_stmt : n->m_cases) {
    next(case_stmt);
  }

  o << ']';

  next(n->m_default);

  o << ')';
  indent--;
}