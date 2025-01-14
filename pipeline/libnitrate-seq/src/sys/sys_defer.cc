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

#include <nitrate-seq/Sequencer.hh>
#include <sys/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc::lex;

int ncc::seq::sys_defer(lua_State* L) {
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

  Sequencer::DeferCallback cb = [L, id](Sequencer*,
                                        Token tok) -> Sequencer::DeferOp {
    lua_rawgeti(L, LUA_REGISTRYINDEX, id); /* Get the function */

    { /* Push the function arguments */
      lua_newtable(L);

      lua_pushstring(L, "ty");
      lua_pushstring(L, qlex_ty_str(tok.get_type()));
      lua_settable(L, -3);

      lua_pushstring(L, "v");
      switch (tok.get_type()) {
        case EofF:
        case KeyW: {
          lua_pushstring(L, kw_repr(tok.as_key()));
          break;
        }
        case Oper: {
          lua_pushstring(L, op_repr(tok.as_op()));
          break;
        }
        case Punc: {
          lua_pushstring(L, punct_repr(tok.as_punc()));
          break;
        }
        case IntL:
        case NumL:
        case Text:
        case Char:
        case Name:
        case MacB:
        case Macr:
        case Note: {
          lua_pushstring(L, std::string(tok.as_string()).c_str());
          break;
        }
      }

      lua_settable(L, -3);
    }

    int err = lua_pcall(L, 1, 1, 0);
    Sequencer::DeferOp R;

    switch (err) {
      case LUA_OK: {
        if (lua_isnil(L, -1)) {
          return Sequencer::UninstallHandler;
        }

        if (!lua_isboolean(L, -1)) {
          qcore_logf(
              QCORE_ERROR,
              "sys_defer: expected boolean return value or nil, got %s\n",
              luaL_typename(L, -1));
          return Sequencer::EmitToken;
        }

        R = lua_toboolean(L, -1) ? Sequencer::EmitToken : Sequencer::SkipToken;
        break;
      }
      case LUA_ERRRUN: {
        qcore_logf(QCORE_ERROR, "sys_defer: lua: %s\n", lua_tostring(L, -1));
        R = Sequencer::EmitToken;
        break;
      }
      case LUA_ERRMEM: {
        qcore_logf(QCORE_ERROR, "sys_defer: memory allocation error\n");
        R = Sequencer::EmitToken;
        break;
      }
      case LUA_ERRERR: {
        qcore_logf(QCORE_ERROR, "sys_defer: error in error handler\n");
        R = Sequencer::EmitToken;
        break;
      }
      default: {
        qcore_logf(QCORE_ERROR, "sys_defer: unexpected error %d\n", err);
        R = Sequencer::EmitToken;
        break;
      }
    }

    lua_pop(L, 1);

    return R;
  };

  get_engine()->m_core->defer_callbacks.push_back(cb);

  return 0;
}
