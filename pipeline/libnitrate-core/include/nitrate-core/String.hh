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

#ifndef __NITRATE_CORE_STRING_FACTORY_H__
#define __NITRATE_CORE_STRING_FACTORY_H__

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ncc::core {
  class str_alias;

  class StringMemory {
    friend class str_alias;

    struct Storage {
      std::unordered_map<uint64_t, std::string> m_map_a;
      std::unordered_map<std::string_view, uint64_t> m_map_b;
      uint64_t m_next_id = 0;
      std::mutex m_mutex;

      Storage() {
        m_map_a[0] = "";
        m_map_b[""] = 0;
        m_next_id = 1;
      }
    };

    static Storage m_storage;

    StringMemory() = delete;

    static std::string_view FromID(uint64_t id);
    static uint64_t FromString(std::string_view str);
    static uint64_t FromString(std::string &&str);

  public:
    static void Clear();
  };

  class __attribute__((packed)) str_alias {
    uint64_t m_id : 40;

  public:
    constexpr str_alias(std::string_view str = "") {
      m_id = str.empty() ? 0 : StringMemory::FromString(str);
    }
    constexpr str_alias(std::string &&str) {
      m_id = str.empty() ? 0 : StringMemory::FromString(std::move(str));
    }
    constexpr str_alias(const char *str) {
      if (str[0] == '\0') {
        m_id = 0;
      } else {
        m_id = StringMemory::FromString(std::string_view(str));
      }
    }

    std::string_view get() const;

    constexpr bool operator==(const str_alias &O) const {
      return m_id == O.m_id;
    }

    constexpr inline auto operator*() const { return get(); }
    inline const auto *operator->() const {
      static thread_local std::string_view sv;
      sv = get();
      return &sv;
    }

    constexpr inline bool operator<(const str_alias &O) const {
      return m_id < O.m_id;
    }
  };

  static inline std::string_view save(std::string_view str) {
    return str_alias(str).get();
  }

  static inline std::string_view save(std::string &&str) {
    return str_alias(str).get();
  }

  static inline std::string_view save(const char *str) {
    return str_alias(str).get();
  }

  static inline str_alias intern(std::string_view str) {
    return str_alias(str);
  }

  static inline str_alias intern(std::string &&str) { return str_alias(str); }

  static inline str_alias intern(const char *str) { return str_alias(str); }
}  // namespace ncc::core

#endif
