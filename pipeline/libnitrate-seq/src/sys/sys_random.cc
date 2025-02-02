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

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc::seq;

auto Sequencer::SysRandom() -> int {
  auto *lua = m_shared->m_L;

  int64_t min;
  int64_t max;

  auto nargs = lua_gettop(lua);
  if (nargs == 0) {
    min = 0;
    max = 0xff;
  } else if (nargs == 1) {
    min = 0;
    if (lua_isnumber(lua, 1) != 0) {
      max = lua_tointeger(lua, 1);
    } else {
      return luaL_error(lua, "Invalid argument #1: expected number, got %s", lua_typename(lua, lua_type(lua, 1)));
    }
  } else if (nargs == 2) {
    if (lua_isnumber(lua, 1) != 0) {
      min = lua_tointeger(lua, 1);
    } else {
      return luaL_error(lua, "Invalid argument #1: expected number, got %s", lua_typename(lua, lua_type(lua, 1)));
    }

    if (lua_isnumber(lua, 2) != 0) {
      max = lua_tointeger(lua, 2);
    } else {
      return luaL_error(lua, "Invalid argument #2: expected number, got %s", lua_typename(lua, lua_type(lua, 2)));
    }
  } else {
    return luaL_error(lua, "Expected at most two arguments, got %d", nargs);
  }

  if (min > max) {
    return luaL_error(lua, "Invalid range: min > max");
  }

  static_assert(sizeof(m_shared->m_random()) == 8);

  uint64_t num = m_shared->m_random();
  num = (num % (max - min + 1)) + min;

  lua_pushinteger(lua, num);

  return 1;
}
