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

#include <execinfo.h>
#include <nitrate-core/Error.h>

#include <cstdlib>
#include <iostream>
#include <vector>

#include "LibMacro.h"

#define PROJECT_REPO_URL "https://github.com/Kracken256/quix"
#define PANIC_LINE_LENGTH 80

#if defined(__GLIBC__)
#include <gnu/libc-version.h>
static std::string libc_version = std::string("GLIBC ") + gnu_get_libc_version();
#else
#error "This libc version is not supported here"
#endif

static std::string base64_encode(const std::string &in) {
  std::string out;

  int val = 0, valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[((val << 8) >>
                                                                                      (valb + 8)) &
                                                                                     0x3F]);
  return out;
}

static std::string escape_json_string(const std::string &s) {
  std::string out;
  for (char c : s) {
    switch (c) {
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\"':
        out += "\\\"";
        break;
      default:
        out += c;
        break;
    }
  }
  return out;
}

static std::string panic_create_report() {
  std::vector<std::string> trace;

  void *array[48];
  size_t size = backtrace(array, 48);
  char **strings = backtrace_symbols(array, size);

  for (size_t i = 0; i < size && strings[i]; i++) trace.push_back(strings[i]);

  free(strings);

  std::string report = "{\"version\":\"1.0\",";
  report += "\"qcore_version\":\"" __TARGET_VERSION "\",";

#if NDEBUG
  report += "\"build\":\"release\",";
#else
  report += "\"build\":\"debug\",";
#endif

#if defined(__clang__)
  report += "\"compiler\":\"clang\",";
#elif defined(__GNUC__)
  report += "\"compiler\":\"gnu\",";
#else
  report += "\"compiler\":\"unknown\",";
#endif

#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) || \
    defined(_M_AMD64)
  report += "\"arch\":\"x86_64\",";
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
  report += "\"arch\":\"x86\",";
#elif defined(__aarch64__)
  report += "\"arch\":\"aarch64\",";
#elif defined(__arm__)
  report += "\"arch\":\"arm\",";
#else
  report += "\"arch\":\"unknown\",";
#endif

#if defined(__linux__)
  report += "\"os\":\"linux\",";
#elif defined(__APPLE__)
  report += "\"os\":\"macos\",";
#elif defined(_WIN32)
  report += "\"os\":\"windows\",";
#else
  report += "\"os\":\"unknown\",";
#endif

  report += "\"trace\":[";
  for (size_t i = 0; i < trace.size(); i++) {
    report += "\"" + escape_json_string(trace[i]) + "\"";
    if (i + 1 < trace.size()) report += ",";
  }

  report += "]}";

  return "LIBNITRATE_CRASHINFO_" + base64_encode(report);
}

static std::vector<std::string> panic_split_message(std::string_view message) {
  std::string buf;
  std::vector<std::string> lines;

  for (size_t i = 0; i < message.size(); i++) {
    switch (message[i]) {
      case '\n': {
        size_t pad = PANIC_LINE_LENGTH - buf.size() - 4;
        if (pad > 0) {
          buf += std::string(pad, ' ');
        }
        lines.push_back(buf);
        buf.clear();
        break;
      }
      default: {
        if (buf.size() + 4 < PANIC_LINE_LENGTH) {
          buf += message[i];
        } else {
          size_t pad = PANIC_LINE_LENGTH - buf.size() - 4;
          if (pad > 0) {
            buf += std::string(pad, ' ');
          }
          lines.push_back(buf);
          buf.clear();
          buf += message[i];
        }

        break;
      }
    }
  }

  if (!buf.empty()) {
    size_t pad = PANIC_LINE_LENGTH - buf.size() - 4;
    if (pad > 0) {
      buf += std::string(pad, ' ');
    }
    lines.push_back(buf);
  }

  return lines;
}

static void panic_render_report(const std::vector<std::string> &lines) {
  { /* Print shockwave */
    std::cout << "\n\n";
    std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
                 "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
    std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
                 "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n\n";
  }

  {
    std::string sep;
    for (size_t i = 0; i < PANIC_LINE_LENGTH - 2; i++) sep += "━";

    std::cerr << "\x1b[31;1m┏" << sep << "┓\x1b[0m\n";
    for (auto &str : lines) std::cerr << "\x1b[31;1m┃\x1b[0m " << str << " \x1b[31;1m┃\x1b[0m\n";
    std::cerr << "\x1b[31;1m┗" << sep << "\x1b[31;1m┛\x1b[0m\n\n";
  }

  setlocale(LC_ALL, "");

  std::cerr << "\x1b[31;1m┏━━━━━━┫ BEGIN STACK TRACE ┣━━\x1b[0m\n";
  std::cerr << "\x1b[31;1m┃\x1b[0m\n";

  void *array[48];
  size_t size = backtrace(array, 48);
  char **strings = backtrace_symbols(array, size);

  for (size_t i = 0; i < size && strings[i]; i++)
    std::cerr << "\x1b[31;1m┣╸╸\x1b[0m \x1b[37;1m" << strings[i] << "\x1b[0m\n";

  free(strings);

  std::cerr << "\x1b[31;1m┃\x1b[0m\n";
  std::cerr << "\x1b[31;1m┗━━━━━━┫ END STACK TRACE ┣━━\x1b[0m\n\n";

  std::cerr << "libc version: " << libc_version << std::endl;

  std::cerr << "The application has encountered a fatal internal "
               "error.\n\n";
  std::cerr << "\x1b[32;40;1;4mPlease report this error\x1b[0m to the QuixCC "
               "developers "
               "at\n\x1b[36;1;4m" PROJECT_REPO_URL "\x1b[0m.\n\n";
  std::cerr << "\x1b[33;49;1;4mPlease include the following report "
               "code:\x1b[0m \n\n  "
            << panic_create_report() << "\n";

  std::cerr << "\n\n▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
  std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n";

  std::cerr << "\nAborting..." << std::endl;
}

LIB_EXPORT void qcore_panic_(const char *msg) { qcore_panicf_("%s", msg); }

LIB_EXPORT void qcore_panicf_(const char *_fmt, ...) {
  va_list args;
  va_start(args, _fmt);
  qcore_vpanicf_(_fmt, args);
  va_end(args);  // Unreachable, but whatever
}  // Unreachable, but whatever

LIB_EXPORT void qcore_vpanicf_(const char *fmt, va_list args) {
  char *msg = nullptr;

  { /* Parse the format string */
    int ret = vasprintf(&msg, fmt, args);
    (void)ret;
  }

  panic_render_report(panic_split_message(msg));

  free(msg);

  abort();
}

LIB_EXPORT void qcore_debug_(const char *msg) { return qcore_debugf_("%s", msg); }

LIB_EXPORT void qcore_debugf_(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  qcore_vdebugf_(fmt, args);
  va_end(args);
}

LIB_EXPORT void qcore_vdebugf_(const char *fmt, va_list args) { vfprintf(stderr, fmt, args); }