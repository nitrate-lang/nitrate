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

#include <algorithm>
#include <core/Sequencer.hh>
#include <nitrate-core/Environment.hh>
#include <nitrate-seq/Sequencer.hh>
#include <optional>
#include <regex>
#include <string>

using namespace ncc::seq;

static auto IsValidImportName(const std::string &name) -> bool {
  if (name.empty()) {
    return false;
  }

  if (std::any_of(name.begin(), name.end(), [](char c) { return c & 0x80; })) {
    return false;
  }

  std::regex re(R"(^[a-zA-Z_][a-zA-Z0-9_]*(::[a-zA-Z_][a-zA-Z0-9_]*)*$)");

  return std::regex_match(name, re);
}

static void CanonicalizeImportName(std::string &name) {
  // Don't assume that filesystems are case-sensitive.
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
}

auto Sequencer::PImpl::SysFetch() -> int {
  auto nargs = lua_gettop(m_L);
  if (nargs != 1) {
    return luaL_error(m_L, "expected 1 argument, got %d", nargs);
  }

  if (lua_isstring(m_L, 1) == 0) {
    return luaL_error(m_L, "expected string, got %s",
                      lua_typename(m_L, lua_type(m_L, 1)));
  }

  std::string import_name = lua_tostring(m_L, 1);
  if (!IsValidImportName(import_name)) {
    return luaL_error(m_L, "invalid import name");
  }

  CanonicalizeImportName(import_name);

  if (auto data = FetchModuleData(import_name)) {
    lua_pushstring(m_L, data->c_str());
    return 1;
  }

  return luaL_error(m_L, "failed to fetch module");
}
