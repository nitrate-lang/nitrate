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

#include <nitrate-core/Env.h>

#include <qcall/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

static const std::vector<std::string_view> immutable_namespaces = {"this."};

int qcall::sys_set(lua_State* L) {
  /**
   * @brief Set named value to the environment.
   */

  int nargs = lua_gettop(L);
  if (nargs != 2) {
    return luaL_error(L, "expected 2 arguments, got %d", nargs);
  }

  if (!lua_isstring(L, 1)) {
    return luaL_error(L, "expected string, got %s",
                      lua_typename(L, lua_type(L, 1)));
  }

  std::string_view key = lua_tostring(L, 1);

  if (key.empty()) {
    return luaL_error(L, "expected non-empty string, got empty string");
  }

  for (const auto& ns : immutable_namespaces) {
    if (key.starts_with(ns)) {
      return luaL_error(L, "cannot set items in immutable namespace");
    }
  }

  if (lua_isnil(L, 2)) {
    qcore_env_set(key.data(), nullptr);
  } else if (lua_isstring(L, 2)) {
    qcore_env_set(key.data(), lua_tostring(L, 2));
  } else {
    return luaL_error(L, "expected string or nil, got %s",
                      lua_typename(L, lua_type(L, 2)));
  }

  return 0;
}
