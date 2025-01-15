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

#include <cstring>
#include <mutex>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <vector>

using namespace ncc;
using namespace ncc;

static constexpr auto kPrimarySegmentSize = 1024 * 16;

static NCC_FORCE_INLINE uint8_t *ALIGNED(uint8_t *ptr, size_t align) {
  size_t mod = reinterpret_cast<uintptr_t>(ptr) % align;
  return mod == 0 ? ptr : (ptr + (align - mod));
}

struct Segment {
  uint8_t *m_base = nullptr, *m_offset = nullptr;
  size_t m_size = 0;
};

class ncc::DynamicArena::PImpl final {
  std::vector<Segment> m_bases;
  std::mutex m_mutex;

  void AllocRegion(size_t size) {
    auto *base = new uint8_t[size];
    m_bases.push_back({base, base, size});
  }

public:
  PImpl() { AllocRegion(kPrimarySegmentSize); }

  ~PImpl() {
    for (auto &base : m_bases) {
      delete[] base.m_base;
    }
  }

  void *Alloc(size_t size, size_t alignment) {
    if (size == 0 || alignment == 0) [[unlikely]] {
      return nullptr;
    }

    bool sync = EnableSync;
    if (sync) {
      m_mutex.lock();
    }

    // Put large allocations in their own segment
    if (size + alignment > kPrimarySegmentSize) [[unlikely]] {
      AllocRegion(size);
    }

    auto &b = m_bases.back();
    auto *start = ALIGNED(b.m_offset, alignment);

    // Check if we can allocate in the current segment
    if ((start + size) <= b.m_base + b.m_size) [[likely]] {
      b.m_offset = start + size;
    } else {
      AllocRegion(kPrimarySegmentSize);

      start = ALIGNED(m_bases.back().m_offset, alignment);

      // I hope my math is right
      qcore_assert((start + size) <=
                   m_bases.back().m_base + m_bases.back().m_size);

      m_bases.back().m_offset = start + size;
    }

    if (sync) {
      m_mutex.unlock();
    }

    return start;
  }
};

NCC_EXPORT DynamicArena::DynamicArena() {
  m_arena = new PImpl();
  m_owned = true;
}

NCC_EXPORT DynamicArena::~DynamicArena() {
  if (m_owned) {
    delete m_arena;
  }
}

NCC_EXPORT void *DynamicArena::Alloc(size_t size, size_t alignment) {
  return m_arena->Alloc(size, alignment);
}
