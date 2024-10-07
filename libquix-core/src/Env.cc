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
#include <threads.h>

#include <cstdio>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#include "LibMacro.h"

struct Environment {
  std::unordered_map<std::string, std::string> env;
  std::stringstream log_buffer;
  qcore_log_t log_level;
};

static std::unordered_map<qcore_env_t, Environment> g_envs;
static std::mutex g_envs_mutex;
static thread_local qcore_env_t g_current_env = 0;
extern "C" {
__attribute__((visibility("default"))) bool qcore_fuzzing = true;
}

LIB_EXPORT qcore_env_t qcore_env_create(qcore_env_t env) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  if (!g_envs.count(env)) {
    g_envs[env] = {};
  }

  return env;
}

LIB_EXPORT void qcore_env_destroy(qcore_env_t env) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(env), "Environment does not exist.");
  g_envs.erase(env);
}

LIB_EXPORT qcore_env_t qcore_env_current() { return g_current_env; }

LIB_EXPORT void qcore_env_set_current(qcore_env_t env) {
  if (env == 0) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(env), "Environment does not exist.");
  g_current_env = env;
}

LIB_EXPORT void qcore_env_set(const char *key, const char *value) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env), "Current environment does not exist.");

  if (value == NULL) {
    g_envs[g_current_env].env.erase(key);
  } else {
    g_envs[g_current_env].env[key] = value;
  }
}

LIB_EXPORT const char *qcore_env_get(const char *key) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env), "Current environment does not exist.");

  if (g_envs[g_current_env].env.count(key)) {
    return g_envs[g_current_env].env[key].c_str();
  } else {
    return NULL;
  }
}

LIB_EXPORT void qcore_begin(qcore_log_t level) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env), "Current environment does not exist.");

  g_envs[g_current_env].log_buffer.str("");
  g_envs[g_current_env].log_level = level;
}

#include <iostream>

LIB_EXPORT void qcore_end() {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env), "Current environment does not exist.");

  if (!qcore_fuzzing) {
    return;
  }

  if (g_envs[g_current_env].env.contains("this.noprint")) {
    return;
  }

  std::string message = g_envs[g_current_env].log_buffer.str();
  std::ostream &ss = std::cerr;

  switch (g_envs[g_current_env].log_level) {
    case QCORE_DEBUG:
      if (g_envs[g_current_env].env.contains("log_enable_debug")) {
        ss << "[DEBUG] " << message << std::endl;
      }
      break;

    case QCORE_INFO:
      ss << "[INFO] " << message << std::endl;
      break;

    case QCORE_WARN:
      ss << "[WARN] " << message << std::endl;
      break;

    case QCORE_ERROR:
      ss << "[ERROR] " << message << std::endl;
      break;

    case QCORE_FATAL:
      ss << "[FATAL] " << message << std::endl;
      break;
  }
}

LIB_EXPORT int qcore_vwritef(const char *fmt, va_list args) {
  char *buffer = NULL;
  int size = vasprintf(&buffer, fmt, args);
  if (size < 0) {
    qcore_panic("Failed to allocate memory for log message.");
  }

  {
    std::lock_guard<std::mutex> lock(g_envs_mutex);
    qcore_assert(g_envs.count(g_current_env), "Current environment does not exist.");

    g_envs[g_current_env].log_buffer << std::string_view(buffer, size);
  }

  free(buffer);

  return size;
}