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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <core/Macro.h>
#include <execinfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <quixcc/Library.h>

#include <atomic>
#include <cstdarg>
#include <iostream>
#include <mutex>
#include <thread>

#define PROJECT_REPO_URL "https://github.com/Kracken256/quix"

#ifndef LIBQUIX_VERSION
#warning "LIBQUIX_VERSION not defined; using default value"
#define LIBQUIX_VERSION "undefined"
#endif

std::atomic<bool> g_is_initialized = false;
std::atomic<uint64_t> g_num_of_contexts = 0;
std::mutex g_library_lock;

static struct {
  std::mutex m_lock;
  quixcc_cache_has_t m_has;
  quixcc_cache_read_t m_read;
  quixcc_cache_write_t m_write;
} g_cache_provider{};

LIB_EXPORT bool quixcc_lib_init() {
  /* We don't need to initialize more than once */
  if (g_is_initialized)
    return true;

  static std::mutex g_mutex;
  std::lock_guard<std::mutex> lock(g_mutex);

#ifdef LLVM_SUUPORT_ALL_TARGETS
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  /* Check if LLVM is initialized */
  if (llvm::TargetRegistry::targets().empty()) {
    std::cerr << "error: LLVM initialization failed" << std::endl;
    return false;
  }
#else
#warning "Building LIBQUIXCC without support for ANY LLVM targets!!"
#endif

  g_is_initialized = true;
  return true;
}

LIB_EXPORT bool quixcc_lib_deinit() {
  /* We don't need to deinitialize more than once */
  if (!g_is_initialized)
    return true;

  static std::mutex g_mutex;
  std::lock_guard<std::mutex> lock(g_mutex);

  /// TODO: Deinitialize everything

  g_is_initialized = false;
  return true;
}
LIB_EXPORT const char *quixcc_lib_version() {
  static std::string version_string =

      "[" LIBQUIX_VERSION "] ["

#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) ||            \
    defined(_M_AMD64)
      "x86_64-"
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
      "x86-"
#elif defined(__aarch64__)
      "aarch64-"
#elif defined(__arm__)
      "arm-"
#else
      "unknown-"
#endif

#if defined(__linux__)
      "linux-"
#elif defined(__APPLE__)
      "macos-"
#elif defined(_WIN32)
      "win32-"
#else
      "unknown-"
#endif

#if defined(__clang__)
      "clang] "
#elif defined(__GNUC__)
      "gnu] "
#else
      "unknown] "
#endif

#if NDEBUG
      "[release]"
#else
      "[debug]"
#endif

      ;

  return version_string.c_str();
}

LIB_EXPORT bool quixcc_cache_bind(quixcc_cache_has_t has, quixcc_cache_read_t read,
                                  quixcc_cache_write_t write) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cache_bind(). "
                 "quixcc_cache_bind() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  if (!has || !read || !write) {
    return false;
  }

  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  if (g_cache_provider.m_has || g_cache_provider.m_read || g_cache_provider.m_write) {
    return false;
  }

  g_cache_provider.m_has = has;
  g_cache_provider.m_read = read;
  g_cache_provider.m_write = write;

  return true;
}

LIB_EXPORT void quixcc_cache_unbind() {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cache_bind(). "
                 "quixcc_cache_bind() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  g_cache_provider.m_has = nullptr;
  g_cache_provider.m_read = nullptr;
  g_cache_provider.m_write = nullptr;
}

LIB_EXPORT ssize_t quixcc_cache_has(const quixcc_cache_key_t *key) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cache_bind(). "
                 "quixcc_cache_bind() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  if (!g_cache_provider.m_has) {
    return -1;
  }

  return g_cache_provider.m_has(key);
}

LIB_EXPORT bool quixcc_cache_read(const quixcc_cache_key_t *key, void *data, size_t datalen) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cache_bind(). "
                 "quixcc_cache_bind() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  if (!g_cache_provider.m_read) {
    return false;
  }

  return g_cache_provider.m_read(key, data, datalen);
}

LIB_EXPORT bool quixcc_cache_write(const quixcc_cache_key_t *key, const void *data,
                                   size_t datalen) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cache_bind(). "
                 "quixcc_cache_bind() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  std::lock_guard<std::mutex> lock(g_cache_provider.m_lock);

  if (!g_cache_provider.m_write) {
    return false;
  }

  return g_cache_provider.m_write(key, data, datalen);
}

LIB_EXPORT bool quixcc_cache_reset() {
  /* Acquire a lock on the library state */
  std::lock_guard<std::mutex> lock(g_library_lock);

  /* Ensure that no contexts are active */
  if (g_num_of_contexts != 0)
    return false;

  /* We have a guarantee that no contexts are active,
   * and none will be created, for now. */

  /* Erase cache line and free the memory */

  // Nothing to do here.

  /* We now have an empty cache and can return */
  return true;
}

std::string base64_encode(const std::string &in) {
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

static std::string geterror_report_string() {
  std::vector<std::string> trace;

  void *array[48];
  size_t size = backtrace(array, 48);
  char **strings = backtrace_symbols(array, size);

  for (size_t i = 0; i < size && strings[i]; i++)
    trace.push_back(strings[i]);

  free(strings);

  std::string report = "{\"version\":\"1.0\",";
  report += "\"quixcc_version\":\"" LIBQUIX_VERSION "\",";

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

#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) ||            \
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

  report += "\"quixcc_run\":\"";

  char buf[(sizeof(void *) * 2) + 2 + 1] = {0}; // 0x[hex word]\0
  snprintf(buf, sizeof(buf), "%p", (void *)quixcc_cc_run);
  report += buf;

  report += "\",\"trace\":[";
  for (size_t i = 0; i < trace.size(); i++) {
    report += "\"" + escape_json_string(trace[i]) + "\"";
    if (i + 1 < trace.size())
      report += ",";
  }

  report += "]}";

  return "LIBQUIXCC_CRASHINFO_" + base64_encode(report);
}

void quixcc_print_stacktrace() {
  // UTF-8 support
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
  std::cerr << "\x1b[31;1m┗━━━━━━┫ END STACK TRACE ┣━━\x1b[0m" << std::endl;
}

void quixcc_print_general_fault_message() {
  std::cerr << "Libquixcc Version: " LIBQUIX_VERSION "\n\n";
  std::cerr << "The libquixcc library has encountered a fatal internal "
               "error.\n\n";
  std::cerr << "\x1b[32;40;1;4mPlease report this error\x1b[0m to the QuixCC "
               "developers "
               "at\n\x1b[36;1;4m" PROJECT_REPO_URL "\x1b[0m.\n\n";
  std::cerr << "\x1b[33;49;1;4mPlease include the following report "
               "code:\x1b[0m \n\n  "
            << geterror_report_string() << std::endl;
}

LIB_EXPORT [[noreturn]] void quixcc_panic(const char *msg) { quixcc_panicf("%s", msg); }

LIB_EXPORT [[noreturn]] void quixcc_panicf(const char *_fmt, ...) {
  char *_msg = nullptr;

  std::cout << "\n\n";
  std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
  std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n\n";

  { // Create the message
    va_list args;
    va_start(args, _fmt);
    int ret = vasprintf(&_msg, _fmt, args);
    (void)ret;
    va_end(args);
  }

  std::string msg = _msg;

  std::vector<std::string> lines;
  const size_t max = 80;

  { // Split the message into lines
    std::string buf;

    for (size_t i = 0; i < msg.size(); i++) {
      switch (msg[i]) {
      case '\n': {
        size_t pad = max - buf.size() - 4;
        if (pad > 0) {
          buf += std::string(pad, ' ');
        }
        lines.push_back(buf);
        buf.clear();
        break;
      }
      default: {
        if (buf.size() + 4 < max) {
          buf += msg[i];
        } else {
          size_t pad = max - buf.size() - 4;
          if (pad > 0) {
            buf += std::string(pad, ' ');
          }
          lines.push_back(buf);
          buf.clear();
          buf += msg[i];
        }

        break;
      }
      }
    }

    if (!buf.empty()) {
      size_t pad = max - buf.size() - 4;
      if (pad > 0) {
        buf += std::string(pad, ' ');
      }
      lines.push_back(buf);
    }
  }

  std::string sep;
  for (size_t i = 0; i < max - 2; i++)
    sep += "━";

  std::cerr << "\x1b[31;1m┏" << sep << "┓\x1b[0m" << std::endl;
  for (auto &str : lines)
    std::cerr << "\x1b[31;1m┃\x1b[0m " << str << " \x1b[31;1m┃\x1b[0m" << std::endl;
  std::cerr << "\x1b[31;1m┗" << sep << "\x1b[31;1m┛\x1b[0m\n" << std::endl;

  quixcc_print_stacktrace();

  std::cerr << std::endl;

  quixcc_print_general_fault_message();

  std::cerr << "\n\n▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n";
  std::cerr << "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚"
               "▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚▚\n\n";

  std::cerr << "\nAborting..." << std::endl;
  abort();

  while (true)
    std::this_thread::yield();
}
