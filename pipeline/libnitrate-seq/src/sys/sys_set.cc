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
#include <nitrate-core/Environment.hh>
#include <nitrate-seq/Sequencer.hh>

using namespace ncc::seq;

static const std::vector<std::string_view> IMMUTABLE_NAMESPACES = {"this."};

auto Sequencer::PImpl::SysSet() -> int {
  auto nargs = lua_gettop(m_L);
  if (nargs != 2) {
    return luaL_error(m_L, "expected 2 arguments, got %d", nargs);
  }

  if (lua_isstring(m_L, 1) == 0) {
    return luaL_error(m_L, "expected string, got %s",
                      lua_typename(m_L, lua_type(m_L, 1)));
  }

  std::string_view key = lua_tostring(m_L, 1);

  if (key.empty()) {
    return luaL_error(m_L, "expected non-empty string, got empty string");
  }

  for (const auto& ns : IMMUTABLE_NAMESPACES) {
    if (key.starts_with(ns)) {
      return luaL_error(m_L, "cannot set items in immutable namespace");
    }
  }

  if (lua_isnil(m_L, 2)) {
    m_env->Set(key, std::nullopt);
  } else if (lua_isstring(m_L, 2) != 0) {
    m_env->Set(key, lua_tostring(m_L, 2));
  } else {
    return luaL_error(m_L, "expected string or nil, got %s",
                      lua_typename(m_L, lua_type(m_L, 2)));
  }

  return 0;
}
