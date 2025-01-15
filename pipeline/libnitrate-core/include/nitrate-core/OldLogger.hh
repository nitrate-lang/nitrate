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
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

void QCorePanic(const char *msg) __attribute__((noreturn));

void QCorePanicF(const char *fmt, ...) __attribute__((noreturn));
void QCoreVPanicF(const char *fmt, va_list args) __attribute__((noreturn));

void QCoreDebug(const char *msg);
void QCoreDebugF(const char *fmt, ...);
void QCoreVDebugF(const char *fmt, va_list args);

#if defined(NDEBUG)
#define qcore_panicf(fmt, ...)                                              \
  QCorePanicF(                                                              \
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
  QCorePanicF(                                                             \
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
#define qcore_debugf(fmt, ...) QCoreDebugF(fmt, ##__VA_ARGS__)
#define qcore_debug(msg) QCoreDebug(msg)
#endif

#define qcore_implement() qcore_panicf("%s is not implemented.", __func__)

typedef enum {
  QCORE_DEBUG,
  QCORE_INFO,
  QCORE_WARN,
  QCORE_ERROR,
  QCORE_FATAL,
} QCoreLog;

void QCoreBegin();
int QCoreVWriteF(const char *fmt, va_list args);
void QCoreEnd(QCoreLog level);

static inline int QCoreWritef(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = QCoreVWriteF(fmt, args);
  va_end(args);
  return ret;
}

static inline int QCoreWrite(const char *msg) { return QCoreWritef("%s", msg); }

#define qcore_logf(_lvl, ...) \
  do {                        \
    QCoreBegin();             \
    QCoreWritef(__VA_ARGS__); \
    QCoreEnd(_lvl);           \
  } while (0)

#define qcore_print(_lvl, _msg) \
  do {                          \
    QCoreBegin();               \
    QCoreWritef("%s", _msg);    \
    QCoreEnd(_lvl);             \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif
