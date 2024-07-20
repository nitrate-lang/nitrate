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

#include <core/Macro.h>
#include <cstring>
#include <libdeflate.h>
#include <qast/Nodes.h>
#include <quixcc/Library.h>
#include <quixcc/interface/SyntaxTreeNodes.h>
#include <quixcc/lexer/Lex.h>
#include <sstream>
#include <string>

using namespace libquixcc;
using namespace libquixcc::qast;

struct ConvState {
  int32_t indent;
  size_t indent_width;
  bool minify;
};

typedef std::basic_stringstream<char, std::char_traits<char>, Arena<char>> ConvStream;

static String escape_string(const String &input) {
  String output = "\"";
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

  output += "\"";

  return output;
}

static void indent(ConvStream &ss, ConvState &state) {
  if (state.minify) {
    return;
  }

  if (state.indent > 0) {
    ss << "\n";
  }

  ss << std::string(state.indent * state.indent_width, ' ');
}

#define OBJECT_BEGIN(__name)                                                                       \
  state.indent++;                                                                                  \
  indent(ss, state);                                                                               \
  ss << "(" __name;

#define OBJECT_SUB(__field) serialize_recurse(__field, ss, state);
#define OBJECT_STR(__field)                                                                        \
  state.indent++;                                                                                  \
  indent(ss, state);                                                                               \
  state.indent--;                                                                                  \
  ss << escape_string(__field);
#define OBJECT_NUM(__field)                                                                        \
  state.indent++;                                                                                  \
  indent(ss, state);                                                                               \
  state.indent--;                                                                                  \
  ss << __field;

#define OBJECT_ALI(__field)                                                                        \
  state.indent++;                                                                                  \
  indent(ss, state);                                                                               \
  state.indent--;                                                                                  \
  ss << "\"" << __field << "\"";

#define OBJECT_ARRAY(__field)                                                                      \
  ss << "[";                                                                                       \
  for (auto it = __field.begin(); it != __field.end(); ++it) {                                     \
    serialize_recurse(*it, ss, state);                                                             \
    if (std::next(it) != __field.end()) {                                                          \
      ss << ",";                                                                                   \
    }                                                                                              \
  }                                                                                                \
  ss << "]";

#define OBJECT_MAP(__field)                                                                        \
  ss << "{";                                                                                       \
  for (const auto &[key, value] : __field) {                                                       \
    serialize_recurse(key, ss, state);                                                             \
    ss << " => ";                                                                                  \
    serialize_recurse(value, ss, state);                                                           \
  }                                                                                                \
  ss << "}";

#define OBJECT_TAGS(__field)                                                                       \
  state.indent++;                                                                                  \
  indent(ss, state);                                                                               \
  state.indent--;                                                                                  \
  ss << "[";                                                                                       \
  for (auto it = __field.begin(); it != __field.end(); ++it) {                                     \
    ss << escape_string(*it);                                                                      \
    if (std::next(it) != __field.end()) {                                                          \
      ss << ",";                                                                                   \
    }                                                                                              \
  }                                                                                                \
  ss << "]";

#define OBJECT_END()                                                                               \
  ss << ")";                                                                                       \
  state.indent--;

#define OBJECT_EMPTY(__name)                                                                       \
  state.indent++;                                                                                  \
  indent(ss, state);                                                                               \
  ss << "(" __name ")";                                                                            \
  state.indent--;

static void serialize_recurse(Node *n, ConvStream &ss, ConvState &state) {
  if (!n) {
    // Nicely handle null nodes
    OBJECT_EMPTY("Nil");
    return;
  }

  switch (n->this_typeid()) {
  case QUIXCC_AST_NODE_BINEXPR: {
    OBJECT_BEGIN("BExpr");
    OBJECT_ALI(n->as<BinExpr>()->get_op());
    OBJECT_SUB(n->as<BinExpr>()->get_lhs());
    OBJECT_SUB(n->as<BinExpr>()->get_rhs());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_UNEXPR: {
    OBJECT_BEGIN("UExpr");
    OBJECT_ALI(n->as<UnaryExpr>()->get_op());
    OBJECT_SUB(n->as<UnaryExpr>()->get_rhs());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_TEREXPR: {
    OBJECT_BEGIN("TExpr");
    OBJECT_SUB(n->as<TernaryExpr>()->get_cond());
    OBJECT_SUB(n->as<TernaryExpr>()->get_lhs());
    OBJECT_SUB(n->as<TernaryExpr>()->get_rhs());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_INT: {
    OBJECT_BEGIN("Int");
    OBJECT_STR(n->as<ConstInt>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FLOAT: {
    OBJECT_BEGIN("Float");
    OBJECT_STR(n->as<ConstFloat>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_STRING: {
    OBJECT_BEGIN("String");
    OBJECT_STR(n->as<ConstString>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_CHAR: {
    OBJECT_BEGIN("Char");
    OBJECT_STR(n->as<ConstChar>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_BOOL: {
    OBJECT_BEGIN("Bool");
    OBJECT_NUM(n->as<ConstBool>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_NULL: {
    OBJECT_EMPTY("Null");
    break;
  }
  case QUIXCC_AST_NODE_UNDEF: {
    OBJECT_EMPTY("Undef");
    break;
  }
  case QUIXCC_AST_NODE_CALL: {
    OBJECT_BEGIN("Call");
    OBJECT_SUB(n->as<Call>()->get_func());
    ss << "[";
    for (const auto &[k, v] : n->as<Call>()->get_args()) {
      indent(ss, state);

      ss << "(Param";
      state.indent++;
      indent(ss, state);
      ss << escape_string(k);
      state.indent--;

      serialize_recurse(v, ss, state);
      ss << ")";
    }
    ss << "]";
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_LIST: {
    OBJECT_BEGIN("List");
    OBJECT_ARRAY(n->as<List>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_ASSOC: {
    OBJECT_BEGIN("Assoc");
    OBJECT_SUB(n->as<Assoc>()->get_key());
    OBJECT_SUB(n->as<Assoc>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FIELD: {
    OBJECT_BEGIN("Field");
    OBJECT_STR(n->as<Field>()->get_field());
    OBJECT_SUB(n->as<Field>()->get_base());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_INDEX: {
    OBJECT_BEGIN("Idx");
    OBJECT_SUB(n->as<Index>()->get_base());
    OBJECT_SUB(n->as<Index>()->get_index());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_SLICE: {
    OBJECT_BEGIN("Slice");
    OBJECT_SUB(n->as<Slice>()->get_base());
    OBJECT_SUB(n->as<Slice>()->get_start());
    OBJECT_SUB(n->as<Slice>()->get_end());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FSTRING: {
    OBJECT_BEGIN("FString");
    OBJECT_STR(n->as<FString>()->get_value());
    OBJECT_ARRAY(n->as<FString>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_IDENT: {
    OBJECT_BEGIN("Name");
    OBJECT_STR(n->as<Ident>()->get_name());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_SEQ_POINT: {
    OBJECT_BEGIN("Seq");
    OBJECT_ARRAY(n->as<SeqPoint>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_POST_UNEXPR: {
    OBJECT_BEGIN("PExpr");
    OBJECT_ALI(n->as<PostUnaryExpr>()->get_op());
    OBJECT_SUB(n->as<PostUnaryExpr>()->get_lhs());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_STMT_EXPR: {
    OBJECT_BEGIN("SExpr");
    OBJECT_SUB(n->as<StmtExpr>()->get_stmt());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_TYPE_EXPR: {
    OBJECT_BEGIN("TExpr");
    OBJECT_SUB(n->as<TypeExpr>()->get_type());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_MUT_TY: {
    OBJECT_BEGIN("Mut");
    OBJECT_SUB(n->as<MutTy>()->get_item());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_U1_TY: {
    OBJECT_EMPTY("U1");
    break;
  }
  case QUIXCC_AST_NODE_U8_TY: {
    OBJECT_EMPTY("U8");
    break;
  }
  case QUIXCC_AST_NODE_U16_TY: {
    OBJECT_EMPTY("U16");
    break;
  }
  case QUIXCC_AST_NODE_U32_TY: {
    OBJECT_EMPTY("U32");
    break;
  }
  case QUIXCC_AST_NODE_U64_TY: {
    OBJECT_EMPTY("U64");
    break;
  }
  case QUIXCC_AST_NODE_U128_TY: {
    OBJECT_EMPTY("U128");
    break;
  }
  case QUIXCC_AST_NODE_I8_TY: {
    OBJECT_EMPTY("I8");
    break;
  }
  case QUIXCC_AST_NODE_I16_TY: {
    OBJECT_EMPTY("I16");
    break;
  }
  case QUIXCC_AST_NODE_I32_TY: {
    OBJECT_EMPTY("I32");
    break;
  }
  case QUIXCC_AST_NODE_I64_TY: {
    OBJECT_EMPTY("I64");
    break;
  }
  case QUIXCC_AST_NODE_I128_TY: {
    OBJECT_EMPTY("I128");
    break;
  }
  case QUIXCC_AST_NODE_F32_TY: {
    OBJECT_EMPTY("F32");
    break;
  }
  case QUIXCC_AST_NODE_F64_TY: {
    OBJECT_EMPTY("F64");
    break;
  }
  case QUIXCC_AST_NODE_VOID_TY: {
    OBJECT_EMPTY("Void");
    break;
  }
  case QUIXCC_AST_NODE_PTR_TY: {
    OBJECT_BEGIN("Ptr");
    OBJECT_SUB(n->as<PtrTy>()->get_item());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_OPAQUE_TY: {
    OBJECT_BEGIN("Opaque");
    OBJECT_STR(n->as<OpaqueTy>()->get_name());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_STRING_TY: {
    OBJECT_EMPTY("String");
    break;
  }
  case QUIXCC_AST_NODE_ENUM_TY: {
    OBJECT_BEGIN("Enum");
    OBJECT_STR(n->as<EnumTy>()->get_name());
    OBJECT_SUB(n->as<EnumTy>()->get_memtype());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_STRUCT_TY: {
    OBJECT_BEGIN("Struct");
    for (const auto &[k, v] : n->as<StructTy>()->get_items()) {
      indent(ss, state);
      ss << "(Field" << escape_string(k);
      serialize_recurse(v, ss, state);
      ss << ")";
    }
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_GROUP_TY: {
    OBJECT_BEGIN("Group");
    OBJECT_ARRAY(n->as<GroupTy>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_REGION_TY: {
    OBJECT_BEGIN("Region");
    OBJECT_ARRAY(n->as<RegionTy>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_UNION_TY: {
    OBJECT_BEGIN("Union");
    OBJECT_ARRAY(n->as<UnionTy>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_ARRAY_TY: {
    OBJECT_BEGIN("Array");
    OBJECT_SUB(n->as<ArrayTy>()->get_item());
    OBJECT_SUB(n->as<ArrayTy>()->get_size());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_VECTOR_TY: {
    OBJECT_BEGIN("Vec");
    OBJECT_SUB(n->as<VectorTy>()->get_item());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_MAP_TY: {
    OBJECT_BEGIN("Map");
    OBJECT_SUB(n->as<MapTy>()->get_key());
    OBJECT_SUB(n->as<MapTy>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_TUPLE_TY: {
    OBJECT_BEGIN("Tuple");
    OBJECT_ARRAY(n->as<TupleTy>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_SET_TY: {
    OBJECT_BEGIN("Set");
    OBJECT_SUB(n->as<SetTy>()->get_item());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_RESULT_TY: {
    OBJECT_BEGIN("Opt");
    OBJECT_SUB(n->as<OptionalTy>()->get_item());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FN_TY: {
    OBJECT_BEGIN("FType");
    OBJECT_SUB(n->as<FuncTy>()->get_return_ty());
    ss << "[";
    for (const auto &param : n->as<FuncTy>()->get_params()) {
      state.indent++;
      indent(ss, state);

      ss << "(Param";
      state.indent++;
      indent(ss, state);
      ss << escape_string(std::get<0>(param));

      state.indent--;

      serialize_recurse(std::get<1>(param), ss, state);
      serialize_recurse(std::get<2>(param), ss, state);

      state.indent--;
      ss << ")";
    }
    ss << "]";
    state.indent++;
    indent(ss, state);
    state.indent--;
    ss << "\"" << n->as<FuncTy>()->get_purity() << "\"";
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_UNRES_TY: {
    OBJECT_BEGIN("Unres");
    OBJECT_STR(n->as<UnresolvedType>()->get_name());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_TYPEDEF: {
    OBJECT_BEGIN("Typedef");
    OBJECT_STR(n->as<TypedefDecl>()->get_name());
    OBJECT_SUB(n->as<TypedefDecl>()->get_type());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FNDECL: {
    OBJECT_BEGIN("FnDecl");
    OBJECT_STR(n->as<FnDecl>()->get_name());
    OBJECT_SUB(n->as<FnDecl>()->get_type());
    OBJECT_TAGS(n->as<FnDecl>()->get_tags());
    OBJECT_NUM((int)n->as<FnDecl>()->get_visibility());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_STRUCT: {
    OBJECT_BEGIN("Struct");
    ss << "[";
    for (const auto &[k, v] : n->as<StructTy>()->get_items()) {
      indent(ss, state);
      ss << "(Field" << escape_string(k);
      serialize_recurse(v, ss, state);
      ss << ")";
    }
    ss << "]";
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_REGION: {
    OBJECT_BEGIN("Region");
    OBJECT_STR(n->as<RegionDef>()->get_name());
    OBJECT_ARRAY(n->as<RegionDef>()->get_fields());
    OBJECT_ARRAY(n->as<RegionDef>()->get_methods());
    OBJECT_ARRAY(n->as<RegionDef>()->get_static_methods());
    OBJECT_TAGS(n->as<RegionDef>()->get_tags());
    OBJECT_NUM((int)n->as<RegionDef>()->get_visibility());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_GROUP: {
    OBJECT_BEGIN("Group");
    OBJECT_STR(n->as<GroupDef>()->get_name());
    OBJECT_ARRAY(n->as<GroupDef>()->get_fields());
    OBJECT_ARRAY(n->as<GroupDef>()->get_methods());
    OBJECT_ARRAY(n->as<GroupDef>()->get_static_methods());
    OBJECT_TAGS(n->as<GroupDef>()->get_tags());
    OBJECT_NUM((int)n->as<GroupDef>()->get_visibility());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_UNION: {
    OBJECT_BEGIN("Union");
    OBJECT_STR(n->as<UnionDef>()->get_name());
    OBJECT_ARRAY(n->as<UnionDef>()->get_fields());
    OBJECT_ARRAY(n->as<UnionDef>()->get_methods());
    OBJECT_ARRAY(n->as<UnionDef>()->get_static_methods());
    OBJECT_TAGS(n->as<UnionDef>()->get_tags());
    OBJECT_NUM((int)n->as<UnionDef>()->get_visibility());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_ENUM: {
    OBJECT_BEGIN("Enum");
    OBJECT_STR(n->as<EnumDef>()->get_name());
    state.indent++;
    indent(ss, state);
    ss << "[";
    for (const auto &[k, v] : n->as<EnumDef>()->get_items()) {
      indent(ss, state);
      ss << "(Field" << escape_string(k);
      serialize_recurse(v, ss, state);
      ss << ")";
    }
    ss << "]";
    state.indent--;
    OBJECT_TAGS(n->as<EnumDef>()->get_tags());
    OBJECT_NUM((int)n->as<EnumDef>()->get_visibility());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FN: {
    OBJECT_BEGIN("Fn");
    OBJECT_STR(n->as<FnDef>()->get_name());
    OBJECT_SUB(n->as<FnDef>()->get_type());
    OBJECT_SUB(n->as<FnDef>()->get_precond());
    OBJECT_SUB(n->as<FnDef>()->get_postcond());
    OBJECT_TAGS(n->as<FnDef>()->get_tags());
    OBJECT_SUB(n->as<FnDef>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_SUBSYSTEM: {
    OBJECT_BEGIN("Subsystem");
    OBJECT_STR(n->as<SubsystemDecl>()->get_name());
    OBJECT_TAGS(n->as<SubsystemDecl>()->get_tags());
    OBJECT_SUB(n->as<SubsystemDecl>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_EXPORT: {
    OBJECT_BEGIN("Export");
    OBJECT_NUM((int)n->as<ExportDecl>()->get_lang());
    OBJECT_SUB(n->as<ExportDecl>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_COMPOSITE_FIELD: {
    OBJECT_BEGIN("Field");
    OBJECT_STR(n->as<CompositeField>()->get_name());
    OBJECT_SUB(n->as<CompositeField>()->get_type());
    OBJECT_SUB(n->as<CompositeField>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_BLOCK: {
    OBJECT_BEGIN("Blk");
    OBJECT_ARRAY(n->as<Block>()->get_items());
    OBJECT_NUM(n->as<Block>()->is_unsafe());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_CONST: {
    OBJECT_BEGIN("Const");
    OBJECT_STR(n->as<ConstDecl>()->get_name());
    OBJECT_SUB(n->as<ConstDecl>()->get_type());
    OBJECT_SUB(n->as<ConstDecl>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_VAR: {
    OBJECT_BEGIN("Var");
    OBJECT_STR(n->as<VarDecl>()->get_name());
    OBJECT_SUB(n->as<VarDecl>()->get_type());
    OBJECT_SUB(n->as<VarDecl>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_LET: {
    OBJECT_BEGIN("Let");
    OBJECT_STR(n->as<LetDecl>()->get_name());
    OBJECT_SUB(n->as<LetDecl>()->get_type());
    OBJECT_SUB(n->as<LetDecl>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_INLINE_ASM: {
    OBJECT_BEGIN("Asm");
    OBJECT_STR(n->as<InlineAsm>()->get_code());
    OBJECT_ARRAY(n->as<InlineAsm>()->get_args());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_RETURN: {
    OBJECT_BEGIN("Ret");
    OBJECT_SUB(n->as<ReturnStmt>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_RETIF: {
    OBJECT_BEGIN("RetIf");
    OBJECT_SUB(n->as<ReturnIfStmt>()->get_cond());
    OBJECT_SUB(n->as<ReturnIfStmt>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_RETZ: {
    OBJECT_BEGIN("RetZ");
    OBJECT_SUB(n->as<RetZStmt>()->get_cond());
    OBJECT_SUB(n->as<RetZStmt>()->get_value());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_RETV: {
    OBJECT_BEGIN("RetV");
    OBJECT_SUB(n->as<RetVStmt>()->get_cond());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_BREAK: {
    OBJECT_EMPTY("Break");
    break;
  }
  case QUIXCC_AST_NODE_CONTINUE: {
    OBJECT_EMPTY("Continue");
    break;
  }
  case QUIXCC_AST_NODE_IF: {
    OBJECT_BEGIN("If");
    OBJECT_SUB(n->as<IfStmt>()->get_cond());
    OBJECT_SUB(n->as<IfStmt>()->get_then());
    OBJECT_SUB(n->as<IfStmt>()->get_else());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_WHILE: {
    OBJECT_BEGIN("While");
    OBJECT_SUB(n->as<WhileStmt>()->get_cond());
    OBJECT_SUB(n->as<WhileStmt>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FOR: {
    OBJECT_BEGIN("For");
    OBJECT_SUB(n->as<ForStmt>()->get_init());
    OBJECT_SUB(n->as<ForStmt>()->get_cond());
    OBJECT_SUB(n->as<ForStmt>()->get_step());
    OBJECT_SUB(n->as<ForStmt>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FORM: {
    OBJECT_BEGIN("Form");
    OBJECT_SUB(n->as<FormStmt>()->get_init());
    OBJECT_SUB(n->as<FormStmt>()->get_generator());
    OBJECT_SUB(n->as<FormStmt>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_FOREACH: {
    OBJECT_BEGIN("Foreach");
    OBJECT_SUB(n->as<ForeachStmt>()->get_iter());
    OBJECT_SUB(n->as<ForeachStmt>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_CASE: {
    OBJECT_BEGIN("Case");
    OBJECT_SUB(n->as<CaseStmt>()->get_cond());
    OBJECT_SUB(n->as<CaseStmt>()->get_body());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_SWITCH: {
    OBJECT_BEGIN("Switch");
    OBJECT_SUB(n->as<SwitchStmt>()->get_cond());
    OBJECT_ARRAY(n->as<SwitchStmt>()->get_cases());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_SLIST: {
    OBJECT_BEGIN("SList");
    OBJECT_ARRAY(n->as<StmtList>()->get_items());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_EXPR_STMT: {
    OBJECT_BEGIN("EStmt");
    OBJECT_SUB(n->as<ExprStmt>()->get_expr());
    OBJECT_END();
    break;
  }
  case QUIXCC_AST_NODE_STMT:
  case QUIXCC_AST_NODE_TYPE:
  case QUIXCC_AST_NODE_DECL:
  case QUIXCC_AST_NODE_EXPR:
  case QUIXCC_AST_NODE_CEXPR: {
    quixcc_panic("Invalid node type for serialization");
  }
  }
}

static char *quixcc_ast_repr_arena(const quixcc_ast_node_t *_node, bool minify, size_t indent,
                                   quixcc_arena_t *arena, size_t *outlen) {
  quixcc_ast_arena.swap(*arena);

  /* Create a string stream based on the arena */
  ConvStream ss;
  ConvState state = {-1, indent, minify};
  const Node *n = static_cast<const Node *>(_node);

  /* Serialize the AST recursively */

  serialize_recurse(const_cast<Node *>(n), ss, state);

  /**
   * @brief We can do the following because the std::string destructor will
   * invoke the arena's destructor, which is a no-op until the arena itself is
   * destroyed. So we can safely return the string's data pointer knowing it will exists for as long
   * as the arena does.
   */
  std::basic_string<char, std::char_traits<char>, Arena<char>> str = ss.str();
  *outlen = str.size();

  quixcc_ast_arena.swap(*arena);

  return str.data();
}

static char *quixcc_ast_repr_malloc(const quixcc_ast_node_t *_node, bool minify, size_t indent,
                                    size_t *outlen) {
  quixcc_arena_t scratch;
  char *out = nullptr, *out_tmp = nullptr;

  quixcc_arena_open(&scratch);

  try {
    out = quixcc_ast_repr_arena(_node, minify, indent, &scratch, outlen);

    if (out) {
      out_tmp = static_cast<char *>(malloc(*outlen));
      if (!out_tmp) {
        quixcc_panic("Failed to allocate memory for AST representation");
      }

      memcpy(out_tmp, out, *outlen);

      out = out_tmp;
    }

  } catch (...) {
  }

  quixcc_arena_close(&scratch);

  return out;
}

LIB_EXPORT char *quixcc_ast_repr(const quixcc_ast_node_t *_node, bool minify, size_t indent,
                                 quixcc_arena_t *arena, size_t *outlen) {
  size_t outlen_v = 0;

  /* Eliminate internal edge cases */
  if (!outlen) {
    outlen = &outlen_v;
  }

  if (arena) {
    return quixcc_ast_repr_arena(_node, minify, indent, arena, outlen);
  } else {
    return quixcc_ast_repr_malloc(_node, minify, indent, outlen);
  }
}

static void raw_deflate(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size,
                        quixcc_arena_t *arena) {
  struct libdeflate_compressor *ctx{};

  /* Allocate a compressor context; level 8 is a fairly good tradeoff */
  ctx = libdeflate_alloc_compressor(8);

  /* Compute the largest possible compressed buffer size */
  *out_size = libdeflate_deflate_compress_bound(ctx, in_size);

  /* Allocate memory for the compressed buffer */
  *out = (uint8_t *)quixcc_arena_alloc(arena, *out_size);

  if (*out == NULL) {
    libdeflate_free_compressor(ctx);
    quixcc_panic("Failed to allocate memory for compressed AST representation");
  }

  /* Compress the data */
  *out_size = libdeflate_deflate_compress(ctx, in, in_size, *out, *out_size);

  /* Liberate the compressor context */
  libdeflate_free_compressor(ctx);

  /* Check for compression failure */
  if (out_size == 0) {
    quixcc_panic("Failed to compress AST representation");
  }
}

LIB_EXPORT void quixcc_ast_brepr(const quixcc_ast_node_t *node, bool compress,
                                 quixcc_arena_t *arena, uint8_t **out, size_t *outlen) {
  char *repr{};
  quixcc_arena_t scratch{};
  bool our_arena{};

  /* Open a scratch arena if one is not provided */
  if (!arena) {
    quixcc_arena_open(&scratch);
    arena = &scratch;
    our_arena = true;
  }

  /* Validate the output buffer */
  if (!out || !outlen) {
    quixcc_panic("Invalid output buffer for AST representation");
  }

  /* Generate the AST representation as ASCII */
  if ((repr = quixcc_ast_repr(node, true, 0, arena, outlen)) == NULL) {
    quixcc_panic("Failed to generate AST representation");
  }

  /* Compress the AST representation */
  if (compress) {
    uint8_t *tmp_out;
    raw_deflate((const uint8_t *)repr, *outlen, &tmp_out, outlen, arena);
    repr = (char *)tmp_out;
  }

  /* Copy the AST representation to the output buffer, if necessary */
  if (our_arena) {
    *out = (uint8_t *)malloc(*outlen);
    if (!*out) {
      quixcc_panic("Failed to allocate memory for AST representation");
    }

    memcpy(*out, repr, *outlen);
    quixcc_arena_close(arena);
  } else {
    /* Otherwise, just return the pointer to the arena alloc'ed buffer */
    *out = (uint8_t *)repr;
  }
}

std::ostream &std::operator<<(std::ostream &os, const UnaryOp &op) {
  os << libquixcc::operator_map_inverse.at((libquixcc::Operator)op);
  return os;
}

std::ostream &std::operator<<(std::ostream &os, const PostUnaryOp &expr) {
  os << libquixcc::operator_map_inverse.at((libquixcc::Operator)expr);
  return os;
}

std::ostream &std::operator<<(std::ostream &os, const BinOp &op) {
  os << libquixcc::operator_map_inverse.at((libquixcc::Operator)op);
  return os;
}

std::ostream &std::operator<<(std::ostream &os, const libquixcc::qast::FuncPurity &purity) {
  switch (purity) {
  case libquixcc::qast::FuncPurity::IMPURE_THREAD_UNSAFE:
    os << "impure";
    break;
  case libquixcc::qast::FuncPurity::IMPURE_THREAD_SAFE:
    os << "impure tsafe";
    break;
  case libquixcc::qast::FuncPurity::PURE:
    os << "pure";
    break;
  case libquixcc::qast::FuncPurity::QUASIPURE:
    os << "quasipure";
    break;
  case libquixcc::qast::FuncPurity::RETROPURE:
    os << "retropure";
    break;
  }

  return os;
}