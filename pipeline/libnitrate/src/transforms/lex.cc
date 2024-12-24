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

#include <nitrate/code.h>

#include <core/SerialUtil.hh>
#include <core/Transform.hh>
#include <cstdint>
#include <nitrate-core/Init.hh>
#include <nitrate-lexer/Lexer.hh>
#include <string_view>
#include <unordered_set>

using namespace ncc::lex;

bool impl_use_json(IScanner *L, std::ostream &O) {
  O << "[";

  Token tok;
  while ((tok = (L->Next())).get_type() != qEofF) {
    uint32_t sl = L->StartLine(tok), sc = L->StartColumn(tok);
    uint32_t el = L->EndLine(tok), ec = L->EndColumn(tok);

    switch (tok.get_type()) {
      case qEofF: { /* End of file */
        break;
      }

      case qKeyW: { /* Keyword */
        O << "[2,\"" << tok << "\"," << sl << "," << sc << "," << el << ","
          << ec << "],";
        break;
      }

      case Oper: { /* Operator */
        O << "[3,\"" << tok << "\"," << sl << "," << sc << "," << el << ","
          << ec << "],";
        break;
      }

      case qPunc: { /* Punctuation */
        O << "[4,\"" << tok << "\"," << sl << "," << sc << "," << el << ","
          << ec << "],";
        break;
      }

      case qName: { /* Identifier */
        O << "[5,\"" << tok << "\"," << sl << "," << sc << "," << el << ","
          << ec << "],";
        break;
      }

      case qIntL: { /* Integer literal */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[6,\"" << tok << "\"," << sl << "," << sc << "," << el << ","
          << ec << "],";
        break;
      }

      case qNumL: { /* Floating-point literal */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[7,\"" << tok << "\"," << sl << "," << sc << "," << el << ","
          << ec << "],";
        break;
      }

      case qText: { /* String literal */
        std::string_view sv = tok.as_string();

        O << "[8,\"" << create_json_string(sv) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qChar: { /* Character literal */
        std::string_view sv = tok.as_string();

        O << "[9,\"" << create_json_string(sv) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }

      case qMacB: { /* Macro block */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[10,\"" << tok.as_string() << "\"," << sl << "," << sc << ","
          << el << "," << ec << "],";
        break;
      }

      case qMacr: { /* Macro call */
        /// We assume that int's are not allowed to contain NULL bytes and
        O << "[11,\"" << tok.as_string() << "\"," << sl << "," << sc << ","
          << el << "," << ec << "],";
        break;
      }

      case qNote: { /* Comment */
        std::string_view sv = tok.as_string();

        O << "[12,\"" << create_json_string(sv) << "\"," << sl << "," << sc
          << "," << el << "," << ec << "],";
        break;
      }
    }

    O.flush();
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

bool impl_use_msgpack(IScanner *L, std::ostream &O) {
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

  Token tok;
  while ((tok = (L->Next())).get_type() != qEofF) {
    uint32_t sl = L->StartLine(tok), sc = L->StartColumn(tok);
    uint32_t el = L->EndLine(tok), ec = L->EndColumn(tok);

    switch (tok.get_type()) {
      case qEofF: { /* End of file */
        break;
      }

      case qKeyW: { /* Keyword */
        msgpack_write_tok(O, 3, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case Oper: { /* Operator */
        msgpack_write_tok(O, 4, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qPunc: { /* Punctuation */
        msgpack_write_tok(O, 5, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qName: { /* Identifier */
        msgpack_write_tok(O, 6, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qIntL: { /* Integer literal */
        msgpack_write_tok(O, 7, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qNumL: { /* Floating-point literal */
        msgpack_write_tok(O, 8, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qText: { /* String literal */
        msgpack_write_tok(O, 9, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qChar: { /* Character literal */
        msgpack_write_tok(O, 10, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qMacB: { /* Macro block */
        msgpack_write_tok(O, 11, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qMacr: { /* Macro call */
        msgpack_write_tok(O, 12, tok.as_string(), sl, sc, el, ec);
        break;
      }

      case qNote: { /* Comment */
        msgpack_write_tok(O, 13, tok.as_string(), sl, sc, el, ec);
        break;
      }
    }

    O.flush();

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

CREATE_TRANSFORM(nit::lex) {
  auto L = Tokenizer(source, env);

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
      return impl_use_json(&L, output);
    case OutMode::MsgPack:
      return impl_use_msgpack(&L, output);
  }
}
