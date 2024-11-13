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

#define LIBNITRATE_INTERNAL

#include <nitrate-core/Lib.h>
#include <nitrate/code.h>

#include <SerialUtil.hh>
#include <cstdint>
#include <functional>
#include <nitrate-seq/Classes.hh>
#include <string_view>
#include <unordered_set>

static bool impl_use_json(qlex_t *L, FILE *O) {
  fputc('[', O);

  qlex_tok_t tok;
  while ((tok = qlex_next(L)).ty != qEofF) {
    uint32_t sl = qlex_line(L, tok.start);
    uint32_t sc = qlex_col(L, tok.start);
    uint32_t el = qlex_line(L, tok.end);
    uint32_t ec = qlex_col(L, tok.end);

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

  fprintf(O, "[1,\"\",0,0,0,0]]");

  return true;
}

static bool msgpack_write_tok(FILE *O, uint8_t t, std::string_view v, uint32_t a, uint32_t b,
                              uint32_t c, uint32_t d) {
  fputc(0b10010000 | 6, O);

  // Write type
  if (!msgpack_write_uint(O, t)) return false;

  // Write value
  if (!msgpack_write_str(O, v)) return false;

  // Write start line
  if (!msgpack_write_uint(O, a)) return false;

  // Write start column
  if (!msgpack_write_uint(O, b)) return false;

  // Write end line
  if (!msgpack_write_uint(O, c)) return false;

  // Write end column
  if (!msgpack_write_uint(O, d)) return false;

  return true;
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
    uint32_t sl = qlex_line(L, tok.start);
    uint32_t sc = qlex_col(L, tok.start);
    uint32_t el = qlex_line(L, tok.end);
    uint32_t ec = qlex_col(L, tok.end);

    switch (tok.ty) {
      case qEofF: { /* End of file */
        __builtin_unreachable();
        break;
      }

      case qErro: { /* Error, invalid token */
        if (!msgpack_write_tok(O, 2, "", sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qKeyW: { /* Keyword */
        if (!msgpack_write_tok(O, 3, qlex_kwstr(tok.v.key), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qOper: { /* Operator */
        if (!msgpack_write_tok(O, 4, qlex_opstr(tok.v.op), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qPunc: { /* Punctuation */
        if (!msgpack_write_tok(O, 5, qlex_punctstr(tok.v.punc), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qName: { /* Identifier */
        if (!msgpack_write_tok(O, 6, qlex_str(L, &tok, nullptr), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qIntL: { /* Integer literal */
        if (!msgpack_write_tok(O, 7, qlex_str(L, &tok, nullptr), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qNumL: { /* Floating-point literal */
        if (!msgpack_write_tok(O, 8, qlex_str(L, &tok, nullptr), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qText: { /* String literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        if (!msgpack_write_tok(O, 9, sv, sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qChar: { /* Character literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        if (!msgpack_write_tok(O, 10, sv, sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qMacB: { /* Macro block */
        if (!msgpack_write_tok(O, 11, qlex_str(L, &tok, nullptr), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qMacr: { /* Macro call */
        if (!msgpack_write_tok(O, 12, qlex_str(L, &tok, nullptr), sl, sc, el, ec)) {
          return false;
        }
        break;
      }

      case qNote: { /* Comment */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        if (!msgpack_write_tok(O, 13, sv, sl, sc, el, ec)) {
          return false;
        }
        break;
      }
    }

    num_entries++;
  }

  if (!msgpack_write_tok(O, qEofF, "", 0, 0, 0, 0)) {
    return false;
  }
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

bool impl_subsys_meta(std::shared_ptr<std::istream> source, FILE *output,
                      std::function<void(const char *)> diag_cb,
                      const std::unordered_set<std::string_view> &opts) {
  (void)diag_cb;

  qprep lexer(source, nullptr, qcore_env_current());

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
