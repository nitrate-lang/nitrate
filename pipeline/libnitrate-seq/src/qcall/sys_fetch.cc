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
#include <nitrate-core/Environment.hh>
#include <nitrate-seq/Preprocess.hh>
#include <optional>
#include <qcall/List.hh>
#include <regex>
#include <string>

extern "C" {
#include <lua/lauxlib.h>
}

static bool is_valid_import_name(const std::string &name) {
  if (name.empty()) {
    return false;
  }

  if (std::any_of(name.begin(), name.end(), [](char c) { return c & 0x80; })) {
    return false;
  }

  std::regex re(R"(^[a-zA-Z_][a-zA-Z0-9_]*(::[a-zA-Z_][a-zA-Z0-9_]*)*$)");
  return std::regex_match(name, re);
}

static void canonicalize_import_name(std::string &name) {
  // Don't assume that filesystems are case-sensitive.
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
}

static std::optional<std::string> fetch_module_data(qprep_impl_t *obj,
                                                    const char *name) {
  // if (!obj->m_fetch_module.first) {
  //   return std::nullopt;
  // }

  // char *module_data = NULL;
  // size_t module_size = 0;

  // // Always put off to tomorrow what can be done today.
  // if (!obj->m_fetch_module.first(obj, name, &module_data, &module_size,
  //                                obj->m_fetch_module.second)) {
  //   return std::nullopt;
  // }

  // std::string data(module_data, module_size);
  // free(module_data);

  // return data;
  (void)obj;
  (void)name;

  qcore_print(QCORE_WARN, "fetch_module_data not implemented");

  return std::nullopt;
}

int qcall::sys_fetch(lua_State *L) {
  /**
   * @brief Download a file.
   */

  qprep_impl_t *obj = get_engine();

  int nargs = lua_gettop(L);
  if (nargs != 1) {
    return luaL_error(L, "expected 1 argument, got %d", nargs);
  }

  if (!lua_isstring(L, 1)) {
    return luaL_error(L, "expected string, got %s",
                      lua_typename(L, lua_type(L, 1)));
  }

  std::string import_name = lua_tostring(L, 1);

  if (!is_valid_import_name(import_name)) {
    return luaL_error(L, "invalid import name");
  }

  canonicalize_import_name(import_name);

  if (auto data = fetch_module_data(obj, import_name.c_str())) {
    lua_pushstring(L, data->c_str());
    return 1;
  } else {
    return luaL_error(L, "failed to fetch module");
  }
}
