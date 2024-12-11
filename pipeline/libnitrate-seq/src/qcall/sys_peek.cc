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

#include <nitrate-lexer/Lexer.h>

#include <core/Preprocess.hh>
#include <qcall/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

int qcall::sys_peek(lua_State* L) {
  /**
   *   @brief Next token.
   */

  qprep_impl_t* obj = get_engine();

  qlex_tok_t tok;

  { /* Get token */
    qlex_flags_t flags;
    bool old;

    old = obj->m_do_expanse;
    flags = qlex_get_flags(obj);

    obj->m_do_expanse = false;
    qlex_set_flags(obj, flags & ~(QLEX_NO_COMMENTS));

    tok = qlex_peek(obj);

    qlex_set_flags(obj, flags);
    obj->m_do_expanse = old;
  }

  lua_newtable(L);

  lua_pushstring(L, "ty");
  lua_pushstring(L, qlex_ty_str(tok.ty));
  lua_settable(L, -3);

  lua_pushstring(L, "v");
  switch (tok.ty) {
    case qEofF:
    case qKeyW: {
      lua_pushstring(L, qlex_kwstr(tok.v.key));
      break;
    }
    case qOper: {
      lua_pushstring(L, qlex_opstr(tok.v.op));
      break;
    }
    case qPunc: {
      lua_pushstring(L, qlex_punctstr(tok.v.punc));
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
      lua_pushstring(L, obj->get_string(tok.v.str_idx).data());
      break;
    }
  }

  lua_settable(L, -3);

  return 1;
}
