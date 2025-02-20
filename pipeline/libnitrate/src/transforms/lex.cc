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

using namespace ncc;
using namespace ncc::lex;

auto ImplUseJson(IScanner *l, std::ostream &o) -> bool {
  o << "[";

  Token tok;
  while ((tok = (l->Next())).GetKind() != EofF) {
    uint32_t sl = l->StartLine(tok);
    uint32_t sc = l->StartColumn(tok);
    uint32_t el = l->EndLine(tok);
    uint32_t ec = l->EndColumn(tok);

    switch (tok.GetKind()) {
      case EofF: { /* End of file */
        break;
      }

      case KeyW: { /* Keyword */
        o << "[2," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case Oper: { /* Operator */
        o << "[3," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case Punc: { /* Punctuation */
        o << "[4," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case Name: { /* Identifier */
        o << "[5," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case IntL: { /* Integer literal */
        /// We assume that int's are not allowed to contain NULL bytes and
        o << "[6," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case NumL: { /* Floating-point literal */
        /// We assume that int's are not allowed to contain NULL bytes and
        o << "[7," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case Text: { /* String literal */
        o << "[8," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case Char: { /* Character literal */

        o << "[9," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case MacB: { /* Macro block */
        /// We assume that int's are not allowed to contain NULL bytes and
        o << "[10," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case Macr: { /* Macro call */
        /// We assume that int's are not allowed to contain NULL bytes and
        o << "[11," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }

      case Note: { /* Comment */
        o << "[12," << CreateJsonString(tok.AsString()) << "," << sl << "," << sc << "," << el << "," << ec << "],";
        break;
      }
    }

    o.flush();
  }

  o << "[1,\"\",0,0,0,0]]";

  return !l->HasError();
}

static void MsgpackWriteTok(std::ostream &o, uint8_t type, std::string_view val, uint32_t sl, uint32_t sc, uint32_t el,
                            uint32_t ec) {
  o.put(0x96);

  MsgpackWriteUint(o, type);
  MsgpackWriteStr(o, val);
  MsgpackWriteUint(o, sl);
  MsgpackWriteUint(o, sc);
  MsgpackWriteUint(o, el);
  MsgpackWriteUint(o, ec);
}

auto ImplUseMsgpack(IScanner *l, std::ostream &o) -> bool {
  size_t num_entries = 0;

  o.put(0xdd);

  off_t offset = o.tellp();
  if (offset == -1) {
    return false;
  }

  o.put(0);
  o.put(0);
  o.put(0);
  o.put(0);

  Token tok;
  while ((tok = (l->Next())).GetKind() != EofF) {
    uint32_t sl = l->StartLine(tok);
    uint32_t sc = l->StartColumn(tok);
    uint32_t el = l->EndLine(tok);
    uint32_t ec = l->EndColumn(tok);

    switch (tok.GetKind()) {
      case EofF: { /* End of file */
        break;
      }

      case KeyW: { /* Keyword */
        MsgpackWriteTok(o, 3, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case Oper: { /* Operator */
        MsgpackWriteTok(o, 4, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case Punc: { /* Punctuation */
        MsgpackWriteTok(o, 5, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case Name: { /* Identifier */
        MsgpackWriteTok(o, 6, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case IntL: { /* Integer literal */
        MsgpackWriteTok(o, 7, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case NumL: { /* Floating-point literal */
        MsgpackWriteTok(o, 8, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case Text: { /* String literal */
        MsgpackWriteTok(o, 9, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case Char: { /* Character literal */
        MsgpackWriteTok(o, 10, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case MacB: { /* Macro block */
        MsgpackWriteTok(o, 11, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case Macr: { /* Macro call */
        MsgpackWriteTok(o, 12, tok.AsString(), sl, sc, el, ec);
        break;
      }

      case Note: { /* Comment */
        MsgpackWriteTok(o, 13, tok.AsString(), sl, sc, el, ec);
        break;
      }
    }

    o.flush();

    num_entries++;
  }

  MsgpackWriteTok(o, EofF, "", 0, 0, 0, 0);
  num_entries++;

  off_t end_offset = o.tellp();
  if (end_offset == -1) {
    return false;
  }

  o.seekp(offset, std::ios_base::beg);

  o.put((num_entries >> 24) & 0xff);
  o.put((num_entries >> 16) & 0xff);
  o.put((num_entries >> 8) & 0xff);
  o.put(num_entries & 0xff);

  o.seekp(end_offset, std::ios_base::beg);

  return !l->HasError();
}

CREATE_TRANSFORM(nit::lex) {
  auto l = Tokenizer(source, env);

  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    Log << "Cannot use both JSON and MsgPack output.";
    return false;
  }

  if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  switch (out_mode) {
    case OutMode::JSON:
      return ImplUseJson(&l, output);
    case OutMode::MsgPack:
      return ImplUseMsgpack(&l, output);
  }
}
