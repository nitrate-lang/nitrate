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
    mutable std::recursive_mutex m_ref_count_mutex;

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

    auto IsInitialized() const -> bool {
      std::lock_guard<std::recursive_mutex> lock(m_ref_count_mutex);

      return m_ref_count > 0;
    }

    auto GetMajorVersion() const -> uint32_t { return Impl::GetSemVersion()[0]; }
    auto GetMinorVersion() const -> uint32_t { return Impl::GetSemVersion()[1]; }
    auto GetPatchVersion() const -> uint32_t { return Impl::GetSemVersion()[2]; }
    auto GetSemVersion() const -> std::array<uint32_t, 3> { return Impl::GetSemVersion(); }
    auto GetCommitHash() const -> std::string_view { return Impl::BuildId().m_commit; }
    auto GetCompileDate() const -> std::string_view { return Impl::BuildId().m_iso8601; }
    auto GetBranch() const -> std::string_view { return Impl::BuildId().m_branch; }

    auto GetRC() -> std::optional<LibraryRCAutoClose> {
      if (!InitRC()) {
        return std::nullopt;
      }

      return LibraryRCAutoClose([this] { InitRC(); }, [this] { DeinitRC(); });
    }
  };

  struct BuildId final {
    std::string_view m_commit;
    std::string_view m_iso8601;
    std::string_view m_branch;
  };

  struct CoreLibrarySetup final {
    static auto Init() -> bool;
    static void Deinit();
    static auto GetSemVersion() -> std::array<uint32_t, 3>;
    static auto BuildId() -> BuildId;
  };

  extern LibraryRC<CoreLibrarySetup> CoreLibrary;
}  // namespace ncc

#endif  // __NITRATE_CORE_LIB_H__
