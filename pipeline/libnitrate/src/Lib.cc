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

#define LIBNITRATE_INTERNAL

#include <nitrate-core/Lib.h>
#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-lexer/Lib.h>
#include <nitrate-parser/Lib.h>
#include <nitrate-seq/Lib.h>
#include <nitrate/code.h>

#include <atomic>

#include "LibMacro.h"

extern "C" {
__attribute__((visibility("default"))) bool nit_lib_ready;
}

static std::atomic<size_t> nit_lib_ref_count = 0;

static bool do_init() {
  nit_lib_ready = true;
  return true;
}

static void do_deinit() { nit_lib_ready = false; }

LIB_EXPORT bool nit_lib_init() {
  if (nit_lib_ref_count++ > 1) {
    return true;
  }

  if (!qcore_lib_init()) {
    return false;
  }

  if (!qlex_lib_init()) {
    return false;
  }

  if (!qprep_lib_init()) {
    return false;
  }

  if (!qparse_lib_init()) {
    return false;
  }

  if (!nr_lib_init()) {
    return false;
  }

  if (!qcode_lib_init()) {
    return false;
  }

  return do_init();
}

LIB_EXPORT void nit_deinit() {
  if (--nit_lib_ref_count > 0) {
    return;
  }

  do_deinit();

  qcode_lib_deinit();
  nr_lib_deinit();
  qparse_lib_deinit();
  qprep_lib_deinit();
  qlex_lib_deinit();
  qcore_lib_deinit();
}

LIB_EXPORT const char* nit_lib_version() {
  static const char* version_string =

      "[" __TARGET_VERSION
      "] ["

#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) || \
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

  return version_string;
}
