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
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/Testing.hh>
#include <string>
#include <string_view>

namespace ncc {
  class NCC_EXPORT __attribute__((packed)) String {
    static std::string DefaultEmptyString;
    friend struct CoreLibrarySetup;

    const std::string *m_p;

    static void ResetInstances();
    [[nodiscard]] [[gnu::pure]] static auto CreateInstance(std::string_view str) -> const std::string &;
    [[nodiscard]] [[gnu::pure]] static auto CreateInstance(std::string &&str) -> const std::string &;

  public:
    constexpr explicit String() : m_p(&DefaultEmptyString) {}
    constexpr String(std::string_view str) : m_p(&CreateInstance(str)) {}
    constexpr String(std::string &&str) : m_p(&CreateInstance(std::move(str))) {}
    constexpr String(const std::string &str) : m_p(&CreateInstance(str)) {}
    constexpr String(const char *str) : String(std::string_view(str)) {}

    [[nodiscard]] [[gnu::pure]] constexpr auto Get() const -> const std::string & { return *m_p; };
    [[nodiscard]] [[gnu::pure]] constexpr auto operator==(const String &o) const -> bool { return Get() == o.Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr auto operator!=(const String &o) const -> bool { return Get() != o.Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr auto operator<(const String &o) const -> bool { return Get() < o.Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr auto operator<=(const String &o) const -> bool { return Get() <= o.Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr auto operator>(const String &o) const -> bool { return Get() > o.Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr auto operator>=(const String &o) const -> bool { return Get() >= o.Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr auto operator*() const { return Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr auto operator->() const { return &Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr operator const std::string &() const { return Get(); }
    [[nodiscard]] [[gnu::pure]] constexpr operator std::string_view() const { return Get(); }
  };

  static_assert(sizeof(String) == sizeof(uintptr_t));

  using string = String;

  static inline auto operator<<(std::ostream &os, const String &str) -> std::ostream & { return os << str.Get(); }
}  // namespace ncc

namespace std {
  template <>
  struct hash<ncc::String> {
    auto operator()(const ncc::String &str) const -> size_t { return std::hash<std::string_view>{}(str.Get()); }
  };
}  // namespace std

#endif
