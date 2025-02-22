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

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace ncc {
  template <typename Impl>
  class LibraryRC;

  class LibraryRCAutoClose final {
    std::function<void()> m_inc, m_dec;

  public:
    LibraryRCAutoClose(std::function<void()> inc, std::function<void()> dec)
        : m_inc(std::move(inc)), m_dec(std::move(dec)) {}

    LibraryRCAutoClose(const LibraryRCAutoClose& o) : m_inc(o.m_inc), m_dec(o.m_dec) {
      if (m_inc) {
        m_inc();
      }
    }

    LibraryRCAutoClose(LibraryRCAutoClose&& o) noexcept : m_inc(std::move(o.m_inc)), m_dec(std::move(o.m_dec)) {}

    ~LibraryRCAutoClose() {
      if (m_dec) {
        m_dec();
      }
    }
  };

  template <typename Impl>
  class LibraryRC final {
    size_t m_ref_count{};
    std::recursive_mutex m_ref_count_mutex;

  public:
    LibraryRC() = default;

    auto InitRC() -> bool {
      std::lock_guard<std::recursive_mutex> lock(m_ref_count_mutex);

      if (m_ref_count++ == 0) {
        if (!Impl::Init()) {
          m_ref_count = 0;
          return false;
        }
      }

      return true;
    }

    void DeinitRC() {
      std::lock_guard<std::recursive_mutex> lock(m_ref_count_mutex);

      if (m_ref_count > 0 && --m_ref_count == 0) {
        Impl::Deinit();
      }
    }

    auto IsInitialized() -> bool {
      std::lock_guard<std::recursive_mutex> lock(m_ref_count_mutex);

      return m_ref_count > 0;
    }

    auto GetVersion() -> std::string_view {
      static std::string version_string =

          std::string("[") + std::string(Impl::GetVersionId()) +
          std::string(
              "] ["

#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) || defined(_M_AMD64)
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

    auto GetRC() -> std::optional<LibraryRCAutoClose> {
      if (!InitRC()) {
        return std::nullopt;
      }

      return LibraryRCAutoClose([this] { InitRC(); }, [this] { DeinitRC(); });
    }
  };

  struct CoreLibrarySetup final {
    static auto Init() -> bool;
    static void Deinit();
    static auto GetVersionId() -> std::string_view;
  };

  extern LibraryRC<CoreLibrarySetup> CoreLibrary;
}  // namespace ncc

#endif  // __NITRATE_CORE_LIB_H__
