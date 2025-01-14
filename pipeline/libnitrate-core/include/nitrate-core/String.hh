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

namespace ncc {
  class auto_intern;

  class StringMemory {
    friend class auto_intern;

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
    static void Reset();
  };

  class __attribute__((packed)) auto_intern {
    uint64_t m_id : 40;

  public:
    constexpr auto_intern(std::string_view str = "") {
      m_id = str.empty() ? 0 : StringMemory::FromString(str);
    }

    constexpr auto_intern(std::string &&str) {
      m_id = str.empty() ? 0 : StringMemory::FromString(std::move(str));
    }

    constexpr auto_intern(const std::string &str) {
      m_id = str.empty() ? 0 : StringMemory::FromString(str);
    }

    constexpr auto_intern(const char *str) {
      if (str[0] == '\0') {
        m_id = 0;
      } else {
        m_id = StringMemory::FromString(std::string_view(str));
      }
    }

    std::string_view get() const;

    constexpr bool operator==(const auto_intern &O) const {
      return m_id == O.m_id;
    }

    constexpr auto operator*() const { return get(); }

    const auto *operator->() const {
      static thread_local std::string_view sv;
      sv = get();
      return &sv;
    }

    constexpr bool operator<(const auto_intern &O) const {
      return m_id < O.m_id;
    }

    constexpr operator std::string_view() const { return get(); }

    constexpr auto getId() const { return m_id; }
  };

  using string = auto_intern;

  static inline std::ostream &operator<<(std::ostream &os,
                                         const auto_intern &str) {
    return os << str.get();
  }
}  // namespace ncc

namespace std {
  template <>
  struct hash<ncc::auto_intern> {
    size_t operator()(const ncc::auto_intern &str) const {
      return std::hash<uint64_t>{}(str.getId());
    }
  };
}  // namespace std

#endif
