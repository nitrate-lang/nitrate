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

#define LIBQUIX_INTERNAL

#include <quix-core/Lib.h>
#include <quix/code.h>

#include <cstdint>
#include <functional>
#include <quix-lexer/Classes.hh>
#include <string_view>
#include <unordered_set>

static std::string create_json_string(std::string_view input) {
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

static bool impl_use_json(qlex_t *L, FILE *O) {
  fputc('[', O);

  qlex_tok_t tok;
  while ((tok = qlex_next(L)).ty != qEofF) {
    qlex_size sl = qlex_line(L, tok.start);
    qlex_size sc = qlex_col(L, tok.start);
    qlex_size el = qlex_line(L, tok.end);
    qlex_size ec = qlex_col(L, tok.end);

    switch (tok.ty) {
      case qEofF: { /* End of file */
        __builtin_unreachable();
        break;
      }

      case qErro: { /* Error, invalid token */
        fprintf(O, "[2,\"\",%u,%u,%u,%u],", sl, sc, el, ec);
        break;
      }

      case qKeyW: { /* Keyword */
        fprintf(O, "[3,\"%s\",%u,%u,%u,%u],", qlex_kwstr(tok.v.key), sl, sc, el, ec);
        break;
      }

      case qOper: { /* Operator */
        fprintf(O, "[4,\"%s\",%u,%u,%u,%u],", qlex_opstr(tok.v.op), sl, sc, el, ec);
        break;
      }

      case qPunc: { /* Punctuation */
        fprintf(O, "[5,\"%s\",%u,%u,%u,%u],", qlex_punctstr(tok.v.punc), sl, sc, el, ec);
        break;
      }

      case qName: { /* Identifier */
        /// We assume that identifiers are not allowed to contain NULL bytes and other special
        /// characters.
        fprintf(O, "[6,\"%s\",%u,%u,%u,%u],", qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qIntL: { /* Integer literal */
        /// We assume that int's are not allowed to contain NULL bytes and other special
        /// characters.
        fprintf(O, "[7,\"%s\",%u,%u,%u,%u],", qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qNumL: { /* Floating-point literal */
        /// We assume that int's are not allowed to contain NULL bytes and other special
        /// characters.
        fprintf(O, "[8,\"%s\",%u,%u,%u,%u],", qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qText: { /* String literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        fprintf(O, "[9,%s,%u,%u,%u,%u],", create_json_string(sv).c_str(), sl, sc, el, ec);
        break;
      }

      case qChar: { /* Character literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        fprintf(O, "[10,%s,%u,%u,%u,%u],", create_json_string(sv).c_str(), sl, sc, el, ec);
        break;
      }

      case qMacB: { /* Macro block */
        /// We assume that int's are not allowed to contain NULL bytes and other special
        /// characters.
        fprintf(O, "[11,\"%s\",%u,%u,%u,%u],", qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qMacr: { /* Macro call */
        /// We assume that int's are not allowed to contain NULL bytes and other special
        /// characters.
        fprintf(O, "[12,\"%s\",%u,%u,%u,%u],", qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qNote: { /* Comment */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        fprintf(O, "[13,%s,%u,%u,%u,%u],", create_json_string(sv).c_str(), sl, sc, el, ec);
        break;
      }
    }
  }

  fprintf(O, "[0,\"\",0,0,0,0]]");

  return true;
}

static void msgpack_write_uint(int &err, FILE *O, uint64_t x) {
  if (x <= INT8_MAX) {
    err |= fputc(x & 0x7f, O);
  } else if (x <= UINT8_MAX) {
    err |= fputc(0xcc, O);
    err |= fputc(x, O);
  } else if (x <= UINT16_MAX) {
    err |= fputc(0xcd, O);
    err |= fputc((x >> 8) & 0xff, O);
    err |= fputc(x & 0xff, O);
  } else if (x <= UINT32_MAX) {
    err |= fputc(0xce, O);
    err |= fputc((x >> 24) & 0xff, O);
    err |= fputc((x >> 16) & 0xff, O);
    err |= fputc((x >> 8) & 0xff, O);
    err |= fputc(x & 0xff, O);
  } else {
    err |= fputc(0xcf, O);
    err |= fputc((x >> 56) & 0xff, O);
    err |= fputc((x >> 48) & 0xff, O);
    err |= fputc((x >> 40) & 0xff, O);
    err |= fputc((x >> 32) & 0xff, O);
    err |= fputc((x >> 24) & 0xff, O);
    err |= fputc((x >> 16) & 0xff, O);
    err |= fputc((x >> 8) & 0xff, O);
    err |= fputc(x & 0xff, O);
  }
}

static void msgpack_write_str(int &err, FILE *O, const char *buf, size_t sz) {
  if (sz <= 31) {
    err |= fputc(0b10100000 | sz, O);
  } else if (sz <= UINT8_MAX) {
    err |= fputc(0xd9, O);
    err |= fputc(sz, O);
  } else if (sz <= UINT16_MAX) {
    err |= fputc(0xda, O);
    err |= fputc((sz >> 8) & 0xff, O);
    err |= fputc(sz & 0xff, O);
  } else if (sz <= UINT32_MAX) {
    err |= fputc(0xdb, O);
    err |= fputc((sz >> 24) & 0xff, O);
    err |= fputc((sz >> 16) & 0xff, O);
    err |= fputc((sz >> 8) & 0xff, O);
    err |= fputc(sz & 0xff, O);
  }

  fwrite(buf, 1, sz, O);
}

static void msgpack_write_tok(int &err, FILE *O, uint8_t t, std::string_view v, uint32_t a,
                              uint32_t b, uint32_t c, uint32_t d) {
  fputc(0b10010000 | 6, O);

  // Write type
  msgpack_write_uint(err, O, t);

  // Write value
  msgpack_write_str(err, O, v.data(), v.size());

  // Write start line
  msgpack_write_uint(err, O, a);

  // Write start column
  msgpack_write_uint(err, O, b);

  // Write end line
  msgpack_write_uint(err, O, c);

  // Write end column
  msgpack_write_uint(err, O, d);
}

static bool impl_use_msgpack(qlex_t *L, FILE *O) {
  size_t num_entries = 0;

  fputc(0xdd, O);

  off_t offset = ftello(O);
  if (offset == -1) {
    return false;
  }

  /* Placeholder value */
  int err = 0;

  err |= fputc(0, O);
  err |= fputc(0, O);
  err |= fputc(0, O);
  err |= fputc(0, O);

  qlex_tok_t tok;
  while ((tok = qlex_next(L)).ty != qEofF) {
    qlex_size sl = qlex_line(L, tok.start);
    qlex_size sc = qlex_col(L, tok.start);
    qlex_size el = qlex_line(L, tok.end);
    qlex_size ec = qlex_col(L, tok.end);

    switch (tok.ty) {
      case qEofF: { /* End of file */
        __builtin_unreachable();
        break;
      }

      case qErro: { /* Error, invalid token */
        msgpack_write_tok(err, O, 2, "", sl, sc, el, ec);
        break;
      }

      case qKeyW: { /* Keyword */
        msgpack_write_tok(err, O, 3, qlex_kwstr(tok.v.key), sl, sc, el, ec);
        break;
      }

      case qOper: { /* Operator */
        msgpack_write_tok(err, O, 4, qlex_opstr(tok.v.op), sl, sc, el, ec);
        break;
      }

      case qPunc: { /* Punctuation */
        msgpack_write_tok(err, O, 5, qlex_punctstr(tok.v.punc), sl, sc, el, ec);
        break;
      }

      case qName: { /* Identifier */
        msgpack_write_tok(err, O, 6, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qIntL: { /* Integer literal */
        msgpack_write_tok(err, O, 7, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qNumL: { /* Floating-point literal */
        msgpack_write_tok(err, O, 8, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qText: { /* String literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        msgpack_write_tok(err, O, 9, sv, sl, sc, el, ec);
        break;
      }

      case qChar: { /* Character literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        msgpack_write_tok(err, O, 10, sv, sl, sc, el, ec);
        break;
      }

      case qMacB: { /* Macro block */
        msgpack_write_tok(err, O, 11, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qMacr: { /* Macro call */
        msgpack_write_tok(err, O, 12, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qNote: { /* Comment */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        msgpack_write_tok(err, O, 13, sv, sl, sc, el, ec);
        break;
      }
    }

    num_entries++;
  }

  msgpack_write_tok(err, O, 0, "", 0, 0, 0, 0);
  num_entries++;

  off_t end_offset = ftello(O);
  if (end_offset == -1) {
    return false;
  }

  if (fseeko(O, offset, SEEK_SET) == -1) {
    return false;
  }

  err |= fputc((num_entries >> 24) & 0xff, O);
  err |= fputc((num_entries >> 16) & 0xff, O);
  err |= fputc((num_entries >> 8) & 0xff, O);
  err |= fputc(num_entries & 0xff, O);

  if (fseeko(O, end_offset, SEEK_SET) == -1) {
    return false;
  }

  return err != EOF;
}

bool impl_subsys_basic_lexer(FILE *source, FILE *output, std::function<void(const char *)> diag_cb,
                             const std::unordered_set<std::string_view> &opts) {
  (void)diag_cb;

  qlex lexer(source, nullptr, qcore_env_current());

  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    qcore_print(QCORE_ERROR, "Cannot use both JSON and MsgPack output.");
    return false;
  } else if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  switch (out_mode) {
    case OutMode::JSON:
      return impl_use_json(lexer.get(), output);
    case OutMode::MsgPack:
      return impl_use_msgpack(lexer.get(), output);
  }
}
