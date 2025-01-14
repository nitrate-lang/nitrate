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

#include <cxxabi.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <vector>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#define PROJECT_REPO_URL "https://github.com/Kracken256/nitrate"
#define PANIC_LINE_LENGTH 80

#if defined(__GLIBC__)
#include <gnu/libc-version.h>
static std::string libc_version =
    std::string("GLIBC ") + gnu_get_libc_version();
#else
#error "This libc version is not supported here"
#endif

static inline std::vector<std::pair<uintptr_t, std::string>> get_backtrace(
    void) {
  unw_cursor_t cursor;
  unw_context_t uc;
  unw_word_t ip, sp, off;
  char sym[512];
  std::vector<std::pair<uintptr_t, std::string>> trace;

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);
  while (unw_step(&cursor) > 0) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &off) == 0) {
      std::string_view name(sym);
      if (name.starts_with("_Z")) {
        int status;
        char *demangled = abi::__cxa_demangle(sym, 0, 0, &status);
        if (status == 0) {
          trace.push_back({ip, demangled});
          free(demangled);
          continue;
        }
      }

      trace.push_back({ip, sym});
    } else {
      trace.push_back({ip, "<unknown>"});
    }
  }

  return trace;
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
    std::cerr
        << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
           "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
    std::cerr
        << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
           "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n\n";
  }

  {
    std::string sep;
    for (size_t i = 0; i < PANIC_LINE_LENGTH - 2; i++) sep += "━";

    std::cerr << "\x1b[31;1m┏" << sep << "┓\x1b[0m\n";
    for (auto &str : lines)
      std::cerr << "\x1b[31;1m┃\x1b[0m " << str << " \x1b[31;1m┃\x1b[0m\n";
    std::cerr << "\x1b[31;1m┗" << sep << "\x1b[31;1m┛\x1b[0m\n\n";
  }

  setlocale(LC_ALL, "");

  std::cerr << "\x1b[31;1m┏━━━━━━┫ BEGIN STACK TRACE ┣━━\x1b[0m\n";
  std::cerr << "\x1b[31;1m┃\x1b[0m\n";

  auto trace = get_backtrace();

  for (size_t i = 0; i < trace.size(); i++) {
    std::cerr << "\x1b[31;1m┣╸╸\x1b[0m \x1b[37;1m";
    std::cerr << "0x" << std::hex << std::setfill('0') << trace[i].first
              << ":\x1b[0m \t";
    std::cerr << trace[i].second << "\n";
  }
  std::cerr << std::dec;

  std::cerr << "\x1b[31;1m┃\x1b[0m\n";
  std::cerr << "\x1b[31;1m┗━━━━━━┫ END STACK TRACE ┣━━\x1b[0m\n\n";

  std::cerr << "libc version: " << libc_version << std::endl;

  std::cerr << "The application has encountered a fatal internal "
               "error.\n\n";
  std::cerr << "\x1b[32;40;1;4mPlease report this error\x1b[0m to the Nitrate "
               "developers "
               "at\n\x1b[36;1;4m" PROJECT_REPO_URL "\x1b[0m.\n\n";

  std::cerr
      << "\n\n▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
         "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
  std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n";

  std::cerr << "\nAborting..." << std::endl;
}

extern "C" NCC_EXPORT void qcore_panic_(const char *msg) {
  qcore_panicf_("%s", msg);
}

extern "C" NCC_EXPORT void qcore_panicf_(const char *_fmt, ...) {
  va_list args;
  va_start(args, _fmt);
  qcore_vpanicf_(_fmt, args);
  va_end(args);  // Unreachable, but whatever
}  // Unreachable, but whatever

extern "C" NCC_EXPORT void qcore_vpanicf_(const char *fmt, va_list args) {
  char *msg = nullptr;

  { /* Parse the format string */
    int ret = vasprintf(&msg, fmt, args);
    (void)ret;
  }

  panic_render_report(panic_split_message(msg));

  free(msg);

  abort();
}

extern "C" NCC_EXPORT void qcore_debug_(const char *msg) {
  return qcore_debugf_("%s", msg);
}

extern "C" NCC_EXPORT void qcore_debugf_(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  qcore_vdebugf_(fmt, args);
  va_end(args);
}

extern "C" NCC_EXPORT void qcore_vdebugf_(const char *fmt, va_list args) {
  vfprintf(stderr, fmt, args);
}