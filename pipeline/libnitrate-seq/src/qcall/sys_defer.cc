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

#include <core/Preprocess.hh>
#include <qcall/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

int qcall::sys_defer(lua_State* L) {
  /**
   *   @brief Defer token callback.
   */

  int nargs = lua_gettop(L);
  if (nargs < 1) {
    return luaL_error(L, "sys_defer: expected at least 1 argument, got %d",
                      nargs);
  }

  if (!lua_isfunction(L, 1)) {
    return luaL_error(L,
                      "sys_defer: expected function as first argument, got %s",
                      luaL_typename(L, 1));
  }

  int id = luaL_ref(L, LUA_REGISTRYINDEX);
  if (id == LUA_REFNIL) {
    return luaL_error(L, "sys_defer: failed to store callback in registry");
  }

  DeferCallback cb = [L, id](qprep_impl_t* obj, qlex_tok_t tok) -> DeferOp {
    lua_rawgeti(L, LUA_REGISTRYINDEX, id); /* Get the function */

    { /* Push the function arguments */
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
    }

    int err = lua_pcall(L, 1, 1, 0);
    DeferOp R;

    switch (err) {
      case LUA_OK: {
        if (lua_isnil(L, -1)) {
          return DeferOp::UninstallHandler;
        }

        if (!lua_isboolean(L, -1)) {
          qcore_logf(
              QCORE_ERROR,
              "sys_defer: expected boolean return value or nil, got %s\n",
              luaL_typename(L, -1));
          return DeferOp::EmitToken;
        }

        R = lua_toboolean(L, -1) ? DeferOp::EmitToken : DeferOp::SkipToken;
        break;
      }
      case LUA_ERRRUN: {
        qcore_logf(QCORE_ERROR, "sys_defer: lua: %s\n", lua_tostring(L, -1));
        R = DeferOp::EmitToken;
        break;
      }
      case LUA_ERRMEM: {
        qcore_logf(QCORE_ERROR, "sys_defer: memory allocation error\n");
        R = DeferOp::EmitToken;
        break;
      }
      case LUA_ERRERR: {
        qcore_logf(QCORE_ERROR, "sys_defer: error in error handler\n");
        R = DeferOp::EmitToken;
        break;
      }
      default: {
        qcore_logf(QCORE_ERROR, "sys_defer: unexpected error %d\n", err);
        R = DeferOp::EmitToken;
        break;
      }
    }

    lua_pop(L, 1);

    return R;
  };

  get_engine()->m_core->defer_callbacks.push_back(cb);

  return 0;
}
