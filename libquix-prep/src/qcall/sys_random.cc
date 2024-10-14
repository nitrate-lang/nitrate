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

#include <openssl/rand.h>
#include <quix-core/Env.h>

#include <core/Preprocess.hh>
#include <cstdio>
#include <qcall/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

int qcall::sys_random(lua_State* L) {
  /**
   * @brief Get a cryptographic random number (in the range [a, b]).
   */

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

  union {
    uint8_t buf[8];
    uint64_t num;
  } u;

  if (RAND_bytes(u.buf, sizeof(u.buf)) != 1) {
    return luaL_error(L, "Failed to generate random number");
  }

  u.num = (u.num % (max - min + 1)) + min;

  lua_pushinteger(L, u.num);

  return 1;
}
