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

#include <nitrate-ir/Lib.h>
#include <nitrate-parser/Lib.h>
#include <sys/resource.h>

#include <atomic>
#include <boost/assert/source_location.hpp>
#include <core/PassManager.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>

namespace boost {
  void throw_exception(std::exception const& m, boost::source_location const&) {
    std::cerr << "boost::throw_exception: " << m.what();
    std::terminate();
  }

  void throw_exception(std::exception const& m) {
    std::cerr << "boost::throw_exception: " << m.what();
    std::terminate();
  }
}  // namespace boost

static std::atomic<size_t> nr_lib_ref_count = 0;

static bool do_init() {
  nr::pass::PassGroupRegistry::RegisterBuiltinGroups();

  return true;
}

static void do_deinit() {}

C_EXPORT bool nr_lib_init() {
  if (nr_lib_ref_count++ > 1) {
    return true;
  }

  if (!ncc::core::Library::InitRC()) {
    return false;
  }

  if (!npar_lib_init()) {
    return false;
  }

  return do_init();
}

C_EXPORT void nr_lib_deinit() {
  if (--nr_lib_ref_count > 0) {
    return;
  }

  npar_lib_deinit();
  ncc::core::Library::DeinitRC();

  return do_deinit();
}

C_EXPORT const char* nr_lib_version() {
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

C_EXPORT const char* nr_strerror() { return ""; }
