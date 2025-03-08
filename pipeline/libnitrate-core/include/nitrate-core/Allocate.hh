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

#ifndef __NITRATE_CORE_MEMORY_H__
#define __NITRATE_CORE_MEMORY_H__

#include <cstddef>
#include <memory_resource>
#include <nitrate-core/AllocateFwd.hh>
#include <nitrate-core/Macro.hh>

namespace ncc {
  class NCC_EXPORT DynamicArena final : public std::pmr::memory_resource {
    class PImpl;
    PImpl *m_pimpl;

  public:
    DynamicArena();
    DynamicArena(const DynamicArena &) = delete;
    DynamicArena(DynamicArena &&o) noexcept : m_pimpl(o.m_pimpl) { o.m_pimpl = nullptr; }
    ~DynamicArena() override;

    auto operator=(DynamicArena &&o) noexcept -> DynamicArena & {
      m_pimpl = o.m_pimpl;
      o.m_pimpl = nullptr;
      return *this;
    }

    void do_deallocate(void *p, size_t bytes, size_t alignment) override;
    [[nodiscard]] auto do_allocate(size_t bytes, size_t alignment) -> void * override;
    [[nodiscard]] bool do_is_equal(const memory_resource &other) const noexcept override;
    [[nodiscard]] auto GetSpaceUsed() const -> size_t;
    [[nodiscard]] auto GetSpaceManaged() const -> size_t;
    void Reset();
  };
}  // namespace ncc

#endif
