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

#ifndef __NITRATE_CORE_CATCH_ALL_H__
#define __NITRATE_CORE_CATCH_ALL_H__

#include <any>
#include <functional>
#include <optional>

namespace ncc::detail {
  [[nodiscard]] std::optional<std::any> CatchAllAny(const std::function<std::any()>& expr);
  [[nodiscard]] bool CatchAllVoid(const std::function<void()>& expr);

  template <typename T, typename = std::enable_if_t<!std::is_same_v<T, void>>>
  static inline std::optional<T> CatchAll(const std::function<std::any()>& expr) {
    auto result = CatchAllAny(expr);
    return result.has_value() ? std::make_optional(std::any_cast<T>(*result)) : std::nullopt;
  }

  template <typename T, typename = std::enable_if_t<std::is_same_v<T, void>>>
  static inline bool CatchAll(const std::function<void()>& expr) {
    return CatchAllVoid(expr);
  };
}  // namespace ncc::detail

/**
 * @brief An ingenious macro to call into exception-prone code and catch all
 * possible exceptions. If an exception is thrown, the return value will be
 * std::nullopt. Otherwise, the return value will be std::optional<std::any>.
 *
 * @note This function may be called from code compiled with the -fno-exceptions
 */
#define OMNI_CATCH(...) ncc::detail::CatchAll<decltype(__VA_ARGS__)>([&]() { return __VA_ARGS__; })

#endif  // __NITRATE_CORE_CATCH_ALL_H__
