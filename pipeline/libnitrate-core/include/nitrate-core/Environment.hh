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

#ifndef __NITRATE_CORE_ENV_H__
#define __NITRATE_CORE_ENV_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <mutex>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Logger.hh>
#include <random>
#include <unordered_map>

typedef uintptr_t qcore_env_t;

/**
 * @brief Create a new environment.
 * @return The environment handle otherwise a NOP if the environment already
 * exists.
 */
qcore_env_t qcore_env_create(qcore_env_t env);

/**
 * @brief Drop a reference to an environment.
 * @param env The environment handle.
 * @note The environment will be destroyed when the last reference is dropped.
 */
void qcore_env_destroy(qcore_env_t env);

/**
 * @brief Get the current environment.
 * @return The current environment handle.
 */
qcore_env_t qcore_env_current();

/**
 * @brief Set the current environment.
 * @param env The environment handle.
 */
void qcore_env_set_current(qcore_env_t env);

/**
 * @brief Get the value of an environment variable.
 * @param key The environment variable key.
 * @return The value of the environment variable, or NULL to unset the variable.
 * @note Strings will be cloned internally.
 */
void qcore_env_set(const char *key, const char *value);

#define qcore_env_unset(key) qcore_env_set(key, NULL)

/**
 * @brief Get the value of an environment variable.
 * @param key The environment variable key.
 * @return The value of the environment variable, or NULL if the variable is not
 * set.
 * @note Strings will be cloned internally.
 */
const char *qcore_env_get(const char *key);

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

namespace ncc::core {
  class IEnvironment {
  public:
    virtual ~IEnvironment() = default;

    virtual bool contains(const std::string &key) = 0;
    virtual std::optional<std::string> get(const std::string &key) = 0;
    virtual void set(std::string key, std::optional<std::string> value,
                     bool privset = false) = 0;
  };

  class MockEnvironment final : public IEnvironment {
    std::unordered_map<std::string, std::string> m_data;
    std::mutex m_mutex;

  public:
    MockEnvironment() = default;
    virtual ~MockEnvironment() = default;

    bool contains(const std::string &key) override {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_data.contains(key);
    }

    std::optional<std::string> get(const std::string &key) override {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (auto it = m_data.find(key); it != m_data.end()) {
        return it->second;
      }
      return std::nullopt;
    }

    void set(std::string key, std::optional<std::string> val,
             bool = false) override {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (val.has_value()) {
        m_data[std::move(key)] = std::move(val.value());
      } else {
        m_data.erase(key);
      }
    }
  };

  class Environment {
    std::unordered_map<std::string, std::string> m_data;
    std::mutex m_mutex;

  public:
    Environment() = default;
    virtual ~Environment() = default;

    bool contains(const std::string &key) {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_data.contains(key);
    }

    std::optional<std::string> get(const std::string &key) {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (auto it = m_data.find(key); it != m_data.end()) {
        return it->second;
      }
      return std::nullopt;
    }

    void set(std::string key, std::optional<std::string> value, bool = false) {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (value.has_value()) {
        m_data[std::move(key)] = std::move(value.value());
      } else {
        m_data.erase(key);
      }
    }
  };

}  // namespace ncc::core

#endif  // __NITRATE_CORE_ENV_H__
