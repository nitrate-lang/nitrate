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
#include <qcall/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc;

int seq::sys_debug(lua_State* L) {
  /**
   * @brief Put a value into the debug log.
   */

  int nargs = lua_gettop(L);
  if (nargs == 0) {
    return luaL_error(L, "Expected at least one argument, got 0");
  }

  qcore_begin(QCORE_DEBUG);

  for (int i = 1; i <= nargs; i++) {
    if (lua_isstring(L, i)) {
      qcore_write(lua_tostring(L, i));
    } else if (lua_isnumber(L, i)) {
      qcore_writef("%g", (double)lua_tonumber(L, i));
    } else if (lua_isboolean(L, i)) {
      qcore_write(lua_toboolean(L, i) ? "true" : "false");
    } else {
      return luaL_error(
          L,
          "Invalid argument #%d: expected string, number, or boolean, got %s",
          i, lua_typename(L, lua_type(L, i)));
    }
  }

  qcore_end();

  return 0;
}
