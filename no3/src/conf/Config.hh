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

#ifndef __NO3_CONF_CONFIG_HH__
#define __NO3_CONF_CONFIG_HH__

#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace no3::conf {
  enum class ConfigItemSerializationTarget : uint8_t {
    JSON,
    YAML,
  };

  class ConfigSerializerException : public std::runtime_error {
  public:
    ConfigSerializerException(const std::string &message) : std::runtime_error(message) {}
  };

  struct ConfigValue {
    std::variant<std::vector<std::string>, std::string, int64_t, bool> m_value;

    ConfigValue() = default;
    ConfigValue(std::variant<std::vector<std::string>, std::string, int64_t, bool> value) : m_value(std::move(value)) {}

    template <typename T>
    [[nodiscard]] [[nodiscard]] auto As() const -> T {
      return std::get<T>(m_value);
    }

    template <typename T>
    [[nodiscard]] auto Is() const -> bool {
      return std::holds_alternative<T>(m_value);
    }

    auto operator==(const ConfigValue &value) const -> bool { return m_value == value.m_value; }
  };

  struct ConfigGroup {
    std::unordered_map<std::string, ConfigValue> m_items;

    ConfigGroup() = default;

    /**
     * @brief Dump the configuration tree to a string
     *
     * @param target Serialization target
     * @return std::string Serialized configuration tree
     * @note If the target is not supported, the function throws
     * ConfigSerializerException.
     */
    auto Dump(ConfigItemSerializationTarget target = ConfigItemSerializationTarget::JSON) const -> std::string;

    auto operator[](const std::string &key) const -> ConfigValue { return m_items.at(key); }

    template <typename T>
    void Set(const std::string &key, T value) {
      m_items[key] = ConfigValue(value);
    }

    template <typename T>
    auto Has(const std::string &key) const -> bool {
      return m_items.contains(key) && m_items.at(key).Is<T>();
    }

    template <typename T>
    auto Is(const std::string &key, T value) const -> bool {
      return has<T>(key) && m_items.at(key).As<T>() == value;
    }

    auto operator==(const ConfigGroup &grp) const -> bool { return m_items == grp.m_items; }
  };

  struct Config {
    ConfigGroup m_root;
    int m_version;

    Config() : m_version(0) {}
    Config(ConfigGroup grp, int version = 0) : m_root(std::move(grp)), m_version(version) {}

    /**
     * @brief Dump the configuration to a string
     *
     * @param target Serialization target
     * @return std::string Serialized configuration
     * @note If the target is not supported, the function throws
     * ConfigSerializerException.
     */
    auto Dump(ConfigItemSerializationTarget target = ConfigItemSerializationTarget::JSON) const -> std::string;

    auto operator==(const Config &cfg) const -> bool { return m_version == cfg.m_version && m_root == cfg.m_root; }

    auto operator[](const std::string &key) const -> ConfigValue { return m_root[key]; }

    auto Keys() const -> std::set<std::string> {
      std::set<std::string> keys;
      for (const auto &item : m_root.m_items) {
        keys.insert(item.first);
      }
      return keys;
    }
  };
}  // namespace no3::conf

#endif  // __NO3_CONF_CONFIG_HH__
