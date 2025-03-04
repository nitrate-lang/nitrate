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
#include <mutex>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <vector>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

static constexpr auto kProjectRepoUrl = "https://github.com/Kracken256/nitrate";
static constexpr size_t kPanicLineLength = 80;

#if defined(__GLIBC__)
#include <gnu/libc-version.h>
static std::string LibcVersion = std::string("GLIBC ") + gnu_get_libc_version();
#else
#error "This libc version is not supported here"
#endif

static inline auto GetBacktrace() -> std::vector<std::pair<uintptr_t, std::string>> {
  constexpr size_t kMaxSymbolSize = 512;
  unw_cursor_t cursor;
  unw_context_t uc;
  unw_word_t ip;
  unw_word_t sp;
  unw_word_t off;
  std::array<char, kMaxSymbolSize> sym;
  std::vector<std::pair<uintptr_t, std::string>> trace;

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);
  while (unw_step(&cursor) > 0) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    if (unw_get_proc_name(&cursor, sym.data(), sym.size(), &off) == 0) {
      std::string_view name(sym.data());
      if (name.starts_with("_Z")) {
        int status;
        char *demangled = abi::__cxa_demangle(sym.data(), nullptr, nullptr, &status);
        if (status == 0) {
          trace.emplace_back(ip, demangled);
          free(demangled);
          continue;
        }
      }

      trace.emplace_back(ip, sym.data());
    } else {
      trace.emplace_back(ip, "<unknown>");
    }
  }

  return trace;
}

static auto PanicSplitMessage(std::string_view message) -> std::vector<std::string> {
  std::string buf;
  std::vector<std::string> lines;

  for (auto c : message) {
    switch (c) {
      case '\n': {
        size_t pad = kPanicLineLength - buf.size() - 4;
        if (pad > 0) {
          buf += std::string(pad, ' ');
        }
        lines.push_back(buf);
        buf.clear();
        break;
      }
      default: {
        if (buf.size() + 4 < kPanicLineLength) {
          buf += c;
        } else {
          lines.push_back(buf);
          buf.clear();
          buf += c;
        }

        break;
      }
    }
  }

  if (!buf.empty()) {
    size_t pad = kPanicLineLength - buf.size() - 4;
    if (pad > 0) {
      buf += std::string(pad, ' ');
    }
    lines.push_back(buf);
  }

  return lines;
}

static void PanicRenderReport(const std::vector<std::string> &lines) {
  static std::mutex panic_mutex;
  std::lock_guard<std::mutex> lock(panic_mutex);

  std::setlocale(LC_ALL, "");  // NOLINT

  { /* Print shockwave */
    std::cerr << "\n\n";
    std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
                 "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
    std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
                 "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n\n";
  }

  {
    std::string sep;
    for (size_t i = 0; i < kPanicLineLength - 2; i++) {
      sep += "━";
    }

    std::cerr << "\x1b[31;1m┏" << sep << "┓\x1b[0m\n";
    for (const auto &str : lines) {
      std::cerr << "\x1b[31;1m┃\x1b[0m " << str << " \x1b[31;1m┃\x1b[0m\n";
    }
    std::cerr << "\x1b[31;1m┗" << sep << "\x1b[31;1m┛\x1b[0m\n\n";
  }

  std::cerr << "\x1b[31;1m┏━━━━━━┫ BEGIN STACK TRACE ┣━━\x1b[0m\n";
  std::cerr << "\x1b[31;1m┃\x1b[0m\n";

  auto trace = GetBacktrace();

  for (auto &[addr, name] : trace) {
    std::cerr << "\x1b[31;1m┣╸╸\x1b[0m \x1b[37;1m";
    std::cerr << "0x" << std::hex << std::setfill('0') << addr << std::dec << ":\x1b[0m \t";
    std::cerr << name << "\n";
  }
  std::cerr << std::dec;

  std::cerr << "\x1b[31;1m┃\x1b[0m\n";
  std::cerr << "\x1b[31;1m┗━━━━━━┫ END STACK TRACE ┣━━\x1b[0m\n\n";

  std::cerr << "libc version: " << LibcVersion << "\n";

  std::cerr << "The application has encountered a fatal internal "
               "error.\n\n";
  std::cerr << "\x1b[32;40;1;4mPlease report this error\x1b[0m to the Nitrate "
               "developers "
               "at\n\x1b[36;1;4m"
            << kProjectRepoUrl << "\x1b[0m.\n\n";

  std::cerr << "\n\n▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
  std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n";

  std::cerr << "\nAborting..." << "\n";
  std::cerr.flush();
}

extern "C" NCC_EXPORT void QCorePanic(const char *msg) { QCorePanicF("%s", msg); }

[[noreturn]] static void QCoreVPanicF(const char *fmt, va_list args) {
  char *msg = nullptr;

  { /* Parse the format string */
    int ret = vasprintf(&msg, fmt, args);
    (void)ret;
  }

  PanicRenderReport(PanicSplitMessage(msg));

  free(msg);

  abort();
}

extern "C" NCC_EXPORT void QCorePanicF(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  QCoreVPanicF(fmt, args);
  va_end(args);
}
