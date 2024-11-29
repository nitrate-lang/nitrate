////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <libdeflate.h>
#include <nitrate-core/Error.h>
#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Node.h>

#include <ParserStruct.hh>
#include <cstring>
#include <nitrate-core/Classes.hh>
#include <sstream>

#include "LibMacro.h"

using namespace qparse;

struct ConvState {
  int32_t indent;
  size_t indent_width;
  bool minify;
};

typedef std::stringstream ConvStream;

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

#define OBJECT_BEGIN(__name) \
  state.indent++;            \
  indent(ss, state);         \
  ss << "(" __name;

#define OBJECT_SUB(__field) serialize_recurse(__field, ss, state);
#define OBJECT_STR(__field) \
  state.indent++;           \
  indent(ss, state);        \
  state.indent--;           \
  ss << escape_string(__field);
#define OBJECT_NUM(__field) \
  state.indent++;           \
  indent(ss, state);        \
  state.indent--;           \
  ss << __field;

#define OBJECT_ALI(__field) \
  state.indent++;           \
  indent(ss, state);        \
  state.indent--;           \
  ss << "\"" << __field << "\"";

#define OBJECT_ARRAY(__field)                                  \
  ss << "[";                                                   \
  for (auto it = __field.begin(); it != __field.end(); ++it) { \
    serialize_recurse(*it, ss, state);                         \
    if (std::next(it) != __field.end()) {                      \
      ss << ",";                                               \
    }                                                          \
  }                                                            \
  ss << "]";

#define OBJECT_MAP(__field)                  \
  ss << "{";                                 \
  for (const auto &[key, value] : __field) { \
    serialize_recurse(key, ss, state);       \
    ss << " => ";                            \
    serialize_recurse(value, ss, state);     \
  }                                          \
  ss << "}";

#define OBJECT_TAGS(__field)                                   \
  state.indent++;                                              \
  indent(ss, state);                                           \
  state.indent--;                                              \
  ss << "[";                                                   \
  for (auto it = __field.begin(); it != __field.end(); ++it) { \
    ss << escape_string(*it);                                  \
    if (std::next(it) != __field.end()) {                      \
      ss << ",";                                               \
    }                                                          \
  }                                                            \
  ss << "]";

#define OBJECT_END() \
  ss << ")";         \
  state.indent--;

#define OBJECT_EMPTY(__name) \
  state.indent++;            \
  indent(ss, state);         \
  ss << "(" __name ")";      \
  state.indent--;

static void serialize_recurse(Node *n, ConvStream &ss, ConvState &state) {
  if (!n) {
    // Nicely handle null nodes
    OBJECT_EMPTY("Nil");
    return;
  }

  switch (n->this_typeid()) {
    case QAST_NODE_BINEXPR: {
      OBJECT_BEGIN("BExpr");
      OBJECT_ALI(n->as<BinExpr>()->get_op());
      OBJECT_SUB(n->as<BinExpr>()->get_lhs());
      OBJECT_SUB(n->as<BinExpr>()->get_rhs());
      OBJECT_END();
      break;
    }
    case QAST_NODE_UNEXPR: {
      OBJECT_BEGIN("UExpr");
      OBJECT_ALI(n->as<UnaryExpr>()->get_op());
      OBJECT_SUB(n->as<UnaryExpr>()->get_rhs());
      OBJECT_END();
      break;
    }
    case QAST_NODE_TEREXPR: {
      OBJECT_BEGIN("TerExpr");
      OBJECT_SUB(n->as<TernaryExpr>()->get_cond());
      OBJECT_SUB(n->as<TernaryExpr>()->get_lhs());
      OBJECT_SUB(n->as<TernaryExpr>()->get_rhs());
      OBJECT_END();
      break;
    }
    case QAST_NODE_INT: {
      OBJECT_BEGIN("Int");
      OBJECT_STR(n->as<ConstInt>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_FLOAT: {
      OBJECT_BEGIN("Float");
      OBJECT_STR(n->as<ConstFloat>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_STRING: {
      OBJECT_BEGIN("String");
      OBJECT_STR(n->as<ConstString>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_CHAR: {
      OBJECT_BEGIN("Char");
      OBJECT_NUM(n->as<ConstChar>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_BOOL: {
      OBJECT_BEGIN("Bool");
      OBJECT_NUM(n->as<ConstBool>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_NULL: {
      OBJECT_EMPTY("Null");
      break;
    }
    case QAST_NODE_UNDEF: {
      OBJECT_EMPTY("Undef");
      break;
    }
    case QAST_NODE_CALL: {
      OBJECT_BEGIN("Call");
      OBJECT_SUB(n->as<Call>()->get_func());
      ss << "[";
      for (const auto &[k, v] : n->as<Call>()->get_args()) {
        OBJECT_BEGIN("Param");
        OBJECT_STR(k);
        OBJECT_SUB(v);
        OBJECT_END();
      }
      ss << "]";
      OBJECT_END();
      break;
    }
    case QAST_NODE_TEMPL_CALL: {
      OBJECT_BEGIN("TCall");
      OBJECT_SUB(n->as<TemplCall>()->get_func());
      ss << "[";
      for (const auto &[k, v] : n->as<TemplCall>()->get_template_args()) {
        OBJECT_BEGIN("Param");
        OBJECT_STR(k);
        OBJECT_SUB(v);
        OBJECT_END();
      }
      ss << "]";
      OBJECT_SUB(n->as<TemplCall>()->get_func());
      ss << "[";
      for (const auto &[k, v] : n->as<TemplCall>()->get_args()) {
        OBJECT_BEGIN("Param");
        OBJECT_STR(k);
        OBJECT_SUB(v);
        OBJECT_END();
      }
      ss << "]";
      OBJECT_END();
      break;
    }
    case QAST_NODE_LIST: {
      OBJECT_BEGIN("List");
      OBJECT_ARRAY(n->as<List>()->get_items());
      OBJECT_END();
      break;
    }
    case QAST_NODE_ASSOC: {
      OBJECT_BEGIN("Assoc");
      OBJECT_SUB(n->as<Assoc>()->get_key());
      OBJECT_SUB(n->as<Assoc>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_FIELD: {
      OBJECT_BEGIN("Field");
      OBJECT_STR(n->as<Field>()->get_field());
      OBJECT_SUB(n->as<Field>()->get_base());
      OBJECT_END();
      break;
    }
    case QAST_NODE_INDEX: {
      OBJECT_BEGIN("Idx");
      OBJECT_SUB(n->as<Index>()->get_base());
      OBJECT_SUB(n->as<Index>()->get_index());
      OBJECT_END();
      break;
    }
    case QAST_NODE_SLICE: {
      OBJECT_BEGIN("Slice");
      OBJECT_SUB(n->as<Slice>()->get_base());
      OBJECT_SUB(n->as<Slice>()->get_start());
      OBJECT_SUB(n->as<Slice>()->get_end());
      OBJECT_END();
      break;
    }
    case QAST_NODE_FSTRING: {
      OBJECT_BEGIN("FString");
      for (const auto &v : n->as<FString>()->get_items()) {
        if (std::holds_alternative<String>(v)) {
          OBJECT_STR(std::get<String>(v));
        } else {
          OBJECT_SUB(std::get<Expr *>(v));
        }
      }
      OBJECT_END();
      break;
    }
    case QAST_NODE_IDENT: {
      OBJECT_BEGIN("Name");
      OBJECT_STR(n->as<Ident>()->get_name());
      OBJECT_END();
      break;
    }
    case QAST_NODE_SEQ_POINT: {
      OBJECT_BEGIN("Seq");
      OBJECT_ARRAY(n->as<SeqPoint>()->get_items());
      OBJECT_END();
      break;
    }
    case QAST_NODE_POST_UNEXPR: {
      OBJECT_BEGIN("PExpr");
      OBJECT_ALI(n->as<PostUnaryExpr>()->get_op());
      OBJECT_SUB(n->as<PostUnaryExpr>()->get_lhs());
      OBJECT_END();
      break;
    }
    case QAST_NODE_STMT_EXPR: {
      OBJECT_BEGIN("SExpr");
      OBJECT_SUB(n->as<StmtExpr>()->get_stmt());
      OBJECT_END();
      break;
    }
    case QAST_NODE_TYPE_EXPR: {
      OBJECT_BEGIN("TExpr");
      OBJECT_SUB(n->as<TypeExpr>()->get_type());
      OBJECT_END();
      break;
    }
    case QAST_NODE_REF_TY: {
      OBJECT_BEGIN("Mut");
      OBJECT_SUB(n->as<RefTy>()->get_item());
      OBJECT_SUB(n->as<RefTy>()->get_width());
      OBJECT_SUB(n->as<RefTy>()->get_range().first);
      OBJECT_SUB(n->as<RefTy>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_U1_TY: {
      OBJECT_BEGIN("U1");
      OBJECT_SUB(n->as<U1>()->get_width());
      OBJECT_SUB(n->as<U1>()->get_range().first);
      OBJECT_SUB(n->as<U1>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_U8_TY: {
      OBJECT_BEGIN("U8");
      OBJECT_SUB(n->as<U8>()->get_width());
      OBJECT_SUB(n->as<U8>()->get_range().first);
      OBJECT_SUB(n->as<U8>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_U16_TY: {
      OBJECT_BEGIN("U16");
      OBJECT_SUB(n->as<U16>()->get_width());
      OBJECT_SUB(n->as<U16>()->get_range().first);
      OBJECT_SUB(n->as<U16>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_U32_TY: {
      OBJECT_BEGIN("U32");
      OBJECT_SUB(n->as<U32>()->get_width());
      OBJECT_SUB(n->as<U32>()->get_range().first);
      OBJECT_SUB(n->as<U32>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_U64_TY: {
      OBJECT_BEGIN("U64");
      OBJECT_SUB(n->as<U64>()->get_width());
      OBJECT_SUB(n->as<U64>()->get_range().first);
      OBJECT_SUB(n->as<U64>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_U128_TY: {
      OBJECT_BEGIN("U128");
      OBJECT_SUB(n->as<U128>()->get_width());
      OBJECT_SUB(n->as<U128>()->get_range().first);
      OBJECT_SUB(n->as<U128>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_I8_TY: {
      OBJECT_BEGIN("I8");
      OBJECT_SUB(n->as<I8>()->get_width());
      OBJECT_SUB(n->as<I8>()->get_range().first);
      OBJECT_SUB(n->as<I8>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_I16_TY: {
      OBJECT_BEGIN("I16");
      OBJECT_SUB(n->as<I16>()->get_width());
      OBJECT_SUB(n->as<I16>()->get_range().first);
      OBJECT_SUB(n->as<I16>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_I32_TY: {
      OBJECT_BEGIN("I32");
      OBJECT_SUB(n->as<I32>()->get_width());
      OBJECT_SUB(n->as<I32>()->get_range().first);
      OBJECT_SUB(n->as<I32>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_I64_TY: {
      OBJECT_BEGIN("I64");
      OBJECT_SUB(n->as<I64>()->get_width());
      OBJECT_SUB(n->as<I64>()->get_range().first);
      OBJECT_SUB(n->as<I64>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_I128_TY: {
      OBJECT_BEGIN("I128");
      OBJECT_SUB(n->as<I128>()->get_width());
      OBJECT_SUB(n->as<I128>()->get_range().first);
      OBJECT_SUB(n->as<I128>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_F16_TY: {
      OBJECT_BEGIN("F16");
      OBJECT_SUB(n->as<F16>()->get_width());
      OBJECT_SUB(n->as<F16>()->get_range().first);
      OBJECT_SUB(n->as<F16>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_F32_TY: {
      OBJECT_BEGIN("F32");
      OBJECT_SUB(n->as<F32>()->get_width());
      OBJECT_SUB(n->as<F32>()->get_range().first);
      OBJECT_SUB(n->as<F32>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_F64_TY: {
      OBJECT_BEGIN("F64");
      OBJECT_SUB(n->as<F64>()->get_width());
      OBJECT_SUB(n->as<F64>()->get_range().first);
      OBJECT_SUB(n->as<F64>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_F128_TY: {
      OBJECT_BEGIN("F128");
      OBJECT_SUB(n->as<F128>()->get_width());
      OBJECT_SUB(n->as<F128>()->get_range().first);
      OBJECT_SUB(n->as<F128>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_VOID_TY: {
      OBJECT_BEGIN("Void");
      OBJECT_SUB(n->as<VoidTy>()->get_width());
      OBJECT_SUB(n->as<VoidTy>()->get_range().first);
      OBJECT_SUB(n->as<VoidTy>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_PTR_TY: {
      OBJECT_BEGIN("Ptr");
      OBJECT_SUB(n->as<PtrTy>()->get_item());
      OBJECT_SUB(n->as<PtrTy>()->get_width());
      OBJECT_SUB(n->as<PtrTy>()->get_range().first);
      OBJECT_SUB(n->as<PtrTy>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_OPAQUE_TY: {
      OBJECT_BEGIN("Opaque");
      OBJECT_STR(n->as<OpaqueTy>()->get_name());
      OBJECT_SUB(n->as<OpaqueTy>()->get_width());
      OBJECT_SUB(n->as<OpaqueTy>()->get_range().first);
      OBJECT_SUB(n->as<OpaqueTy>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_STRUCT_TY: {
      OBJECT_BEGIN("Struct");
      for (const auto &[k, v] : n->as<StructTy>()->get_items()) {
        OBJECT_BEGIN("Field");
        OBJECT_STR(k);
        OBJECT_SUB(v);
        OBJECT_END();
      }
      OBJECT_SUB(n->as<StructTy>()->get_width());
      OBJECT_SUB(n->as<StructTy>()->get_range().first);
      OBJECT_SUB(n->as<StructTy>()->get_range().second);
      OBJECT_END();
      break;
    }

    case QAST_NODE_ARRAY_TY: {
      OBJECT_BEGIN("Array");
      OBJECT_SUB(n->as<ArrayTy>()->get_item());
      OBJECT_SUB(n->as<ArrayTy>()->get_size());
      OBJECT_SUB(n->as<ArrayTy>()->get_width());
      OBJECT_SUB(n->as<ArrayTy>()->get_range().first);
      OBJECT_SUB(n->as<ArrayTy>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_TUPLE_TY: {
      OBJECT_BEGIN("Tuple");
      OBJECT_ARRAY(n->as<TupleTy>()->get_items());
      OBJECT_SUB(n->as<TupleTy>()->get_width());
      OBJECT_SUB(n->as<TupleTy>()->get_range().first);
      OBJECT_SUB(n->as<TupleTy>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_FN_TY: {
      OBJECT_BEGIN("FType");
      OBJECT_SUB(n->as<FuncTy>()->get_return_ty());
      ss << "[";
      for (const auto &param : n->as<FuncTy>()->get_params()) {
        OBJECT_BEGIN("Param");
        OBJECT_STR(std::get<0>(param));
        OBJECT_SUB(std::get<1>(param));
        OBJECT_SUB(std::get<2>(param));
        OBJECT_END();
      }
      ss << "]";
      state.indent++;
      indent(ss, state);
      state.indent--;
      ss << "\"" << n->as<FuncTy>()->get_purity() << "\"";
      OBJECT_SUB(n->as<FuncTy>()->get_width());
      OBJECT_SUB(n->as<FuncTy>()->get_range().first);
      OBJECT_SUB(n->as<FuncTy>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_UNRES_TY: {
      OBJECT_BEGIN("Unres");
      OBJECT_STR(n->as<UnresolvedType>()->get_name());
      OBJECT_SUB(n->as<UnresolvedType>()->get_width());
      OBJECT_SUB(n->as<UnresolvedType>()->get_range().first);
      OBJECT_SUB(n->as<UnresolvedType>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_INFER_TY: {
      OBJECT_BEGIN("Infer");
      OBJECT_SUB(n->as<InferType>()->get_width());
      OBJECT_SUB(n->as<InferType>()->get_range().first);
      OBJECT_SUB(n->as<InferType>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_TEMPL_TY: {
      OBJECT_BEGIN("TType");
      OBJECT_SUB(n->as<TemplType>()->get_template());
      OBJECT_ARRAY(n->as<TemplType>()->get_args());
      OBJECT_SUB(n->as<TemplType>()->get_width());
      OBJECT_SUB(n->as<TemplType>()->get_range().first);
      OBJECT_SUB(n->as<TemplType>()->get_range().second);
      OBJECT_END();
      break;
    }
    case QAST_NODE_TYPEDEF: {
      OBJECT_BEGIN("Typedef");
      OBJECT_STR(n->as<TypedefDecl>()->get_name());
      OBJECT_SUB(n->as<TypedefDecl>()->get_type());
      OBJECT_END();
      break;
    }
    case QAST_NODE_FNDECL: {
      OBJECT_BEGIN("FnDecl");
      OBJECT_STR(n->as<FnDecl>()->get_name());
      { /* Template parameters */
        state.indent++;
        indent(ss, state);
        ss << "[";
        for (const auto &[name, type, def] :
             n->as<FnDecl>()->get_template_params().value_or(
                 TemplateParameters())) {
          OBJECT_BEGIN("TParam");
          OBJECT_STR(name);
          OBJECT_SUB(type);
          OBJECT_SUB(def);
          OBJECT_END();
        }
        ss << "]";
        state.indent--;
      }
      OBJECT_SUB(n->as<FnDecl>()->get_type());
      OBJECT_ARRAY(n->as<FnDecl>()->get_tags());
      OBJECT_NUM((int)n->as<FnDecl>()->get_visibility());
      OBJECT_END();
      break;
    }
    case QAST_NODE_STRUCT: {
      OBJECT_BEGIN("Struct");
      OBJECT_STR(n->as<StructDef>()->get_name());
      { /* Template parameters */
        state.indent++;
        indent(ss, state);
        ss << "[";
        for (const auto &[name, type, def] :
             n->as<StructDef>()->get_template_params().value_or(
                 TemplateParameters())) {
          OBJECT_BEGIN("TParam");
          OBJECT_STR(name);
          OBJECT_SUB(type);
          OBJECT_SUB(def);
          OBJECT_END();
        }
        ss << "]";
        state.indent--;
      }
      OBJECT_ARRAY(n->as<StructDef>()->get_fields());
      OBJECT_ARRAY(n->as<StructDef>()->get_methods());
      OBJECT_ARRAY(n->as<StructDef>()->get_static_methods());
      OBJECT_ARRAY(n->as<StructDef>()->get_tags());
      OBJECT_NUM((int)n->as<StructDef>()->get_visibility());
      OBJECT_END();
      break;
    }
    case QAST_NODE_ENUM: {
      OBJECT_BEGIN("Enum");
      OBJECT_STR(n->as<EnumDef>()->get_name());
      OBJECT_SUB(n->as<EnumDef>()->get_type());
      state.indent++;
      indent(ss, state);
      ss << "[";
      for (const auto &[k, v] : n->as<EnumDef>()->get_items()) {
        OBJECT_BEGIN("Field");
        OBJECT_STR(k);
        OBJECT_SUB(v);
        OBJECT_END();
      }
      ss << "]";
      state.indent--;
      OBJECT_ARRAY(n->as<EnumDef>()->get_tags());
      OBJECT_NUM((int)n->as<EnumDef>()->get_visibility());
      OBJECT_END();
      break;
    }
    case QAST_NODE_FN: {
      OBJECT_BEGIN("Fn");
      OBJECT_STR(n->as<FnDef>()->get_name());
      state.indent++;
      indent(ss, state);
      ss << "[";
      for (const auto &[v, mut] : n->as<FnDef>()->get_captures()) {
        OBJECT_BEGIN("Capture");
        OBJECT_STR(v);
        OBJECT_NUM((int)mut);
        OBJECT_END();
      }
      ss << "]";
      state.indent--;
      { /* Template parameters */
        state.indent++;
        indent(ss, state);
        ss << "[";
        for (const auto &[name, type, def] :
             n->as<FnDef>()->get_template_params().value_or(
                 TemplateParameters())) {
          OBJECT_BEGIN("TParam");
          OBJECT_STR(name);
          OBJECT_SUB(type);
          OBJECT_SUB(def);
          OBJECT_END();
        }
        ss << "]";
        state.indent--;
      }
      OBJECT_SUB(n->as<FnDef>()->get_type());
      OBJECT_SUB(n->as<FnDef>()->get_precond());
      OBJECT_SUB(n->as<FnDef>()->get_postcond());
      OBJECT_ARRAY(n->as<FnDef>()->get_tags());
      OBJECT_SUB(n->as<FnDef>()->get_body());
      OBJECT_END();
      break;
    }
    case QAST_NODE_SUBSYSTEM: {
      OBJECT_BEGIN("Subsystem");
      OBJECT_STR(n->as<SubsystemDecl>()->get_name());
      OBJECT_ARRAY(n->as<SubsystemDecl>()->get_tags());
      OBJECT_TAGS(n->as<SubsystemDecl>()->get_deps());
      OBJECT_SUB(n->as<SubsystemDecl>()->get_body());
      OBJECT_END();
      break;
    }
    case QAST_NODE_EXPORT: {
      OBJECT_BEGIN("Export");
      OBJECT_STR(n->as<ExportDecl>()->get_abi_name());
      OBJECT_SUB(n->as<ExportDecl>()->get_body());
      OBJECT_END();
      break;
    }
    case QAST_NODE_COMPOSITE_FIELD: {
      OBJECT_BEGIN("Field");
      OBJECT_STR(n->as<CompositeField>()->get_name());
      OBJECT_SUB(n->as<CompositeField>()->get_type());
      OBJECT_SUB(n->as<CompositeField>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_BLOCK: {
      OBJECT_BEGIN("Blk");
      OBJECT_ARRAY(n->as<Block>()->get_items());
      OBJECT_NUM((int)n->as<Block>()->get_safety());
      OBJECT_END();
      break;
    }
    case QAST_NODE_VOLSTMT: {
      OBJECT_BEGIN("VStmt");
      OBJECT_SUB(n->as<VolStmt>()->get_stmt());
      OBJECT_END();
      break;
    }
    case QAST_NODE_CONST: {
      OBJECT_BEGIN("Const");
      OBJECT_STR(n->as<ConstDecl>()->get_name());
      OBJECT_SUB(n->as<ConstDecl>()->get_type());
      OBJECT_SUB(n->as<ConstDecl>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_VAR: {
      OBJECT_BEGIN("Var");
      OBJECT_STR(n->as<VarDecl>()->get_name());
      OBJECT_SUB(n->as<VarDecl>()->get_type());
      OBJECT_SUB(n->as<VarDecl>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_LET: {
      OBJECT_BEGIN("Let");
      OBJECT_STR(n->as<LetDecl>()->get_name());
      OBJECT_SUB(n->as<LetDecl>()->get_type());
      OBJECT_SUB(n->as<LetDecl>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_INLINE_ASM: {
      OBJECT_BEGIN("Asm");
      OBJECT_STR(n->as<InlineAsm>()->get_code());
      OBJECT_ARRAY(n->as<InlineAsm>()->get_args());
      OBJECT_END();
      break;
    }
    case QAST_NODE_RETURN: {
      OBJECT_BEGIN("Ret");
      OBJECT_SUB(n->as<ReturnStmt>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_RETIF: {
      OBJECT_BEGIN("RetIf");
      OBJECT_SUB(n->as<ReturnIfStmt>()->get_cond());
      OBJECT_SUB(n->as<ReturnIfStmt>()->get_value());
      OBJECT_END();
      break;
    }
    case QAST_NODE_BREAK: {
      OBJECT_EMPTY("Break");
      break;
    }
    case QAST_NODE_CONTINUE: {
      OBJECT_EMPTY("Continue");
      break;
    }
    case QAST_NODE_IF: {
      OBJECT_BEGIN("If");
      OBJECT_SUB(n->as<IfStmt>()->get_cond());
      OBJECT_SUB(n->as<IfStmt>()->get_then());
      OBJECT_SUB(n->as<IfStmt>()->get_else());
      OBJECT_END();
      break;
    }
    case QAST_NODE_WHILE: {
      OBJECT_BEGIN("While");
      OBJECT_SUB(n->as<WhileStmt>()->get_cond());
      OBJECT_SUB(n->as<WhileStmt>()->get_body());
      OBJECT_END();
      break;
    }
    case QAST_NODE_FOR: {
      OBJECT_BEGIN("For");
      OBJECT_SUB(n->as<ForStmt>()->get_init());
      OBJECT_SUB(n->as<ForStmt>()->get_cond());
      OBJECT_SUB(n->as<ForStmt>()->get_step());
      OBJECT_SUB(n->as<ForStmt>()->get_body());
      OBJECT_END();
      break;
    }
    case QAST_NODE_FOREACH: {
      OBJECT_BEGIN("Foreach");
      OBJECT_STR(n->as<ForeachStmt>()->get_idx_ident());
      OBJECT_STR(n->as<ForeachStmt>()->get_val_ident());
      OBJECT_SUB(n->as<ForeachStmt>()->get_expr());
      OBJECT_SUB(n->as<ForeachStmt>()->get_body());
      OBJECT_END();
      break;
    }
    case QAST_NODE_CASE: {
      OBJECT_BEGIN("Case");
      OBJECT_SUB(n->as<CaseStmt>()->get_cond());
      OBJECT_SUB(n->as<CaseStmt>()->get_body());
      OBJECT_END();
      break;
    }
    case QAST_NODE_SWITCH: {
      OBJECT_BEGIN("Switch");
      OBJECT_SUB(n->as<SwitchStmt>()->get_cond());
      OBJECT_ARRAY(n->as<SwitchStmt>()->get_cases());
      OBJECT_SUB(n->as<SwitchStmt>()->get_default());
      OBJECT_END();
      break;
    }
    case QAST_NODE_EXPR_STMT: {
      OBJECT_BEGIN("EStmt");
      OBJECT_SUB(n->as<ExprStmt>()->get_expr());
      OBJECT_END();
      break;
    }
    case QAST_NODE_STMT:
    case QAST_NODE_TYPE:
    case QAST_NODE_DECL:
    case QAST_NODE_EXPR: {
      qcore_panicf("Invalid node type for serialization: %s", n->this_nameof());
    }
    case QAST_NODE_CEXPR:
      OBJECT_BEGIN("CExpr");
      OBJECT_SUB(n->as<ConstExpr>()->get_value());
      OBJECT_END();
      break;
  }
}

LIB_EXPORT char *qparse_repr(const qparse_node_t *node, bool minify,
                             size_t indent, size_t *outlen) {
  size_t outlen_v = 0;

  /* Eliminate internal edge cases */
  if (!outlen) {
    outlen = &outlen_v;
  }

  /* Create a string stream based on the arena */
  ConvStream ss;
  ConvState state = {-1, indent, minify};
  const Node *n = static_cast<const Node *>(node);

  /* Serialize the AST recursively */
  serialize_recurse(const_cast<Node *>(n), ss, state);

  std::string str = ss.str();
  *outlen = str.size();

  return strdup(str.c_str());
}

CPP_EXPORT std::ostream &std::operator<<(std::ostream &os,
                                         const qlex_op_t &op) {
  os << qlex_opstr((qlex_op_t)op);
  return os;
}

std::ostream &std::operator<<(std::ostream &os, const FuncPurity &purity) {
  switch (purity) {
    case FuncPurity::IMPURE_THREAD_UNSAFE:
      os << "impure";
      break;
    case FuncPurity::IMPURE_THREAD_SAFE:
      os << "impure tsafe";
      break;
    case FuncPurity::PURE:
      os << "pure";
      break;
    case FuncPurity::QUASIPURE:
      os << "quasipure";
      break;
    case FuncPurity::RETROPURE:
      os << "retropure";
      break;
  }

  return os;
}
