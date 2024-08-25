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

#define __QXIR_IMPL__

#include <libdeflate.h>
#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-qxir/Node.h>

#include <cstring>
#include <sstream>

#include "core/LibMacro.h"

using namespace qxir;

struct ConvState {
  int32_t indent;
  size_t indent_width;
  bool minify;
};

typedef std::basic_stringstream<char, std::char_traits<char>, Arena<char>> ConvStream;

static std::string escape_string(std::string_view input) {
  std::string output = "\"";
  output.reserve(input.length() + 2);

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

  ss << "\n";

  if (state.indent > 0) {
    ss << std::string(state.indent * state.indent_width, ' ');
  }
}

std::ostream &qxir::operator<<(std::ostream &os, qxir::Op op) {
  static const std::unordered_map<Op, std::string> op_map = {
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
      {Op::Offsetof, "offsetof"},
      {Op::BitcastAs, "bitcast_as"},
      {Op::CastAs, "cast_as"},
      {Op::Bitsizeof, "bitsizeof"},
  };

  return os << op_map.at(op);
}

static void serialize_recurse(Expr *n, ConvStream &ss, ConvState &state) {
  if (!n) {
    // Nicely handle null nodes
    ss << "{?}";
    return;
  }

  if (n->isConst()) {
    ss << "const ";
  }
  if (n->isVolatile()) {
    ss << "volatile ";
  }

  switch (n->getKind()) {
    case QIR_NODE_BINEXPR: {
      ss << "(";
      serialize_recurse(n->as<BinExpr>()->getLHS(), ss, state);
      ss << " ";
      ss << n->as<BinExpr>()->getOp();
      ss << " ";
      serialize_recurse(n->as<BinExpr>()->getRHS(), ss, state);
      ss << ")";
      break;
    }
    case QIR_NODE_UNEXPR: {
      ss << "(";
      ss << n->as<UnExpr>()->getOp();
      ss << " ";
      serialize_recurse(n->as<UnExpr>()->getExpr(), ss, state);
      ss << ")";
      break;
    }
    case QIR_NODE_POST_UNEXPR: {
      ss << "(";
      serialize_recurse(n->as<PostUnExpr>()->getExpr(), ss, state);
      ss << n->as<PostUnExpr>()->getOp();
      ss << ")";
      break;
    }
    case QIR_NODE_INT: {
      ss << n->as<Int>()->getValue();
      break;
    }
    case QIR_NODE_FLOAT: {
      ss << n->as<Float>()->getValue();
      break;
    }
    case QIR_NODE_STRING: {
      ss << escape_string(n->as<String>()->getValue());
      break;
    }
    case QIR_NODE_LIST: {
      ss << "{";
      for (auto it = n->as<List>()->getItems().begin(); it != n->as<List>()->getItems().end();
           ++it) {
        serialize_recurse(*it, ss, state);
        if (std::next(it) != n->as<List>()->getItems().end()) {
          ss << ",";
        }
      }
      ss << "}";
      break;
    }
    case QIR_NODE_ALLOC: {
      ss << "alloc ";
      serialize_recurse(n->as<Alloc>()->getType(), ss, state);
      break;
    }
    case QIR_NODE_CALL: {
      serialize_recurse(n->as<Call>()->getFn(), ss, state);
      ss << "(";
      for (auto it = n->as<Call>()->getArgs().begin(); it != n->as<Call>()->getArgs().end(); ++it) {
        serialize_recurse(*it, ss, state);
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
        serialize_recurse(*it, ss, state);
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
    case QIR_NODE_ASYNC: {
      ss << "async {";
      state.indent++;
      indent(ss, state);
      for (auto it = n->as<Seq>()->getItems().begin(); it != n->as<Seq>()->getItems().end(); ++it) {
        serialize_recurse(*it, ss, state);
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
      serialize_recurse(n->as<Index>()->getExpr(), ss, state);
      ss << "[";
      serialize_recurse(n->as<Index>()->getIndex(), ss, state);
      ss << "]";
      break;
    }
    case QIR_NODE_IDENT: {
      ss << n->as<Ident>()->getName();
      break;
    }
    case QIR_NODE_EXPORT: {
      ss << "export[" << n->as<Export>()->getAbiName() << "] ";
      ss << n->as<Export>()->getName();
      ss << " = ";
      serialize_recurse(n->as<Export>()->getValue(), ss, state);
      break;
    }
    case QIR_NODE_LOCAL: {
      ss << "local ";
      ss << n->as<Local>()->getName();
      ss << " = ";
      serialize_recurse(n->as<Local>()->getValue(), ss, state);
      break;
    }
    case QIR_NODE_RET: {
      ss << "ret ";
      serialize_recurse(n->as<Ret>()->getExpr(), ss, state);
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
      serialize_recurse(n->as<If>()->getCond(), ss, state);
      ss << ") then ";
      serialize_recurse(n->as<If>()->getThen(), ss, state);
      ss << " else ";
      serialize_recurse(n->as<If>()->getElse(), ss, state);
      break;
    }
    case QIR_NODE_WHILE: {
      ss << "while (";
      serialize_recurse(n->as<While>()->getCond(), ss, state);
      ss << ") ";
      serialize_recurse(n->as<While>()->getBody(), ss, state);
      break;
    }
    case QIR_NODE_FOR: {
      ss << "for (";
      serialize_recurse(n->as<For>()->getInit(), ss, state);
      ss << "; ";
      serialize_recurse(n->as<For>()->getCond(), ss, state);
      ss << "; ";
      serialize_recurse(n->as<For>()->getStep(), ss, state);
      ss << ") ";
      serialize_recurse(n->as<For>()->getBody(), ss, state);
      break;
    }
    case QIR_NODE_FORM: {
      ss << "form (";
      serialize_recurse(n->as<Form>()->getMaxJobs(), ss, state);
      ss << ") (" << n->as<Form>()->getIdxIdent() << "; ";
      ss << n->as<Form>()->getValIdent() << "; ";
      serialize_recurse(n->as<Form>()->getExpr(), ss, state);
      ss << ") ";
      serialize_recurse(n->as<Form>()->getBody(), ss, state);
      break;
    }
    case QIR_NODE_FOREACH: {
      ss << "form (" << n->as<Form>()->getIdxIdent() << "; ";
      ss << n->as<Form>()->getValIdent() << "; ";
      serialize_recurse(n->as<Form>()->getExpr(), ss, state);
      ss << ") ";
      serialize_recurse(n->as<Form>()->getBody(), ss, state);
      break;
    }
    case QIR_NODE_CASE: {
      ss << "case ";
      serialize_recurse(n->as<Case>()->getCond(), ss, state);
      ss << ": ";
      serialize_recurse(n->as<Case>()->getBody(), ss, state);
      break;
    }
    case QIR_NODE_SWITCH: {
      ss << "switch (";
      serialize_recurse(n->as<Switch>()->getCond(), ss, state);
      ss << ") {";
      state.indent++;
      indent(ss, state);
      for (auto it = n->as<Switch>()->getCases().begin(); it != n->as<Switch>()->getCases().end();
           ++it) {
        serialize_recurse(*it, ss, state);
        ss << ",";
        indent(ss, state);
      }
      ss << "default: ";
      serialize_recurse(n->as<Switch>()->getDefault(), ss, state);
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
        if (std::next(it) != n->as<Fn>()->getParams().end()) {
          ss << ",";
        }
      }
      ss << ") ";
      serialize_recurse(n->as<Fn>()->getBody(), ss, state);
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
      serialize_recurse(n->as<PtrTy>()->getPointee(), ss, state);
      ss << "*";
      break;
    }
    case QIR_NODE_OPAQUE_TY: {
      ss << "opaque ";
      ss << n->as<OpaqueTy>()->getName();
      break;
    }
    case QIR_NODE_STRING_TY: {
      ss << "string";
      break;
    }
    case QIR_NODE_STRUCT_TY: {
      ss << "struct {";
      state.indent++;
      indent(ss, state);
      for (auto it = n->as<StructTy>()->getFields().begin();
           it != n->as<StructTy>()->getFields().end(); ++it) {
        serialize_recurse(*it, ss, state);
        ss << ",";

        if (std::next(it) != n->as<StructTy>()->getFields().end()) {
          indent(ss, state);
        }
      }
      state.indent--;
      indent(ss, state);
      ss << "}";
      break;
    }
    case QIR_NODE_UNION_TY: {
      ss << "union {";
      state.indent++;
      indent(ss, state);
      for (auto it = n->as<UnionTy>()->getFields().begin();
           it != n->as<UnionTy>()->getFields().end(); ++it) {
        serialize_recurse(*it, ss, state);
        ss << ",";

        if (std::next(it) != n->as<UnionTy>()->getFields().end()) {
          indent(ss, state);
        }
      }
      state.indent--;
      indent(ss, state);
      ss << "}";
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      ss << "[";
      serialize_recurse(n->as<ArrayTy>()->getElement(), ss, state);
      ss << "; ";
      serialize_recurse(n->as<ArrayTy>()->getCount(), ss, state);
      ss << "]";
      break;
    }
    case QIR_NODE_LIST_TY: {
      ss << "[";
      serialize_recurse(n->as<ListTy>()->getElement(), ss, state);
      ss << "]";
      break;
    }
    case QIR_NODE_INTRIN_TY: {
      ss << "[";
      ss << escape_string(n->as<IntrinTy>()->getName());
      ss << "]";
      break;
    }
    case QIR_NODE_FN_TY: {
      ss << "fn (";
      for (auto it = n->as<FnTy>()->getParams().begin(); it != n->as<FnTy>()->getParams().end();
           ++it) {
        serialize_recurse(*it, ss, state);
        if (std::next(it) != n->as<FnTy>()->getParams().end()) {
          ss << ",";
        }
      }
      ss << "): ";
      serialize_recurse(n->as<FnTy>()->getReturn(), ss, state);
      break;
    }
    case QIR_NODE_TMP: {
      ss << "`" << static_cast<uint64_t>(n->as<Tmp>()->getTmpType());
      ss << ";" << n->as<Tmp>()->getData().type().name() << "`";
      break;
    }
    default: {
      qcore_panicf("Unknown node type: %d", n->getKind());
    }
  }
}

static char *qxir_repr(const Expr *_node, bool minify, size_t indent, qcore_arena_t *arena,
                       size_t *outlen) {
  std::swap(qxir_arena.get(), *arena);

  /* Create a string stream based on the arena */
  ConvStream ss;
  ConvState state = {0, indent, minify};
  const Expr *n = static_cast<const Expr *>(_node);

  /* Serialize the AST recursively */
  serialize_recurse(const_cast<Expr *>(n), ss, state);

  /**
   * @brief We can do the following because the std::string destructor will
   * invoke the arena's destructor, which is a no-op until the arena itself is
   * destroyed. So we can safely return the string's data pointer knowing it will exists for as long
   * as the arena does.
   */
  std::basic_string<char, std::char_traits<char>, Arena<char>> str = ss.str();
  *outlen = str.size();

  std::swap(qxir_arena.get(), *arena);

  /**
   * This will get around one of the compiler's warnings about returning a pointer to a temporary
   * object.
   */
  char *unsafe_bypass = static_cast<char *>(str.data());
  return unsafe_bypass;
}

static void raw_deflate(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size,
                        qcore_arena_t *arena) {
  struct libdeflate_compressor *ctx{};

  /* Allocate a compressor context; level 8 is a fairly good tradeoff */
  ctx = libdeflate_alloc_compressor(8);
  if (!ctx) {
    qcore_panic("Failed to allocate: libdeflate_compressor content. out-of-memory");
  }

  /* Compute the largest possible compressed buffer size */
  *out_size = libdeflate_deflate_compress_bound(ctx, in_size);

  /* Allocate memory for the compressed buffer */
  *out = static_cast<uint8_t *>(qcore_arena_alloc(arena, *out_size));

  if (!*out) {
    libdeflate_free_compressor(ctx);
    qcore_panic("Failed to allocate memory for compressed AST representation");
  }

  /* Compress the data */
  *out_size = libdeflate_deflate_compress(ctx, in, in_size, *out, *out_size);

  /* Liberate the compressor context */
  libdeflate_free_compressor(ctx);

  /* Check for compression failure */
  if (out_size == 0) {
    qcore_panic("Failed to compress AST representation");
  }
}

static void qxir_brepr(const Expr *node, bool compress, qcore_arena_t *arena, uint8_t **out,
                       size_t *outlen) {
  char *repr;

  /* Generate the AST representation as ASCII */
  if ((repr = qxir_repr(node, true, 0, arena, outlen)) == NULL) {
    qcore_panic("Failed to generate AST representation");
  }

  /* Compress the AST representation */
  if (compress) {
    uint8_t *tmp_out;
    raw_deflate((const uint8_t *)repr, *outlen, &tmp_out, outlen, arena);
    repr = (char *)tmp_out;
  }

  *out = (uint8_t *)repr;
}

LIB_EXPORT bool qxir_write(qmodule_t *mod, qxir_serial_t mode, FILE *out, size_t *outlen,
                           uint32_t argcnt, ...) {
  qcore_arena_t arena;
  size_t v_outlen;
  Expr *node;

  if (!outlen) {
    outlen = &v_outlen;
  }

  node = static_cast<Expr *>(mod->getRoot());

  qcore_arena_open(&arena);

  switch (mode) {
    case QXIR_SERIAL_CODE: {
      char *buf = qxir_repr(node, false, 2, &arena, outlen);
      fwrite(buf, 1, *outlen, out);
      break;
    }
    case QXIR_SERIAL_CODE_MIN: {
      char *buf = qxir_repr(node, true, 0, &arena, outlen);
      fwrite(buf, 1, *outlen, out);
      break;
    }
    case QXIR_SERIAL_B10: {
      uint8_t *buf;
      qxir_brepr(node, true, &arena, &buf, outlen);
      fwrite(buf, 1, *outlen, out);
      break;
    }
  }

  qcore_arena_close(&arena);

  return true;
}
