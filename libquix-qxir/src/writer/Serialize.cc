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

#include <core/LibMacro.h>
#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-qxir/Lib.h>
#include <quix-qxir/TypeDecl.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <unordered_set>

using namespace qxir;

struct ConvState {
  int32_t indent;
  size_t indent_width;
  bool minify;
  std::unordered_set<uint64_t> types;

  ConvState(size_t index_width, bool minify)
      : indent(0), indent_width(index_width), minify(minify) {}
};

template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<Op, std::string_view> opstr_map = make_bimap<Op, std::string_view>({
    {Op::Plus, "+"},
    {Op::Minus, "-"},
    {Op::Times, "*"},
    {Op::Slash, "/"},
    {Op::Percent, "%"},
    {Op::BitAnd, "&"},
    {Op::BitOr, "|"},
    {Op::BitXor, "^"},
    {Op::BitNot, "~"},
    {Op::LogicAnd, "&&"},
    {Op::LogicOr, "||"},
    {Op::LogicNot, "!"},
    {Op::LShift, "<<"},
    {Op::RShift, ">>"},
    {Op::ROTR, ">>>"},
    {Op::ROTL, "<<<"},
    {Op::Inc, "++"},
    {Op::Dec, "--"},
    {Op::Set, "="},
    {Op::LT, "<"},
    {Op::GT, ">"},
    {Op::LE, "<="},
    {Op::GE, ">="},
    {Op::Eq, "=="},
    {Op::NE, "!="},
    {Op::Alignof, "alignof"},
    {Op::BitcastAs, "bitcast_as"},
    {Op::CastAs, "cast_as"},
    {Op::Bitsizeof, "bitsizeof"},
});

static inline FILE &operator<<(FILE &ss, const char *s) {
  fprintf(&ss, "%s", s);
  return ss;
}

static inline FILE &operator<<(FILE &ss, Op s) {
  fprintf(&ss, "%s", opstr_map.left.at(s).data());
  return ss;
}

static inline FILE &operator<<(FILE &ss, const std::string_view &s) {
  fprintf(&ss, "%s", s.data());
  return ss;
}

static inline FILE &operator<<(FILE &ss, const size_t s) {
  fprintf(&ss, "%zu", s);
  return ss;
}

static void escape_string(FILE &ss, std::string_view input) {
  fputc('"', &ss);

  for (char ch : input) {
    switch (ch) {
      case '"':
        fprintf(&ss, "\\\"");
        break;
      case '\\':
        fprintf(&ss, "\\\\");
        break;
      case '\b':
        fprintf(&ss, "\\b");
        break;
      case '\f':
        fprintf(&ss, "\\f");
        break;
      case '\n':
        fprintf(&ss, "\\n");
        break;
      case '\r':
        fprintf(&ss, "\\r");
        break;
      case '\t':
        fprintf(&ss, "\\t");
        break;
      case '\0':
        fprintf(&ss, "\\0");
        break;
      default:
        if (ch >= 32 && ch < 127) {
          fputc(ch, &ss);
        } else {
          char hex[5];
          snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
          fprintf(&ss, "%s", hex);
        }
        break;
    }
  }

  fputc('"', &ss);
}

static void indent(FILE &ss, ConvState &state) {
  if (state.minify) {
    return;
  }

  ss << "\n";

  if (state.indent > 0) {
    ss << std::string(state.indent * state.indent_width, ' ');
  }
}

static bool serialize_recurse(Expr *n, FILE &ss, FILE &typedefs, ConvState &state
#if !defined(NDEBUG)
                              ,
                              std::unordered_set<Expr *> &visited, bool is_cylic
#endif
) {
  if (!n) { /* Nicely handle null nodes */
    ss << "{?}";
    return true;
  }

#if !defined(NDEBUG)
  if (is_cylic) {
    if (visited.contains(n)) {
      ss << "{...}";
      return true;
    }
    visited.insert(n);
  }
#define recurse(x) serialize_recurse(x, ss, typedefs, state, visited, is_cylic)
#define recurse_ex(x, stream) serialize_recurse(x, stream, typedefs, state, visited, is_cylic)
#else
#define recurse(x) serialize_recurse(x, ss, typedefs, state)
#define recurse_ex(x, stream) serialize_recurse(x, stream, typedefs, state)
#endif

  switch (n->getKind()) {
    case QIR_NODE_BINEXPR: {
      ss << "(";
      recurse(n->as<BinExpr>()->getLHS());
      ss << " ";
      ss << n->as<BinExpr>()->getOp();
      ss << " ";
      recurse(n->as<BinExpr>()->getRHS());
      ss << ")";
      break;
    }
    case QIR_NODE_UNEXPR: {
      ss << "(";
      ss << n->as<UnExpr>()->getOp();
      ss << "(";
      recurse(n->as<UnExpr>()->getExpr());
      ss << "))";
      break;
    }
    case QIR_NODE_POST_UNEXPR: {
      ss << "(";
      recurse(n->as<PostUnExpr>()->getExpr());
      ss << n->as<PostUnExpr>()->getOp();
      ss << ")";
      break;
    }
    case QIR_NODE_INT: {
      recurse(n->as<Int>()->getType());
      ss << " " << n->as<Int>()->getValue();
      break;
    }
    case QIR_NODE_FLOAT: {
      recurse(n->as<Float>()->getType());
      ss << " " << n->as<Float>()->getValue();
      break;
    }
    case QIR_NODE_LIST: {
      // Check if it matches the string literal pattern
      List *L = n->as<List>();

      recurse(L->getType());
      ss << " ";

      bool is_cstring = false;
      std::string c_string;
      for (size_t i = 0; i < L->getItems().size(); i++) {
        if (L->getItems()[i]->getKind() != QIR_NODE_BINEXPR) {
          break;
        }

        BinExpr *BE = L->getItems()[i]->as<BinExpr>();

        if (BE->getLHS()->getKind() != QIR_NODE_INT) {
          break;
        }

        if (BE->getRHS()->getKind() != QIR_NODE_I8_TY) {
          break;
        }

        if (BE->getOp() != Op::CastAs) {
          break;
        }

        c_string.push_back((char)BE->getLHS()->as<Int>()->getNativeRepresentation());

        if (i + 1 == L->getItems().size()) {  // Last item
          if (BE->getLHS()->as<Int>()->getNativeRepresentation() != 0) {
            break;
          }

          is_cstring = true;

          escape_string(ss, c_string);
          break;
        }
      }

      if (!is_cstring) {
        ss << "{";
        for (auto it = L->getItems().begin(); it != L->getItems().end(); ++it) {
          recurse(*it);
          if (std::next(it) != L->getItems().end()) {
            ss << ",";
          }
        }
        ss << "}";
      }
      break;
    }
    case QIR_NODE_CALL: {
      auto tkind = n->as<Call>()->getTarget()->getKind();
      if (tkind == QIR_NODE_LOCAL) {
        ss << n->as<Call>()->getTarget()->as<Local>()->getName();
      } else if (tkind == QIR_NODE_FN) {
        ss << n->as<Call>()->getTarget()->as<Fn>()->getName();
      } else {
        recurse(n->as<Call>()->getTarget());
      }
      ss << "(";
      for (auto it = n->as<Call>()->getArgs().begin(); it != n->as<Call>()->getArgs().end(); ++it) {
        recurse(*it);
        if (std::next(it) != n->as<Call>()->getArgs().end()) {
          ss << ",";
        }
      }
      ss << ")";
      break;
    }
    case QIR_NODE_SEQ: {
      ss << "seq {";
      state.indent++;
      indent(ss, state);
      for (auto it = n->as<Seq>()->getItems().begin(); it != n->as<Seq>()->getItems().end(); ++it) {
        if ((*it)->getKind() == QIR_NODE_IGN) {
          continue;
        }

        recurse(*it);
        ss << ",";

        if (std::next(it) != n->as<Seq>()->getItems().end()) {
          indent(ss, state);
        }
      }
      state.indent--;
      indent(ss, state);
      ss << "}";
      break;
    }
    case QIR_NODE_INDEX: {
      recurse(n->as<Index>()->getExpr());
      ss << "[";
      recurse(n->as<Index>()->getIndex());
      ss << "]";
      break;
    }
    case QIR_NODE_IDENT: {
      recurse(n->as<Ident>()->getType());
      ss << " " << n->as<Ident>()->getName();
      break;
    }
    case QIR_NODE_EXTERN: {
      ss << "extern ";
      escape_string(ss, n->as<Extern>()->getAbiName());
      ss << " ";
      recurse(n->as<Extern>()->getValue());
      break;
    }
    case QIR_NODE_LOCAL: {
      ss << "local ";
      ss << n->as<Local>()->getName();
      if (n->as<Local>()->getValue()->isType()) {
      } else {
        ss << " = ";
        recurse(n->as<Local>()->getValue());
      }
      break;
    }
    case QIR_NODE_RET: {
      ss << "ret ";
      recurse(n->as<Ret>()->getExpr());
      break;
    }
    case QIR_NODE_BRK: {
      ss << "brk";
      break;
    }
    case QIR_NODE_CONT: {
      ss << "cont";
      break;
    }

    case QIR_NODE_IF: {
      ss << "if (";
      recurse(n->as<If>()->getCond());
      ss << ") then ";
      recurse(n->as<If>()->getThen());
      ss << " else ";
      recurse(n->as<If>()->getElse());
      break;
    }
    case QIR_NODE_WHILE: {
      ss << "while (";
      recurse(n->as<While>()->getCond());
      ss << ") ";
      recurse(n->as<While>()->getBody());
      break;
    }
    case QIR_NODE_FOR: {
      ss << "for (";
      recurse(n->as<For>()->getInit());
      ss << "; ";
      recurse(n->as<For>()->getCond());
      ss << "; ";
      recurse(n->as<For>()->getStep());
      ss << ") ";
      recurse(n->as<For>()->getBody());
      break;
    }
    case QIR_NODE_FORM: {
      ss << "form (";
      recurse(n->as<Form>()->getMaxJobs());
      ss << ") (" << n->as<Form>()->getIdxIdent() << "; ";
      ss << n->as<Form>()->getValIdent() << "; ";
      recurse(n->as<Form>()->getExpr());
      ss << ") ";
      recurse(n->as<Form>()->getBody());
      break;
    }
    case QIR_NODE_CASE: {
      ss << "case ";
      recurse(n->as<Case>()->getCond());
      ss << ": ";
      recurse(n->as<Case>()->getBody());
      break;
    }
    case QIR_NODE_SWITCH: {
      ss << "switch (";
      recurse(n->as<Switch>()->getCond());
      ss << ") {";
      state.indent++;
      indent(ss, state);
      for (auto it = n->as<Switch>()->getCases().begin(); it != n->as<Switch>()->getCases().end();
           ++it) {
        recurse(*it);
        ss << ",";
        indent(ss, state);
      }
      ss << "default: ";
      recurse(n->as<Switch>()->getDefault());
      state.indent--;
      indent(ss, state);
      ss << "}";
      break;
    }
    case QIR_NODE_FN: {
      ss << "fn ";
      ss << n->as<Fn>()->getName();
      ss << "(";
      for (auto it = n->as<Fn>()->getParams().begin(); it != n->as<Fn>()->getParams().end(); ++it) {
        ss << it->second << ": ";
        recurse(it->first);
        if (std::next(it) != n->as<Fn>()->getParams().end() || n->as<Fn>()->isVariadic()) {
          ss << ",";
        }
      }
      if (n->as<Fn>()->isVariadic()) {
        ss << "...";
      }
      ss << ") -> ";
      recurse(n->as<Fn>()->getReturn());
      ss << " ";
      recurse(n->as<Fn>()->getBody());
      break;
    }
    case QIR_NODE_ASM: {
      qcore_implement("ASM serialization");
    }
    case QIR_NODE_IGN: {
      break;
    }
    case QIR_NODE_U1_TY: {
      ss << "u1";
      break;
    }
    case QIR_NODE_U8_TY: {
      ss << "u8";
      break;
    }
    case QIR_NODE_U16_TY: {
      ss << "u16";
      break;
    }
    case QIR_NODE_U32_TY: {
      ss << "u32";
      break;
    }
    case QIR_NODE_U64_TY: {
      ss << "u64";
      break;
    }
    case QIR_NODE_U128_TY: {
      ss << "u128";
      break;
    }
    case QIR_NODE_I8_TY: {
      ss << "i8";
      break;
    }
    case QIR_NODE_I16_TY: {
      ss << "i16";
      break;
    }
    case QIR_NODE_I32_TY: {
      ss << "i32";
      break;
    }
    case QIR_NODE_I64_TY: {
      ss << "i64";
      break;
    }
    case QIR_NODE_I128_TY: {
      ss << "i128";
      break;
    }
    case QIR_NODE_F16_TY: {
      ss << "f16";
      break;
    }
    case QIR_NODE_F32_TY: {
      ss << "f32";
      break;
    }
    case QIR_NODE_F64_TY: {
      ss << "f64";
      break;
    }
    case QIR_NODE_F128_TY: {
      ss << "f128";
      break;
    }
    case QIR_NODE_VOID_TY: {
      ss << "void";
      break;
    }
    case QIR_NODE_PTR_TY: {
      recurse(n->as<PtrTy>()->getPointee());
      ss << "*";
      break;
    }
    case QIR_NODE_OPAQUE_TY: {
      ss << "opaque ";
      ss << n->as<OpaqueTy>()->getName();
      break;
    }
    case QIR_NODE_STRUCT_TY: {
      uint64_t type_id = n->as<StructTy>()->getUniqId();
      if (!state.types.contains(type_id)) {
        typedefs << "%" << type_id << " = struct {";
        state.indent++;
        indent(typedefs, state);
        for (auto it = n->as<StructTy>()->getFields().begin();
             it != n->as<StructTy>()->getFields().end(); ++it) {
          if ((*it)->getKind() == QIR_NODE_STRUCT_TY || (*it)->getKind() == QIR_NODE_UNION_TY) {
            typedefs << "%" << (*it)->as<StructTy>()->getUniqId();
          } else {
            recurse_ex(*it, typedefs);
          }
          typedefs << ",";

          if (std::next(it) != n->as<StructTy>()->getFields().end()) {
            indent(typedefs, state);
          }
        }
        state.indent--;
        indent(typedefs, state);
        typedefs << "}\n";
        state.types.insert(type_id);
      }

      ss << "%" << type_id;
      break;
    }
    case QIR_NODE_UNION_TY: {
      uint64_t type_id = n->as<UnionTy>()->getUniqId();
      if (!state.types.contains(type_id)) {
        typedefs << "%" << type_id << " = union {";
        state.indent++;
        indent(typedefs, state);
        for (auto it = n->as<UnionTy>()->getFields().begin();
             it != n->as<UnionTy>()->getFields().end(); ++it) {
          if ((*it)->getKind() == QIR_NODE_STRUCT_TY || (*it)->getKind() == QIR_NODE_UNION_TY) {
            typedefs << "%" << (*it)->as<StructTy>()->getUniqId();
          } else {
            recurse_ex(*it, typedefs);
          }
          typedefs << ",";

          if (std::next(it) != n->as<UnionTy>()->getFields().end()) {
            indent(typedefs, state);
          }
        }
        state.indent--;
        indent(typedefs, state);
        typedefs << "}\n";
        state.types.insert(type_id);
      }

      ss << "%" << type_id;
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      ss << "[";
      recurse(n->as<ArrayTy>()->getElement());
      ss << "; " << n->as<ArrayTy>()->getCount();
      ss << "]";
      break;
    }
    case QIR_NODE_FN_TY: {
      ss << "fn (";
      bool variadic = n->as<FnTy>()->getAttrs().contains(FnAttr::Variadic);
      for (auto it = n->as<FnTy>()->getParams().begin(); it != n->as<FnTy>()->getParams().end();
           ++it) {
        recurse(*it);
        if (std::next(it) != n->as<FnTy>()->getParams().end() || variadic) {
          ss << ",";
        }
      }
      if (variadic) {
        ss << "...";
      }
      ss << "): ";
      recurse(n->as<FnTy>()->getReturn());
      break;
    }
    case QIR_NODE_TMP: {
      ss << "`" << static_cast<uint64_t>(n->as<Tmp>()->getTmpType());
      ss << ";" << n->as<Tmp>()->getData().index() << "`";
      break;
    }
  }

  return true;
}

static bool to_codeform(Expr *node, bool minify, size_t indent_size, FILE &ss) {
  ConvState state(indent_size, minify);
  qmodule_t *mod = node->getModule();

  if (!minify) {
    { /* Print the module name */
      ss << "; Module: " << mod->getName() << "\n";
    }

    { /* Print the mutation passes applied */
      ss << "; Passes: [";
      size_t i = 0;
      for (auto it = mod->getPassesApplied().begin(); it != mod->getPassesApplied().end(); ++it) {
        ss << *it;

        if (std::next(it) != mod->getPassesApplied().end()) {
          ss << ",";

          if (!minify) {
            ss << " ";
            if (i % 6 == 0 && i != 0) {
              ss << "\n;          ";
            }
          }
        }

        i++;
      }
      ss << "]\n";
    }

    { /* Print the analysis passes applied */
      ss << "; Checks: [";
      size_t i = 0;
      for (auto it = mod->getChecksApplied().begin(); it != mod->getChecksApplied().end(); ++it) {
        ss << *it;

        if (std::next(it) != mod->getChecksApplied().end()) {
          ss << ",";

          if (!minify) {
            ss << " ";
            if (i % 6 == 0 && i != 0) {
              ss << "\n;          ";
            }
          }
        }

        i++;
      }
      ss << "]\n";
    }

    { /* Print other metadata */
      auto now = std::chrono::system_clock::now();
      auto in_time_t = std::chrono::system_clock::to_time_t(now);
      std::stringstream tmp_ss;
      tmp_ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
      std::string datestamp = tmp_ss.str();

      ss << "; Timestamp: " << datestamp << "\n";
      ss << "; Compiler: " << qxir_lib_version() << "\n";
      ss << "; Compiler invented by Wesley Jones\n\n";
    }
  }

#if !defined(NDEBUG)
  std::unordered_set<Expr *> v;
  bool is_cylic = !node->is_acyclic();
#endif

  char *body_content = NULL, *typedef_content = NULL;
  size_t body_content_size = 0, typedef_content_size = 0;
  FILE *body = open_memstream(&body_content, &body_content_size);
  if (!body) {
    return false;
  }
  FILE *typedefs = open_memstream(&typedef_content, &typedef_content_size);
  if (!typedefs) {
    return false;
  }

  try {
    /* Serialize the AST recursively */
    bool result = serialize_recurse(node, *body, *typedefs, state
#if !defined(NDEBUG)
                                    ,
                                    v, is_cylic
#endif
    );

    if (!result) {
      fclose(typedefs);
      fclose(body);
      free(body_content);
      free(typedef_content);
      return false;
    }

    fclose(typedefs);
    fclose(body);

    fwrite(typedef_content, 1, typedef_content_size, &ss);
    fwrite(body_content, 1, body_content_size, &ss);

    free(body_content);
    free(typedef_content);

    return true;
  } catch (...) {
    fclose(typedefs);
    fclose(body);
    free(body_content);
    free(typedef_content);
    return false;
  }
}

LIB_EXPORT bool qxir_write(const qxir_node_t *_node, qxir_serial_t mode, FILE *out, size_t *outlen,
                           uint32_t argcnt, ...) {
  (void)argcnt;

  bool status;
  Expr *node;
  long start, end;

  node = static_cast<Expr *>(const_cast<qxir_node_t *>(_node));

  if (outlen) {
    if ((start = ftell(out)) == -1) {
      return false;
    }
  }

  try {
    switch (mode) {
      case QXIR_SERIAL_CODE: {
        status = to_codeform(node, false, 2, *out);
        break;
      }
    }
  } catch (...) {
    return false;
  }

  if (outlen) {
    if ((end = ftell(out)) == -1) {
      return false;
    }

    *outlen = end - start;
  }

  fflush(out);

  return status;
}
