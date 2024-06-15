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

std::string ParseNode::to_string() const {
  return ParseTreeSerializer().serialize(this);
}

std::string ParseTreeSerializer::serialize(const ParseNode *node) {
  o << "(Program \"v1.0\" \"quix:1.0\"";
  dispatch(node);
  o << ')';
  return o.str();
}

void serialize::ParseTreeSerializer::dispatch(const ParseNode *n) {
#define match(type) \
  if (n->is<type>()) return conv(n->as<type>())

  match(ExprStmtNode);
  match(StmtExprNode);
  match(NopStmtNode);
  match(BlockNode);
  match(StmtGroupNode);
  match(StaticCastExprNode);
  match(BitCastExprNode);
  match(SignedUpcastExprNode);
  match(UnsignedUpcastExprNode);
  match(DowncastExprNode);
  match(PtrToIntCastExprNode);
  match(IntToPtrCastExprNode);
  match(UnaryExprNode);
  match(PostUnaryExprNode);
  match(BinaryExprNode);
  match(CallExprNode);
  match(ListExprNode);
  match(MemberAccessNode);
  match(IndexNode);
  match(FStringNode);
  match(ConstUnaryExprNode);
  match(ConstPostUnaryExprNode);
  match(ConstBinaryExprNode);
  match(IdentifierNode);
  match(MutTypeNode);
  match(U8TypeNode);
  match(U16TypeNode);
  match(U32TypeNode);
  match(U64TypeNode);
  match(U128TypeNode);
  match(I8TypeNode);
  match(I16TypeNode);
  match(I32TypeNode);
  match(I64TypeNode);
  match(I128TypeNode);
  match(F32TypeNode);
  match(F64TypeNode);
  match(BoolTypeNode);
  match(VoidTypeNode);
  match(NullTypeNode);
  match(PointerTypeNode);
  match(OpaqueTypeNode);
  match(StringTypeNode);
  match(EnumTypeNode);
  match(StructTypeNode);
  match(GroupTypeNode);
  match(RegionTypeNode);
  match(UnionTypeNode);
  match(ArrayTypeNode);
  match(VectorTypeNode);
  match(ResultTypeNode);
  match(GeneratorTypeNode);
  match(FunctionTypeNode);
  match(UserTypeNode);
  match(IntegerNode);
  match(FloatLiteralNode);
  match(StringNode);
  match(CharNode);
  match(BoolLiteralNode);
  match(NullLiteralNode);
  match(UndefLiteralNode);
  match(TypedefNode);
  match(VarDeclNode);
  match(LetDeclNode);
  match(ConstDeclNode);
  match(FunctionDeclNode);
  match(StructDefNode);
  match(StructFieldNode);
  match(RegionDefNode);
  match(RegionFieldNode);
  match(GroupDefNode);
  match(GroupFieldNode);
  match(UnionDefNode);
  match(UnionFieldNode);
  match(EnumDefNode);
  match(EnumFieldNode);
  match(FunctionDefNode);
  match(FunctionParamNode);
  match(SubsystemNode);
  match(ExportNode);
  match(InlineAsmNode);
  match(ReturnStmtNode);
  match(RetifStmtNode);
  match(RetzStmtNode);
  match(RetvStmtNode);
  match(IfStmtNode);
  match(WhileStmtNode);
  match(ForStmtNode);
  match(FormStmtNode);
  match(CaseStmtNode);
  match(SwitchStmtNode);

  LOG(FATAL) << "No conversion function for node type " << (int)n->ntype
             << " found." << std::endl;
}

void ParseTreeSerializer::ind() {
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

void ParseTreeSerializer::conv(const PtrToIntCastExprNode *n) {
  indent++;
  ind();

  o << "(Ptr2Int";
  next(n->m_type);
  next(n->m_expr);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const IntToPtrCastExprNode *n) {
  indent++;
  ind();

  o << "(Int2Ptr";
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
    o << "(\"" << escape_string(arg.first) << "\"";
    next(arg.second);
    o << ')';
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

void ParseTreeSerializer::conv(const ResultTypeNode *n) {
  indent++;
  ind();

  o << "(Result";
  next(n->m_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const GeneratorTypeNode *n) {
  indent++;
  ind();

  o << "(Gen";
  next(n->m_type);
  o << ')';
  indent--;
}

void ParseTreeSerializer::conv(const FunctionTypeNode *n) {
  indent++;
  ind();

  o << "(Func";
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

  if (n->m_scoped) {
    o << " 1";
  } else {
    o << " 0";
  }

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