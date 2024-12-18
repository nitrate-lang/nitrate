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

#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>

using namespace ncc::core;

size_t Library::ref_count;
std::mutex Library::ref_count_mutex;

CPP_EXPORT CoreLibraryAutoClose::~CoreLibraryAutoClose() {
  Library::DeinitRC();
}

bool Library::init_library() {
  // Nothing to do here for now.

  return true;
}

void Library::deinit_library() {
  /* After nitrate-core is deinitialized, all str_aliases are invalid. */
  StringMemory::Clear();
}

CPP_EXPORT bool Library::InitRC() {
  std::lock_guard<std::mutex> lock(ref_count_mutex);

  if (ref_count == 0) {
    if (!init_library()) {
      return false;
    }
  }

  ++ref_count;

  return true;
}

CPP_EXPORT void Library::DeinitRC() {
  std::lock_guard<std::mutex> lock(ref_count_mutex);

  if (ref_count > 0 && --ref_count == 0) {
    deinit_library();
  }
}

CPP_EXPORT bool Library::IsInitialized() {
  std::lock_guard<std::mutex> lock(ref_count_mutex);

  return ref_count > 0;
}

CPP_EXPORT std::string_view Library::GetVersion() {
  static std::string_view version_string =

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

CPP_EXPORT std::optional<CoreLibraryAutoClose> Library::GetRC() {
  if (!InitRC()) {
    return std::nullopt;
  }

  return CoreLibraryAutoClose();
}
