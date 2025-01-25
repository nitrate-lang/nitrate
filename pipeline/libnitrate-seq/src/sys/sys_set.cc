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

static const std::vector<std::string_view> IMMUTABLE_NAMESPACES = {"this."};

auto Sequencer::SysSet() -> int {
  auto *lua = m_shared->m_L;

  auto nargs = lua_gettop(lua);
  if (nargs != 2) {
    return luaL_error(lua, "expected 2 arguments, got %d", nargs);
  }

  if (lua_isstring(lua, 1) == 0) {
    return luaL_error(lua, "expected string, got %s",
                      lua_typename(lua, lua_type(lua, 1)));
  }

  std::string_view key = lua_tostring(lua, 1);

  bool illegal_set =
      std::any_of(IMMUTABLE_NAMESPACES.begin(), IMMUTABLE_NAMESPACES.end(),
                  [&key](auto ns) { return key.starts_with(ns); });

  if (illegal_set) {
    return luaL_error(lua, "cannot set items in immutable namespace");
  }

  if (lua_isnil(lua, 2)) {
    m_env->Set(key, std::nullopt);
  } else {
    m_env->Set(key, lua_tostring(lua, 2));
  }

  lua_pushvalue(lua, 2);

  return 1;
}
