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

#include <nitrate-lexer/Lexer.hh>
#include <nitrate-seq/Preprocess.hh>
#include <qcall/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc::lex;

int qcall::sys_next(lua_State* L) {
  Token tok = get_engine()->Next();

  lua_newtable(L);

  lua_pushstring(L, "ty");
  lua_pushstring(L, qlex_ty_str(tok.ty));
  lua_settable(L, -3);

  lua_pushstring(L, "v");
  switch (tok.ty) {
    case qEofF:
    case qKeyW: {
      lua_pushstring(L, kw_repr(tok.as_key()));
      break;
    }
    case qOper: {
      lua_pushstring(L, op_repr(tok.as_op()));
      break;
    }
    case qPunc: {
      lua_pushstring(L, punct_repr(tok.as_punc()));
      break;
    }
    case qIntL:
    case qNumL:
    case qText:
    case qChar:
    case qName:
    case qMacB:
    case qMacr:
    case qNote: {
      lua_pushstring(L, std::string(tok.as_string()).c_str());
      break;
    }
  }

  lua_settable(L, -3);

  return 1;
}
