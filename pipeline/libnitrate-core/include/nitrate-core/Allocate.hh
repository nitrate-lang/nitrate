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
    class NCC_EXPORT Iterator {
      friend class DynamicArena;

      DynamicArena *m_arena;
      size_t m_chunk_index;
      uint8_t *m_chunk_pos, *m_chunk_end;

      Iterator(DynamicArena *arena, size_t chunk_index, uint8_t *chunk_pos, uint8_t *chunk_end)
          : m_arena(arena), m_chunk_index(chunk_index), m_chunk_pos(chunk_pos), m_chunk_end(chunk_end) {}

    public:
      auto operator++() -> Iterator &;
      [[nodiscard]] auto operator*() const -> uint8_t &;
      [[nodiscard]] auto operator==(const Iterator &o) const -> bool;
    };

    class NCC_EXPORT ConstIterator {
      friend class DynamicArena;

      const DynamicArena *m_arena;
      size_t m_chunk_index;
      const uint8_t *m_chunk_pos, *m_chunk_end;

      ConstIterator(const DynamicArena *arena, size_t chunk_index, const uint8_t *chunk_pos, const uint8_t *chunk_end)
          : m_arena(arena), m_chunk_index(chunk_index), m_chunk_pos(chunk_pos), m_chunk_end(chunk_end) {}

    public:
      auto operator++() -> ConstIterator &;
      [[nodiscard]] auto operator*() const -> const uint8_t &;
      [[nodiscard]] auto operator==(const ConstIterator &o) const -> bool;
    };

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
    [[nodiscard]] auto do_is_equal(const memory_resource &other) const noexcept -> bool override;
    [[nodiscard]] auto GetSpaceUsed() const -> size_t;
    [[nodiscard]] auto GetSpaceManaged() const -> size_t;
    void Reset();

    [[nodiscard]] auto begin() -> Iterator;              // NOLINT(readability-identifier-naming)
    [[nodiscard]] auto end() -> Iterator;                // NOLINT(readability-identifier-naming)
    [[nodiscard]] auto cbegin() const -> ConstIterator;  // NOLINT(readability-identifier-naming)
    [[nodiscard]] auto cend() const -> ConstIterator;    // NOLINT(readability-identifier-naming)
  };
}  // namespace ncc

#endif
