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

#include <qcall/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

#include <quix-core/Env.h>

#include <atomic>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <list>
#include <sstream>
#include <string>

static std::atomic<uint64_t> sys_fetch_request_count = 0;

int qcall::sys_fetch(lua_State* L) {
  /**
   * @brief Download a file.
   */

  int nargs = lua_gettop(L);
  if (nargs < 1) {
    return luaL_error(L, "expected at least 1 argument, got %d", nargs);
  }

  if (!lua_isstring(L, 1)) {
    return luaL_error(L, "expected string, got %s", lua_typename(L, lua_type(L, 1)));
  }

  bool bypass_cache = false;
  if (nargs > 1) {
    if (!lua_isboolean(L, 2)) {
      return luaL_error(L, "expected boolean, got %s", lua_typename(L, lua_type(L, 2)));
    }

    bypass_cache = lua_toboolean(L, 2);
  }
  (void)bypass_cache;

  const char* unsafe_uri = lua_tostring(L, 1);

  try {
    std::stringstream result;
    curlpp::Easy request;

    request.setOpt(new curlpp::options::Url(unsafe_uri));
    request.setOpt(new curlpp::options::FollowLocation(true));
    request.setOpt(new curlpp::options::WriteStream(&result));

    std::list<std::string> header = {
        "User-Agent: QUIX Compiler Suite",
        "QUIX-Request-Id: " + std::to_string(++sys_fetch_request_count),
        "QUIX-Session-Id: " + std::string(qcore_env_get("this.job")),
    };
    request.setOpt(new curlpp::options::HttpHeader(header));

    request.perform();

    lua_pushstring(L, result.str().c_str());

    return 1;
  } catch (curlpp::RuntimeError& e) {
    return luaL_error(L, "curlpp::RuntimeError: %s", e.what());
  } catch (curlpp::LogicError& e) {
    return luaL_error(L, "curlpp::LogicError: %s", e.what());
  }
}
