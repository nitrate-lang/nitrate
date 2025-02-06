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

#ifndef __NITRATE_CORE_ASSERT_H__
#define __NITRATE_CORE_ASSERT_H__

#include <array>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

void QCorePanic(const char *msg) __attribute__((noreturn));
void QCorePanicF(const char *fmt, ...) __attribute__((noreturn));

static inline auto GetStrerror() {
  constexpr size_t kMaxMessageSize = 256;

  std::array<char, kMaxMessageSize> buf;
  strerror_r(errno, buf.data(), buf.size());
  return std::string(buf.data());
}

#if defined(NDEBUG)
#define qcore_panicf(fmt, ...)                                                                                   \
  QCorePanicF(fmt "\nSource File: %s\nSource Line: %d\nFunction: unknown\nErrno: %s\n", ##__VA_ARGS__, __FILE__, \
              __LINE__, GetStrerror().c_str())

#define qcore_panic(msg) qcore_panicf("%s", msg)

#define qcore_assert(expr, ...)      \
  (static_cast<bool>(expr) ? void(0) \
                           : qcore_panicf("Assertion failed: %s;\nCondition: (%s);\n", "" #__VA_ARGS__, #expr))
#else
#define qcore_panicf(fmt, ...)                                                                                        \
  QCorePanicF(fmt "\nSource File: %s\nSource Line: %d\nFunction: %s\nErrno: %s\n", ##__VA_ARGS__, __FILE__, __LINE__, \
              __PRETTY_FUNCTION__, GetStrerror().c_str())

#define qcore_panic(msg) qcore_panicf("%s", msg)

#define qcore_assert(expr, ...)      \
  (static_cast<bool>(expr) ? void(0) \
                           : qcore_panicf("Assertion failed: %s;\nCondition: (%s);\n", "" #__VA_ARGS__, #expr))
#endif

#define qcore_implement() qcore_panicf("%s is not implemented.", __func__)

#ifdef __cplusplus
}
#endif

#endif
