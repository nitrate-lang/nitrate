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
    return luaL_error(L, "sys_defer: expected at least 1 argument, got %d", nargs);
  }

  if (!lua_isfunction(L, 1)) {
    return luaL_error(L, "sys_defer: expected function as first argument, got %s",
                      luaL_typename(L, 1));
  }

  int id = luaL_ref(L, LUA_REGISTRYINDEX);
  if (id == LUA_REFNIL) {
    return luaL_error(L, "sys_defer: failed to store callback in registry");
  }

  DeferCallback cb = [L, id](qprep_impl_t* obj, qlex_tok_t tok) -> DeferOp {
    lua_rawgeti(L, LUA_REGISTRYINDEX, id); /* Push the function */

    { /* Push the function arguments */
      lua_newtable(L);

      lua_pushstring(L, "ty");
      lua_pushstring(L, qlex_ty_str(tok.ty));
      lua_settable(L, -3);

      lua_pushstring(L, "v");
      switch (tok.ty) {
        case qEofF:
        case qErro: {
          lua_pushnil(L);
          break;
        }
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

    if (lua_pcall(L, 1, 1, 0) != 0) {
      qcore_print(QCORE_ERROR, "sys_defer: %s\n", lua_tostring(L, -1));
      return DeferOp::EmitToken;
    }

    if (lua_isnil(L, -1)) {
      return DeferOp::UninstallHandler;
    }

    if (!lua_isboolean(L, -1)) {
      qcore_print(QCORE_ERROR, "sys_defer: expected boolean return value or nil, got %s\n",
                  luaL_typename(L, -1));
      return DeferOp::EmitToken;
    }

    return lua_toboolean(L, -1) ? DeferOp::EmitToken : DeferOp::SkipToken;
  };

  get_engine()->m_core->defer_callbacks.push_back(cb);

  return 0;
}
