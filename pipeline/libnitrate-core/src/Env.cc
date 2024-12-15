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
#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <threads.h>

#include <cstdio>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

struct Environment {
  std::unordered_map<std::string, std::string> env;
  std::stringstream log_buffer;
  qcore_log_t log_level;
};

static std::unordered_map<qcore_env_t, Environment> g_envs;
static std::mutex g_envs_mutex;
static thread_local qcore_env_t g_current_env = 0;
extern "C" {
__attribute__((visibility("default"))) bool qcore_fuzzing = false;
}

static void qcore_default_logger(qcore_log_t, const char *, size_t, void *) {}

static thread_local qcore_logger_t g_current_logger = qcore_default_logger;
static thread_local void *g_current_logger_data = nullptr;

C_EXPORT qcore_env_t qcore_env_create(qcore_env_t env) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  if (!g_envs.count(env)) {
    g_envs[env] = {};
  }

  return env;
}

C_EXPORT void qcore_env_destroy(qcore_env_t env) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(env), "Environment does not exist.");
  g_envs.erase(env);
}

C_EXPORT qcore_env_t qcore_env_current() { return g_current_env; }

C_EXPORT void qcore_env_set_current(qcore_env_t env) {
  if (env == 0) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(env), "Environment does not exist.");
  g_current_env = env;
}

C_EXPORT void qcore_env_set(const char *key, const char *value) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env),
               "Current environment does not exist.");

  if (value == NULL) {
    g_envs[g_current_env].env.erase(key);
  } else {
    g_envs[g_current_env].env[key] = value;
  }
}

C_EXPORT const char *qcore_env_get(const char *key) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env),
               "Current environment does not exist.");

  if (g_envs[g_current_env].env.count(key)) {
    return g_envs[g_current_env].env[key].c_str();
  } else {
    return NULL;
  }
}

C_EXPORT void qcore_bind_logger(qcore_logger_t logger, void *data) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  g_current_logger = logger ? logger : qcore_default_logger;
  g_current_logger_data = data;
}

C_EXPORT void qcore_begin(qcore_log_t level) {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env),
               "Current environment does not exist.");

  g_envs[g_current_env].log_buffer.str("");
  g_envs[g_current_env].log_level = level;
}

C_EXPORT void qcore_end() {
  std::lock_guard<std::mutex> lock(g_envs_mutex);

  qcore_assert(g_envs.count(g_current_env),
               "Current environment does not exist.");

  if (qcore_fuzzing) {
    return;
  }

  let current = g_envs.at(g_current_env);
  std::string message = current.log_buffer.str();

  while (message.ends_with("\n")) {
    message.pop_back();
  }

  static const std::unordered_map<qcore_log_t, const char *> level_names = {
      {QCORE_DEBUG, "DEBUG"}, {QCORE_INFO, "INFO"},   {QCORE_WARN, "WARN"},
      {QCORE_ERROR, "ERROR"}, {QCORE_FATAL, "FATAL"},
  };

  std::stringstream log_message;

  switch (current.log_level) {
    case QCORE_DEBUG: {
      log_message << "\x1b[1mdebug:\x1b[0m " << message << "\x1b[0m";
      break;
    }

    case QCORE_INFO: {
      log_message << "\x1b[37;1minfo:\x1b[0m " << message << "\x1b[0m";
      break;
    }

    case QCORE_WARN: {
      log_message << "\x1b[35;1mwarning:\x1b[0m " << message << "\x1b[0m";
      break;
    }

    case QCORE_ERROR: {
      log_message << "\x1b[31;1merror:\x1b[0m " << message << "\x1b[0m";
      break;
    }

    case QCORE_FATAL: {
      log_message << "\x1b[31;1;4mfatal error:\x1b[0m " << message << "\x1b[0m";
      break;
    }
  }

  message = log_message.str();
  g_current_logger(current.log_level, message.c_str(), message.size(),
                   g_current_logger_data);
}

C_EXPORT int qcore_vwritef(const char *fmt, va_list args) {
  char *buffer = NULL;
  int size = vasprintf(&buffer, fmt, args);
  if (size < 0) {
    qcore_panic("Failed to allocate memory for log message.");
  }

  {
    std::lock_guard<std::mutex> lock(g_envs_mutex);
    qcore_assert(g_envs.count(g_current_env),
                 "Current environment does not exist.");

    g_envs[g_current_env].log_buffer << std::string_view(buffer, size);
  }

  free(buffer);

  return size;
}