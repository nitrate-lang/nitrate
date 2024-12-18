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

#ifndef __NITRATE_CORE_LIB_H__
#define __NITRATE_CORE_LIB_H__

#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace ncc::core {
  template <typename Impl>
  class LibraryRC;

  template <typename Impl>
  class LibraryRCAutoClose final {
    friend class LibraryRC<Impl>;

    LibraryRCAutoClose() = default;

  public:
    ~LibraryRCAutoClose() { Impl::DeinitRC(); }
  };

  template <typename Impl>
  class LibraryRC final {
    LibraryRC() = delete;

    inline static size_t ref_count;
    inline static std::mutex ref_count_mutex;

  public:
    static bool InitRC() {
      std::lock_guard<std::mutex> lock(ref_count_mutex);

      if (ref_count == 0) {
        if (!Impl::Init()) {
          return false;
        }
      }

      ++ref_count;

      return true;
    }

    static void DeinitRC() {
      std::lock_guard<std::mutex> lock(ref_count_mutex);

      if (ref_count > 0 && --ref_count == 0) {
        Impl::Deinit();
      }
    }

    static bool IsInitialized() {
      std::lock_guard<std::mutex> lock(ref_count_mutex);

      return ref_count > 0;
    }

    static std::string_view GetVersion() {
      static std::string version_string =

          std::string("[") + std::string(Impl::GetVersionId()) +
          std::string(
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
          );

      return version_string;
    }

    static std::optional<LibraryRCAutoClose<Impl>> GetRC() {
      if (!InitRC()) {
        return std::nullopt;
      }

      return LibraryRCAutoClose<Impl>();
    }
  };

  struct CoreLibrarySetup final {
    static bool Init();
    static void Deinit();
    static std::string_view GetVersionId();
  };

  using CoreLibrary = LibraryRC<CoreLibrarySetup>;

}  // namespace ncc::core

#endif  // __NITRATE_CORE_LIB_H__
