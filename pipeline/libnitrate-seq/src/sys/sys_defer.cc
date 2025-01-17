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

#include <core/Sequencer.hh>
#include <nitrate-seq/Sequencer.hh>

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc::seq;
using namespace ncc::lex;

auto Sequencer::PImpl::SysDefer() -> int {
  int nargs = lua_gettop(m_L);
  if (nargs < 1) {
    return luaL_error(m_L, "sys_defer: expected at least 1 argument, got %d",
                      nargs);
  }

  if (!lua_isfunction(m_L, 1)) {
    return luaL_error(m_L,
                      "sys_defer: expected function as first argument, got %s",
                      luaL_typename(m_L, 1));
  }

  int id = luaL_ref(m_L, LUA_REGISTRYINDEX);
  if (id == LUA_REFNIL) {
    return luaL_error(m_L, "sys_defer: failed to store callback in registry");
  }

  DeferCallback cb = [&](Sequencer* engine, Token tok) -> DeferOp {
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, id); /* Get the function */

    { /* Push the function arguments */
      lua_newtable(m_L);

      lua_pushstring(m_L, "ty");
      lua_pushstring(m_L, to_string(tok.GetKind())->c_str());
      lua_settable(m_L, -3);

      lua_pushstring(m_L, "v");
      switch (tok.GetKind()) {
        case EofF:
        case KeyW: {
          lua_pushstring(m_L, kw_repr(tok.GetKeyword()));
          break;
        }
        case Oper: {
          lua_pushstring(m_L, op_repr(tok.GetOperator()));
          break;
        }
        case Punc: {
          lua_pushstring(m_L, punct_repr(tok.GetPunctor()));
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
          lua_pushstring(m_L, std::string(tok.GetString()).c_str());
          break;
        }
      }

      lua_settable(m_L, -3);
    }

    int err = lua_pcall(m_L, 1, 1, 0);
    DeferOp r;

    switch (err) {
      case LUA_OK: {
        if (lua_isnil(m_L, -1)) {
          return UninstallHandler;
        }

        if (!lua_isboolean(m_L, -1)) {
          ncc::Log << Error
                   << "sys_defer: expected boolean return value or nil, got "
                   << luaL_typename(m_L, -1);

          engine->SetFailBit();
          return EmitToken;
        }

        r = (lua_toboolean(m_L, -1) != 0) ? EmitToken : SkipToken;
        break;
      }
      case LUA_ERRRUN: {
        ncc::Log << Error << "sys_defer: lua: " << lua_tostring(m_L, -1);
        engine->SetFailBit();
        r = EmitToken;
        break;
      }
      case LUA_ERRMEM: {
        ncc::Log << Error << "sys_defer: memory allocation error\n";
        engine->SetFailBit();
        r = EmitToken;
        break;
      }
      case LUA_ERRERR: {
        ncc::Log << Error << "sys_defer: error in error handler\n";
        engine->SetFailBit();
        r = EmitToken;
        break;
      }
      default: {
        ncc::Log << Error << "sys_defer: unexpected error " << err << "\n";
        engine->SetFailBit();
        r = EmitToken;
        break;
      }
    }

    lua_pop(m_L, 1);

    return r;
  };

  m_defer.push_back(cb);

  return 0;
}
