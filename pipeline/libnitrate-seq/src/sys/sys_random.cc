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
#include <cstdio>
#include <nitrate-core/Environment.hh>
#include <nitrate-seq/Sequencer.hh>

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc::seq;

auto Sequencer::PImpl::SysRandom() -> int {
  int64_t min;
  int64_t max;

  auto nargs = lua_gettop(m_L);
  if (nargs == 0) {
    min = 0;
    max = 0xff;
  } else if (nargs == 1) {
    min = 0;
    if (lua_isnumber(m_L, 1) != 0) {
      max = lua_tointeger(m_L, 1);
    } else {
      return luaL_error(m_L, "Invalid argument #1: expected number, got %s",
                        lua_typename(m_L, lua_type(m_L, 1)));
    }
  } else if (nargs == 2) {
    if (lua_isnumber(m_L, 1) != 0) {
      min = lua_tointeger(m_L, 1);
    } else {
      return luaL_error(m_L, "Invalid argument #1: expected number, got %s",
                        lua_typename(m_L, lua_type(m_L, 1)));
    }

    if (lua_isnumber(m_L, 2) != 0) {
      max = lua_tointeger(m_L, 2);
    } else {
      return luaL_error(m_L, "Invalid argument #2: expected number, got %s",
                        lua_typename(m_L, lua_type(m_L, 2)));
    }
  } else {
    return luaL_error(m_L, "Expected at most two arguments, got %d", nargs);
  }

  if (min > max) {
    return luaL_error(m_L, "Invalid range: min > max");
  }

  static_assert(sizeof(m_random()) == 8);

  uint64_t num = m_random();
  num = (num % (max - min + 1)) + min;

  lua_pushinteger(m_L, num);

  return 1;
}
