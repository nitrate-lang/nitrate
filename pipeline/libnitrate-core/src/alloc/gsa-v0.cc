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

#include <stdint.h>
#include <stdlib.h>

#include <alloc/Collection.hh>
#include <cstring>
#include <nitrate-core/Logger.hh>
#include <vector>

#define REGION_SIZE (1024 * 16)

static inline uintptr_t ALIGNED(uintptr_t ptr, size_t align) {
  return (ptr % align) ? (ptr + (align - (ptr % align))) : ptr;
}

void mem::gba_v0_t::open(bool thread_safe) {
  m_thread_safe = thread_safe;
  alloc_region(REGION_SIZE);
}

size_t mem::gba_v0_t::close() {
  size_t total = 0;

  for (size_t i = 0; i < m_bases.size(); i++) {
    total += m_bases[i].size;
    delete[] reinterpret_cast<uint8_t *>(m_bases[i].base);
  }

  return total;
}

void *mem::gba_v0_t::alloc(size_t size, size_t alignment) {
  if (size == 0 || alignment == 0) {
    return nullptr;
  }

  if (m_thread_safe) {
    m_mutex.lock();
  }

  uintptr_t start;

  if (size > REGION_SIZE) [[unlikely]] {
    alloc_region(size);
  }

  auto &R = m_bases.back();

  start = ALIGNED(R.offset, alignment);

  if ((start + size) <= R.base + R.size) [[likely]] {
    R.offset = start + size;
  } else {
    alloc_region(REGION_SIZE);

    start = ALIGNED(m_bases.back().offset, alignment);
    if ((start + size) > m_bases.back().base + m_bases.back().size)
        [[unlikely]] {
      qcore_panicf(
          "Out of memory: failed to allocate %zu bytes @ alignment %zu", size,
          alignment);
    }

    m_bases.back().offset = start + size;
  }

  if (m_thread_safe) {
    m_mutex.unlock();
  }

  return reinterpret_cast<void *>(start);
}
