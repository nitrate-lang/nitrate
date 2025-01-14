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

#ifndef __NITRATE_CORE_OLD_LOGGER_H__
#define __NITRATE_CORE_OLD_LOGGER_H__

#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

void qcore_panic_(const char *msg) __attribute__((noreturn));

void qcore_panicf_(const char *fmt, ...) __attribute__((noreturn));
void qcore_vpanicf_(const char *fmt, va_list args) __attribute__((noreturn));

void qcore_debug_(const char *msg);
void qcore_debugf_(const char *fmt, ...);
void qcore_vdebugf_(const char *fmt, va_list args);

#if defined(NDEBUG)
#define qcore_panicf(fmt, ...)                                              \
  qcore_panicf_(                                                            \
      fmt                                                                   \
      "\nSource File: %s\nSource Line: %d\nFunction: unknown\nErrno: %s\n", \
      ##__VA_ARGS__, __FILE__, __LINE__, strerror(errno))

#define qcore_panic(msg) qcore_panicf("%s", msg)

#define qcore_assert(expr, ...)                                    \
  (static_cast<bool>(expr)                                         \
       ? void(0)                                                   \
       : qcore_panicf("Assertion failed: %s;\nCondition: (%s);\n", \
                      "" #__VA_ARGS__, #expr))
#else
#define qcore_panicf(fmt, ...)                                             \
  qcore_panicf_(                                                           \
      fmt "\nSource File: %s\nSource Line: %d\nFunction: %s\nErrno: %s\n", \
      ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__, strerror(errno))

#define qcore_panic(msg) qcore_panicf("%s", msg)

#define qcore_assert(expr, ...)                                    \
  (static_cast<bool>(expr)                                         \
       ? void(0)                                                   \
       : qcore_panicf("Assertion failed: %s;\nCondition: (%s);\n", \
                      "" #__VA_ARGS__, #expr))
#endif

#if defined(NDEBUG) || defined(QCORE_NDEBUG)
#define qcore_debugf(fmt, ...)
#define qcore_debug(msg)
#else
#define qcore_debugf(fmt, ...) qcore_debugf_(fmt, ##__VA_ARGS__)
#define qcore_debug(msg) qcore_debug_(msg)
#endif

#define qcore_implement() qcore_panicf("%s is not implemented.", __func__)

typedef enum {
  QCORE_DEBUG,
  QCORE_INFO,
  QCORE_WARN,
  QCORE_ERROR,
  QCORE_FATAL,
} qcore_log_t;

void qcore_begin();
int qcore_vwritef(const char *fmt, va_list args);
void qcore_end(qcore_log_t level);

typedef void (*qcore_logger_t)(qcore_log_t level, const char *msg, size_t len,
                               void *);

static inline int qcore_writef(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = qcore_vwritef(fmt, args);
  va_end(args);
  return ret;
}

static inline int qcore_write(const char *msg) {
  return qcore_writef("%s", msg);
}

#define qcore_logf(_lvl, ...)  \
  do {                         \
    qcore_begin();             \
    qcore_writef(__VA_ARGS__); \
    qcore_end(_lvl);           \
  } while (0)

#define qcore_print(_lvl, _msg) \
  do {                          \
    qcore_begin();              \
    qcore_writef("%s", _msg);   \
    qcore_end(_lvl);            \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif
