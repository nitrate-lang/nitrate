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

#include <nitrate-seq/Lib.h>

#include <atomic>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>

static std::atomic<size_t> qprep_lib_ref_count = 0;

bool do_init() { return true; }

void do_deinit() { return; }

C_EXPORT bool qprep_lib_init() {
  if (qprep_lib_ref_count++ > 1) {
    return true;
  }

  if (!ncc::core::CoreLibrary::InitRC()) {
    return false;
  }

  if (!ncc::lex::LexerLibrary::InitRC()) {
    return false;
  }

  return do_init();
}

C_EXPORT void qprep_lib_deinit() {
  if (--qprep_lib_ref_count > 0) {
    return;
  }

  do_deinit();

  ncc::lex::LexerLibrary::DeinitRC();
  ncc::core::CoreLibrary::DeinitRC();

  return;
}

C_EXPORT const char* qprep_lib_version() {
  static const char* version_string =

      "[" __TARGET_VERSION
      "] ["

#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || \
    defined(_M_X64) || defined(_M_AMD64)
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

C_EXPORT const char* qprep_strerror() { return ""; }