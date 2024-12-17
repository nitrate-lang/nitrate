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

#ifndef __NITRATE_CORE_CLASSES_H__
#define __NITRATE_CORE_CLASSES_H__

#ifndef __cplusplus
#error "This header is for C++ only."
#endif

#include <nitrate-core/Env.h>
#include <nitrate-core/Memory.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <random>

class qcore_arena final {
  std::optional<qcore_arena_t> m_arena;

  qcore_arena(const qcore_arena &) = delete;

public:
  qcore_arena() {
    m_arena = 0;
    qcore_arena_open(&m_arena.value());
  }

  ~qcore_arena() {
    if (m_arena.has_value()) {
      qcore_arena_close(&m_arena.value());
    }
  }

  qcore_arena(qcore_arena &&o) : m_arena(std::move(o.m_arena)) {}
  qcore_arena &operator=(qcore_arena &&o) {
    m_arena = std::move(o.m_arena);
    return *this;
  }

  qcore_arena_t *get() { return &m_arena.value(); }
};

class qcore_env final {
  qcore_env_t m_env;

public:
  qcore_env() {
    std::random_device rd;
    std::uniform_int_distribution<uintptr_t> gen;
    m_env = qcore_env_create(gen(rd));
    qcore_env_set_current(m_env);

    {  // Set a random job ID
      boost::uuids::random_generator gen;
      boost::uuids::uuid uuid = gen();
      std::string uuid_str = boost::uuids::to_string(uuid);
      qcore_env_set("this.job", uuid_str.c_str());
    }

    {  // Set the compiler start time
      std::chrono::system_clock::time_point now =
          std::chrono::system_clock::now();
      std::chrono::milliseconds ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              now.time_since_epoch());

      qcore_env_set("this.created_at", std::to_string(ms.count()).c_str());
    }
  }
  ~qcore_env() { qcore_env_destroy(m_env); }

  qcore_env_t &get() { return m_env; }
};

#endif  // __NITRATE_CORE_CLASSES_H__
