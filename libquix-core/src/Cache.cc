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

#include <execinfo.h>
#include <quix-core/Cache.h>
#include <quix-core/Error.h>

#include <atomic>
#include <cstdarg>
#include <iostream>
#include <mutex>
#include <thread>

#include "LibMacro.h"

#define PROJECT_REPO_URL "https://github.com/Kracken256/quix"

static struct {
  std::mutex m_lock;
  qcore_cache_has_t m_has;
  qcore_cache_read_t m_read;
  qcore_cache_write_t m_write;
} g_cache_provider{};

LIB_EXPORT bool qcore_cache_bind(qcore_cache_has_t has, qcore_cache_read_t read,
                                 qcore_cache_write_t write) {
  if (!has || !read || !write) {
    return false;
  }

  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  if (g_cache_provider.m_has || g_cache_provider.m_read || g_cache_provider.m_write) {
    return false;
  }

  g_cache_provider.m_has = has;
  g_cache_provider.m_read = read;
  g_cache_provider.m_write = write;

  return true;
}

LIB_EXPORT void qcore_cache_unbind() {
  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  g_cache_provider.m_has = nullptr;
  g_cache_provider.m_read = nullptr;
  g_cache_provider.m_write = nullptr;
}

LIB_EXPORT ssize_t qcore_cache_has(const qcore_cache_key_t *key) {
  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  qcore_assert(key, "qcore_cache_has: key is null");
  qcore_assert(g_cache_provider.m_has, "qcore_cache_has: cache provider not bound");

  return g_cache_provider.m_has(key);
}

LIB_EXPORT bool qcore_cache_read(const qcore_cache_key_t *key, void *data, size_t datalen) {
  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  qcore_assert(key && data, "qcore_cache_read: key or data is null");
  qcore_assert(g_cache_provider.m_read, "qcore_cache_read: cache provider not bound");

  return g_cache_provider.m_read(key, data, datalen);
}

LIB_EXPORT bool qcore_cache_write(const qcore_cache_key_t *key, const void *data, size_t datalen) {
  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  qcore_assert(key && data, "qcore_cache_write: key or data is null");
  qcore_assert(g_cache_provider.m_write, "qcore_cache_write: cache provider not bound");

  return g_cache_provider.m_write(key, data, datalen);
}
