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

#include <alloc/Collection.hh>
#include <cstring>
#include <nitrate-core/Logger.hh>
#include <vector>

using namespace ncc;

static constexpr auto kSegmentSize = 1024 * 16;

static inline uint8_t *ALIGNED(uint8_t *ptr, size_t align) {
  auto addr_int = reinterpret_cast<uintptr_t>(ptr);
  return (addr_int % align != 0) ? (ptr + (align - (addr_int % align))) : ptr;
}

DynamicArena::PImpl::PImpl() { AllocRegion(kSegmentSize); }

DynamicArena::PImpl::~PImpl() {
  for (auto &base : m_bases) {
    delete[] base.m_base;
  }
}

void *DynamicArena::PImpl::Alloc(size_t size, size_t alignment) {
  if (size == 0 || alignment == 0) {
    return nullptr;
  }

  m_mutex.lock();

  if (size > kSegmentSize) [[unlikely]] {
    AllocRegion(size);
  }

  auto &b = m_bases.back();

  auto *start = ALIGNED(b.m_offset, alignment);

  if ((start + size) <= b.m_base + b.m_size) [[likely]] {
    b.m_offset = start + size;
  } else {
    AllocRegion(kSegmentSize);

    start = ALIGNED(m_bases.back().m_offset, alignment);
    if ((start + size) > m_bases.back().m_base + m_bases.back().m_size)
        [[unlikely]] {
      qcore_panicf(
          "Out of memory: failed to allocate %zu bytes @ alignment %zu", size,
          alignment);
    }

    m_bases.back().m_offset = start + size;
  }

  m_mutex.unlock();

  return start;
}
