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

#include <cstdio>
#include <nitrate-core/Environment.hh>
#include <nitrate-seq/Sequencer.hh>
#include <random>
#include <sys/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

int ncc::seq::sys_random(lua_State* L) {
  int64_t min, max;

  int nargs = lua_gettop(L);
  if (nargs == 0) {
    min = 0;
    max = 0xff;
  } else if (nargs == 1) {
    min = 0;
    if (lua_isnumber(L, 1)) {
      max = lua_tointeger(L, 1);
    } else {
      return luaL_error(L, "Invalid argument #1: expected number, got %s",
                        lua_typename(L, lua_type(L, 1)));
    }
  } else if (nargs == 2) {
    if (lua_isnumber(L, 1)) {
      min = lua_tointeger(L, 1);
    } else {
      return luaL_error(L, "Invalid argument #1: expected number, got %s",
                        lua_typename(L, lua_type(L, 1)));
    }

    if (lua_isnumber(L, 2)) {
      max = lua_tointeger(L, 2);
    } else {
      return luaL_error(L, "Invalid argument #2: expected number, got %s",
                        lua_typename(L, lua_type(L, 2)));
    }
  } else {
    return luaL_error(L, "Expected at most two arguments, got %d", nargs);
  }

  if (min > max) {
    return luaL_error(L, "Invalid range: min > max");
  }

  auto engine = get_engine();

  static_assert(sizeof(engine->m_core->m_random()) == 8);

  uint64_t num = engine->m_core->m_random();
  num = (num % (max - min + 1)) + min;

  lua_pushinteger(L, num);

  return 1;
}
