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

namespace ncc {
  class IMemory {
  public:
    virtual ~IMemory() = default;

    /**
     * @brief Allocates a block of memory.
     *  @param size The size of the block to allocate. Values of 0 are permitted
     *  as long as the returned address isn't read from or written to.
     *  @param align The alignment of the block to allocate. A Value of 0 will
     * return nullptr.
     *  @return A pointer to the allocated block of memory. If the allocation
     * fails, a panic will occur. The returned address is guaranteed to be
     * aligned to the specified alignment except when the alignment is 0.
     * @note nullptr is only returned when the alignment is 0.
     */
    virtual auto Alloc(size_t size, size_t align = kDefaultAlignment) -> void * = 0;

    static constexpr size_t kDefaultAlignment = 16;
  };

  class DynamicArena final : public IMemory {
    class PImpl;
    PImpl *m_pimpl;

  public:
    DynamicArena();
    DynamicArena(const DynamicArena &) = delete;
    ~DynamicArena() override;

    DynamicArena(DynamicArena &&o) noexcept {
      m_pimpl = o.m_pimpl;
      o.m_pimpl = nullptr;
    }

    auto operator=(DynamicArena &&o) noexcept -> DynamicArena & {
      m_pimpl = o.m_pimpl;
      o.m_pimpl = nullptr;
      return *this;
    }

    auto Alloc(size_t size, size_t align = kDefaultAlignment) -> void * override;
  };
}  // namespace ncc

#endif
