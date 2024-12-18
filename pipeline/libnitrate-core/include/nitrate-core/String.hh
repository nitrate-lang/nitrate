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

#include <boost/bimap.hpp>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>

namespace ncc::core {
  class StringMemory;

  class __attribute__((packed)) str_alias {
    friend class StringMemory;

    uint64_t m_id : 40;

    static constexpr str_alias get(uint64_t id) {
      str_alias alias;
      alias.m_id = id;
      return alias;
    }

  public:
    std::string_view get() const;

    constexpr bool operator==(const str_alias &other) const {
      return m_id == other.m_id;
    }

    std::string_view operator*() const { return get(); }

    inline const std::string_view *operator->() const {
      static thread_local std::string_view sv;
      sv = get();
      return &sv;
    }

    bool operator<(const str_alias &other) const { return m_id < other.m_id; }
  };

  class StringMemory {
    struct Storage {
      std::unordered_set<std::string> m_strings;
      boost::bimap<uint64_t, std::string_view> m_bimap;
      uint64_t m_next_id = 0;
      std::mutex m_mutex;
    };

    static Storage m_storage;

    StringMemory() = delete;

  public:
    static str_alias get(std::string_view str);
    static std::string_view save(std::string_view str);

    static std::string_view from_id(uint64_t id);
  };

  static inline std::string_view save(std::string_view str) {
    return StringMemory::save(str);
  }

  static inline str_alias intern(std::string_view str) {
    return StringMemory::get(str);
  }

}  // namespace ncc::core

#endif
