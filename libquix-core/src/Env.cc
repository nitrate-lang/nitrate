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

#include <quix-core/Env.h>
#include <quix-core/Error.h>

#include <mutex>
#include <string>
#include <unordered_map>

#include "LibMacro.h"

static std::unordered_map<qcore_env_t, std::unordered_map<std::string, std::string>> g_envs = {
    {0, {}},
};
static std::mutex g_envs_mutex;
static thread_local qcore_env_t g_current_env = 0;

LIB_EXPORT qcore_env_t qcore_env_create(qcore_env_t env) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  if (!g_envs.count(env)) {
    g_envs[env] = {};
  }

  return env;
}

LIB_EXPORT void qcore_env_forget(qcore_env_t env) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);
  
  qcore_assert(g_envs.count(env), "Environment does not exist.");
  g_envs.erase(env);
}

LIB_EXPORT qcore_env_t qcore_env_current() { return g_current_env; }

LIB_EXPORT void qcore_env_set_current(qcore_env_t env) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(env), "Environment does not exist.");
  g_current_env = env;
}

LIB_EXPORT void qcore_env_set(const char *key, const char *value) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env), "Current environment does not exist.");

  if (value == NULL) {
    g_envs[g_current_env].erase(key);
  } else {
    g_envs[g_current_env][key] = value;
  }
}

LIB_EXPORT const char *qcore_env_get(const char *key) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env), "Current environment does not exist.");

  if (g_envs[g_current_env].count(key)) {
    return g_envs[g_current_env][key].c_str();
  } else {
    return NULL;
  }
}
