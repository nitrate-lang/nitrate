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

#define __QUIX_IMPL__

#include <libdeflate.h>
#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-qxir/Lib.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <unordered_set>

#include "core/LibMacro.h"
#include "quix-qxir/TypeDecl.h"

using namespace qxir;

struct ConvState {
  int32_t indent;
  size_t indent_width;
  bool minify;
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
    {Op::Typeof, "typeof"},
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

static bool serialize_recurse(Expr *n, FILE &ss, ConvState &state
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
#define recurse(x) serialize_recurse(x, ss, state, visited, is_cylic)
#else
#define recurse(x) serialize_recurse(x, ss, state)
#endif

  if (n->isConstExpr() && !n->isLiteral() && !n->isType()) {
    ss << "const ";
  }

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
    case QIR_NODE_STRING: {
      ss << "%" << n->as<String>()->getUniqId();
      break;
    }
    case QIR_NODE_LIST: {
      ss << "{";
      for (auto it = n->as<List>()->getItems().begin(); it != n->as<List>()->getItems().end();
           ++it) {
        recurse(*it);
        if (std::next(it) != n->as<List>()->getItems().end()) {
          ss << ",";
        }
      }
      ss << "}";
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
      ss << n->as<Ident>()->getName();
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
      recurse(n->as<Local>()->getType());
      ss << " ";
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
      if (n->as<Switch>()->getDefault()->getKind() != QIR_NODE_VOID_TY) {
        ss << "default: ";
        recurse(n->as<Switch>()->getDefault());
      }
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
      ss << "%" << n->as<StructTy>()->getUniqId();
      break;
    }
    case QIR_NODE_UNION_TY: {
      ss << "%" << n->as<UnionTy>()->getUniqId();
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
    default: {
      qcore_panicf("Unknown node type: %d", n->getKind());
    }
  }

  return true;
}

static bool to_codeform(Expr *node, bool minify, size_t indent_size, FILE &ss) {
  ConvState state = {0, indent_size, minify};
  qmodule_t *mod = node->getModule();

  if (!minify) {
    { /* Print the module name */
      ss << "; Module: " << mod->getName() << "\n";
    }

    { /* Print the passes applied */
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

  {
    std::unordered_set<uint64_t> node_ids;

    auto cb = [&](Expr *par, Expr **_cur) -> IterOp {
      Expr *n = *_cur;

      switch (n->getKind()) {
        case QIR_NODE_STRING: {
          uint64_t type_id = n->getUniqId();
          if (node_ids.contains(type_id)) {
            break;
          }

          node_ids.insert(type_id);

          ss << "%" << type_id << " = ";
          escape_string(ss, n->as<String>()->getValue());
          ss << "\n";
          break;
        }

        case QIR_NODE_STRUCT_TY: {
          uint64_t type_id = n->getType()->getUniqId();
          if (node_ids.contains(type_id)) {
            break;
          }
          node_ids.insert(type_id);
          ss << "%" << type_id << " = struct {";

          state.indent++;
          indent(ss, state);
          for (auto it = n->as<StructTy>()->getFields().begin();
               it != n->as<StructTy>()->getFields().end(); ++it) {
            serialize_recurse(*it, ss, state
#if !defined(NDEBUG)
                              ,
                              v, is_cylic
#endif
            );
            ss << ",";

            if (std::next(it) != n->as<StructTy>()->getFields().end()) {
              indent(ss, state);
            }
          }
          state.indent--;
          indent(ss, state);
          ss << "}\n";
          break;
        }

        case QIR_NODE_UNION_TY: {
          uint64_t type_id = n->getType()->getUniqId();
          if (node_ids.contains(type_id)) {
            break;
          }
          node_ids.insert(type_id);
          ss << "%" << type_id << " = union {";
          state.indent++;
          indent(ss, state);
          for (auto it = n->as<UnionTy>()->getFields().begin();
               it != n->as<UnionTy>()->getFields().end(); ++it) {
            serialize_recurse(*it, ss, state
#if !defined(NDEBUG)
                              ,
                              v, is_cylic
#endif
            );
            ss << ",";

            if (std::next(it) != n->as<UnionTy>()->getFields().end()) {
              indent(ss, state);
            }
          }
          state.indent--;
          indent(ss, state);
          ss << "}\n";
          break;
        }

        default: {
          break;
        }
      }

      return IterOp::Proceed;
    };

    iterate<dfs_pre>(node, cb);

    if (node_ids.size() > 0) {
      ss << "\n";
    }
  }

  /* Serialize the AST recursively */
  return serialize_recurse(node, ss, state
#if !defined(NDEBUG)
                           ,
                           v, is_cylic
#endif
  );
}

static bool raw_deflate(const uint8_t *in, size_t in_size, FILE &out) {
  struct libdeflate_compressor *ctx{};

  /* Allocate a compressor context; level 8 is a fairly good tradeoff */
  ctx = libdeflate_alloc_compressor(8);
  if (!ctx) {
    return false;
  }

  /* Compute the largest possible compressed buffer size */
  size_t out_size = libdeflate_deflate_compress_bound(ctx, in_size);

  uint8_t *buf = new uint8_t[out_size];

  out_size = libdeflate_deflate_compress(ctx, in, in_size, buf, out_size);

  libdeflate_free_compressor(ctx);

  /* Check for compression failure */
  if (out_size == 0) {
    delete[] buf;
    return false;
  }

  fwrite(buf, 1, out_size, &out);

  delete[] buf;

  return true;
}

static bool to_binform(Expr *node, bool compress, FILE &out) {
  char *membuf;
  size_t memlen;

  FILE *mem = open_memstream(&membuf, &memlen);

  /* Generate the AST representation as ASCII */
  if (!to_codeform(node, true, 0, *mem)) {
    fclose(mem);
    return false;
  }

  if (compress) {
    if (!raw_deflate((const uint8_t *)membuf, memlen, out)) {
      fclose(mem);
      return false;
    }
  }

  fclose(mem);

  return true;
}

LIB_EXPORT bool qxir_write(const qxir_node_t *_node, qxir_serial_t mode, FILE *out, size_t *outlen,
                           uint32_t argcnt, ...) {
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
      case QXIR_SERIAL_CODE_MIN: {
        status = to_codeform(node, true, 0, *out);
        break;
      }
      case QXIR_SERIAL_B10: {
        status = to_binform(node, true, *out);
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
