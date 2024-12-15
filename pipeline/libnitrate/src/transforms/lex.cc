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

#include <nitrate-core/Lib.h>
#include <nitrate/code.h>

#include <core/SerialUtil.hh>
#include <core/Transformer.hh>
#include <nitrate-lexer/Classes.hh>
#include <string_view>
#include <unordered_set>

bool impl_use_json(qlex_t *L, std::ostream &O) {
  O << "[";

  qlex_tok_t tok;
  while ((tok = qlex_next(L)).ty != qEofF) {
    uint32_t sl = qlex_line(L, tok.start);
    uint32_t sc = qlex_col(L, tok.start);
    uint32_t el = qlex_line(L, tok.end);
    uint32_t ec = qlex_col(L, tok.end);

    switch (tok.ty) {
      case qEofF: { /* End of file */
        break;
      }

      case qKeyW: { /* Keyword */
        O << "[2,\"" << qlex_kwstr(tok.v.key) << "\"," << sl << "," << sc << ","
          << el << "," << ec << "],";
        break;
      }

      case qOper: { /* Operator */
        O << "[3,\"" << qlex_opstr(tok.v.op) << "\"," << sl << "," << sc << ","
          << el << "," << ec << "],";
        break;
      }

      case qPunc: { /* Punctuation */
        O << "[4,\"" << qlex_punctstr(tok.v.punc) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qName: { /* Identifier */
        O << "[5,\"" << qlex_str(L, &tok, nullptr) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qIntL: { /* Integer literal */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[6,\"" << qlex_str(L, &tok, nullptr) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qNumL: { /* Floating-point literal */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[7,\"" << qlex_str(L, &tok, nullptr) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qText: { /* String literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        O << "[8,\"" << create_json_string(sv) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qChar: { /* Character literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        O << "[9,\"" << create_json_string(sv) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qMacB: { /* Macro block */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[10,\"" << qlex_str(L, &tok, nullptr) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qMacr: { /* Macro call */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[11,\"" << qlex_str(L, &tok, nullptr) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qNote: { /* Comment */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        O << "[12,\"" << create_json_string(sv) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }
    }
  }

  O << "[1,\"\",0,0,0,0]]";

  return true;
}

static void msgpack_write_tok(std::ostream &O, uint8_t type,
                              std::string_view val, uint32_t sl, uint32_t sc,
                              uint32_t el, uint32_t ec) {
  O.put(0x96);

  msgpack_write_uint(O, type);
  msgpack_write_str(O, val);
  msgpack_write_uint(O, sl);
  msgpack_write_uint(O, sc);
  msgpack_write_uint(O, el);
  msgpack_write_uint(O, ec);
}

bool impl_use_msgpack(qlex_t *L, std::ostream &O) {
  size_t num_entries = 0;

  O.put(0xdd);

  off_t offset = O.tellp();
  if (offset == -1) {
    return false;
  }

  O.put(0);
  O.put(0);
  O.put(0);
  O.put(0);

  qlex_tok_t tok;
  while ((tok = qlex_next(L)).ty != qEofF) {
    uint32_t sl = qlex_line(L, tok.start);
    uint32_t sc = qlex_col(L, tok.start);
    uint32_t el = qlex_line(L, tok.end);
    uint32_t ec = qlex_col(L, tok.end);

    switch (tok.ty) {
      case qEofF: { /* End of file */
        break;
      }

      case qKeyW: { /* Keyword */
        msgpack_write_tok(O, 3, qlex_kwstr(tok.v.key), sl, sc, el, ec);
        break;
      }

      case qOper: { /* Operator */
        msgpack_write_tok(O, 4, qlex_opstr(tok.v.op), sl, sc, el, ec);
        break;
      }

      case qPunc: { /* Punctuation */
        msgpack_write_tok(O, 5, qlex_punctstr(tok.v.punc), sl, sc, el, ec);
        break;
      }

      case qName: { /* Identifier */
        msgpack_write_tok(O, 6, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qIntL: { /* Integer literal */
        msgpack_write_tok(O, 7, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qNumL: { /* Floating-point literal */
        msgpack_write_tok(O, 8, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qText: { /* String literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        msgpack_write_tok(O, 9, sv, sl, sc, el, ec);
        break;
      }

      case qChar: { /* Character literal */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        msgpack_write_tok(O, 10, sv, sl, sc, el, ec);
        break;
      }

      case qMacB: { /* Macro block */
        msgpack_write_tok(O, 11, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qMacr: { /* Macro call */
        msgpack_write_tok(O, 12, qlex_str(L, &tok, nullptr), sl, sc, el, ec);
        break;
      }

      case qNote: { /* Comment */
        size_t S;
        const char *str = qlex_str(L, &tok, &S);
        std::string_view sv(str, S);

        msgpack_write_tok(O, 13, sv, sl, sc, el, ec);
        break;
      }
    }

    num_entries++;
  }

  msgpack_write_tok(O, qEofF, "", 0, 0, 0, 0);
  num_entries++;

  off_t end_offset = O.tellp();
  if (end_offset == -1) {
    return false;
  }

  O.seekp(offset, std::ios_base::beg);

  O.put((num_entries >> 24) & 0xff);
  O.put((num_entries >> 16) & 0xff);
  O.put((num_entries >> 8) & 0xff);
  O.put(num_entries & 0xff);

  O.seekp(end_offset, std::ios_base::beg);

  return true;
}

bool nit::lex(std::istream &source, std::ostream &output,
              const std::unordered_set<std::string_view> &opts) {
  qlex lexer(source, nullptr, qcore_env_current());

  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    qcore_logf(QCORE_ERROR, "Cannot use both JSON and MsgPack output.");
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
