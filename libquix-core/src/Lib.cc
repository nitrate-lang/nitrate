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

#include <quix-core/Lib.h>

#include <atomic>

#include "LibMacro.h"

#ifndef QCORE_ID
#warning "QCORE_ID must be defined"
#define QCORE_ID "?"
#endif

static std::atomic<size_t> qcore_lib_ref_count = 0;

LIB_EXPORT bool qcore_lib_init() {
  qcore_lib_ref_count++;
  return true;
}

LIB_EXPORT void qcore_lib_deinit() {
  qcore_lib_ref_count--;

  if (qcore_lib_ref_count > 0) {
    return;
  }

  // Deinitialize the library here.
  // Nothing to do for now.
}

LIB_EXPORT const char* qcore_lib_version() {
  static const char* version_string =

      "[" QCORE_ID
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