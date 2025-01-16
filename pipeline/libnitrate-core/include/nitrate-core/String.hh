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
#include <nitrate-core/Macro.hh>
#include <string>
#include <string_view>

namespace ncc {
  class String;

  class NCC_EXPORT StringMemory {
    friend class String;

    /* assert(!str.empty()) */
    static uint64_t FromString(std::string_view str);

    /* assert(!str.empty()) */
    static uint64_t FromString(std::string &&str);

  public:
    StringMemory() = delete;
    static void Reset();
  };

  class NCC_EXPORT __attribute__((packed)) String {
    uint64_t m_id : 40;

  public:
    constexpr explicit String() : m_id(0) {}

    constexpr NCC_FORCE_INLINE String(std::string_view str)
        : m_id(str.empty() ? 0 : StringMemory::FromString(str)) {}

    constexpr NCC_FORCE_INLINE String(std::string &&str)
        : m_id(str.empty() ? 0 : StringMemory::FromString(std::move(str))) {}

    constexpr NCC_FORCE_INLINE String(const std::string &str)
        : m_id(str.empty() ? 0 : StringMemory::FromString(str)) {}

    constexpr NCC_FORCE_INLINE String(const char *str)
        : m_id(str[0] == 0 ? 0
                           : StringMemory::FromString(std::string_view(str))) {}

    [[nodiscard]] auto Get() const -> std::string_view;

    bool operator==(const String &o) const;
    bool operator<(const String &o) const;

    constexpr auto operator*() const { return Get(); }

    const auto *operator->() const {
      static thread_local std::string_view sv;
      sv = Get();
      return &sv;
    }

    constexpr operator std::string_view() const { return Get(); }

    [[nodiscard]] constexpr auto GetId() const { return m_id; }
  };

  using string = String;

  static inline std::ostream &operator<<(std::ostream &os, const String &str) {
    return os << str.Get();
  }
}  // namespace ncc

namespace std {
  template <>
  struct hash<ncc::String> {
    size_t operator()(const ncc::String &str) const {
      return std::hash<std::string_view>{}(str.Get());
    }
  };
}  // namespace std

#endif
