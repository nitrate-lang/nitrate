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
#include <string>

#include "LibMacro.h"

using namespace qxir;

struct ConvState {
  int32_t indent;
  size_t indent_width;
  bool minify;
};

typedef std::basic_stringstream<char, std::char_traits<char>, Arena<char>> ConvStream;

static std::string escape_string(const std::string &input) {
  std::string output = "\"";
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

static void serialize_recurse(Expr *n, ConvStream &ss, ConvState &state) {
  if (!n) {
    // Nicely handle null nodes
    OBJECT_EMPTY("Nil");
    return;
  }

  switch (n->thisTypeId()) {
    default:
      qcore_panicf("Unknown node type: %d", n->thisTypeId());
      break;

      (void)escape_string;
      /// TODO:
  }
}

static char *qxir_repr_arena(const Expr *_node, bool minify, size_t indent, qcore_arena_t *arena,
                             size_t *outlen) {
  qxir_arena.swap(*arena);

  /* Create a string stream based on the arena */
  ConvStream ss;
  ConvState state = {-1, indent, minify};
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

  qxir_arena.swap(*arena);

  char *unsafe_bypass = static_cast<char *>(str.data());

  return unsafe_bypass;
}

static char *qxir_repr_malloc(const Expr *_node, bool minify, size_t indent, size_t *outlen) {
  qcore_arena_t scratch;
  char *out = nullptr, *out_tmp = nullptr;

  qcore_arena_open(&scratch);

  try {
    out = qxir_repr_arena(_node, minify, indent, &scratch, outlen);

    if (out) {
      out_tmp = static_cast<char *>(malloc(*outlen));
      if (!out_tmp) {
        qcore_panic("Failed to allocate memory for AST representation");
      }

      memcpy(out_tmp, out, *outlen);

      out = out_tmp;
    }

  } catch (...) {
  }

  qcore_arena_close(&scratch);

  return out;
}

LIB_EXPORT char *qxir_repr(const Expr *_node, bool minify, size_t indent, qcore_arena_t *arena,
                           size_t *outlen) {
  size_t outlen_v = 0;

  /* Eliminate internal edge cases */
  if (!outlen) {
    outlen = &outlen_v;
  }

  if (arena) {
    return qxir_repr_arena(_node, minify, indent, arena, outlen);
  } else {
    return qxir_repr_malloc(_node, minify, indent, outlen);
  }
}

static void raw_deflate(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size,
                        qcore_arena_t *arena) {
  struct libdeflate_compressor *ctx{};

  /* Allocate a compressor context; level 8 is a fairly good tradeoff */
  ctx = libdeflate_alloc_compressor(8);

  /* Compute the largest possible compressed buffer size */
  *out_size = libdeflate_deflate_compress_bound(ctx, in_size);

  /* Allocate memory for the compressed buffer */
  *out = (uint8_t *)qcore_arena_alloc(arena, *out_size);

  if (*out == NULL) {
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

LIB_EXPORT void qxir_brepr(const Expr *node, bool compress, qcore_arena_t *arena, uint8_t **out,
                           size_t *outlen) {
  char *repr{};
  qcore_arena_t scratch{};
  bool our_arena{};

  /* Open a scratch arena if one is not provided */
  if (!arena) {
    qcore_arena_open(&scratch);
    arena = &scratch;
    our_arena = true;
  }

  /* Validate the output buffer */
  if (!out || !outlen) {
    qcore_panic("Invalid output buffer for AST representation");
  }

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

  /* Copy the AST representation to the output buffer, if necessary */
  if (our_arena) {
    *out = (uint8_t *)malloc(*outlen);
    if (!*out) {
      qcore_panic("Failed to allocate memory for AST representation");
    }

    memcpy(*out, repr, *outlen);
    qcore_arena_close(arena);
  } else {
    /* Otherwise, just return the pointer to the arena alloc'ed buffer */
    *out = (uint8_t *)repr;
  }
}
