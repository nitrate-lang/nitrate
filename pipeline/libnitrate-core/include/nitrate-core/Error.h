////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
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

#ifndef __NITRATE_CORE_ERROR_H__
#define __NITRATE_CORE_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Print debug into and abort.
 *
 * @param msg The message to print.
 * @note This function is thread-safe.
 *
 * @note This function will print the message to stderr and abort the program.
 */
void qcore_panic_(const char *msg) __attribute__((noreturn));

/**
 * @brief Print debug into and abort.
 *
 * @param fmt The format string.
 * @param ... Printf-style arguments.
 * @note This function is thread-safe.
 *
 * @note This function will print the message to stderr and abort the program.
 */
void qcore_panicf_(const char *fmt, ...) __attribute__((noreturn));
void qcore_vpanicf_(const char *fmt, va_list args) __attribute__((noreturn));

void qcore_debug_(const char *msg);
void qcore_debugf_(const char *fmt, ...);
void qcore_vdebugf_(const char *fmt, va_list args);

#define qcore_panicf(fmt, ...)                                                       \
  qcore_panicf_(fmt "\nSource File: %s\nSource Line: %d\nFunction: %s\nErrno: %s\n", \
                ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__, strerror(errno))

#define qcore_panic(msg) qcore_panicf("%s", msg)

#define qcore_assert(expr, ...) \
  (static_cast<bool>(expr)      \
       ? void(0)                \
       : qcore_panicf("Assertion failed: %s;\nCondition: (%s);\n", "" #__VA_ARGS__, #expr))

#if defined(NDEBUG) || defined(QCORE_NDEBUG)
#define qcore_debugf(fmt, ...)
#define qcore_debug(msg)
#else
#define qcore_debugf(fmt, ...) qcore_debugf_(fmt, ##__VA_ARGS__)
#define qcore_debug(msg) qcore_debug_(msg)
#endif

#define qcore_implement() qcore_panicf("%s is not implemented.", __func__)

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_CORE_ERROR_H__
