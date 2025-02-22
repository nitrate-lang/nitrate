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

#include <mutex>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/SmartLock.hh>
#include <vector>

using namespace ncc;

static constexpr size_t kPrimarySegmentSize = 1024 * 16;

static NCC_FORCE_INLINE auto ALIGNED(uint8_t *ptr, size_t align) -> uint8_t * {
  size_t mod = reinterpret_cast<uintptr_t>(ptr) % align;
  return mod == 0 ? ptr : (ptr + (align - mod));
}

struct Segment {
  uint8_t *m_base = nullptr, *m_offset = nullptr;
  size_t m_size = 0;
};

class ncc::DynamicArena::PImpl final {
  std::vector<Segment> m_bases;
  mutable std::mutex m_mutex;

  void AllocRegion(size_t size) {
    auto *base = new uint8_t[size];
    m_bases.push_back({base, base, size});
  }

public:
  PImpl() {
    m_bases.reserve(64);

    AllocRegion(kPrimarySegmentSize);
  }

  ~PImpl() {
    for (auto &base : m_bases) {
      delete[] base.m_base;
    }
  }

  auto Allocate(size_t size, size_t alignment) -> void * {
    /* Sizeof 0 is permitted as long the the returned address isn't read from or
     * written to. */

    uint8_t *start;
    Segment *b;

    SmartLock lock(m_mutex);

    /* If the requested size plus the alignment is greater than the primary
     * segment size, then allocate a new segment to store the payload. */
    if (size + alignment > kPrimarySegmentSize) [[unlikely]] {
      AllocRegion(size + alignment);
      b = &m_bases.back();
      start = ALIGNED(b->m_offset, alignment);
    } else {
      /* Otherwise, prepare to allocate in the current segment. */
      b = &m_bases.back();
      start = ALIGNED(b->m_offset, alignment);

      /* If the requested size is greater than the remaining space in the
       * current segment, allocate a new primary size segment to store the
       * payload. */
      if (start + size > b->m_base + b->m_size) [[unlikely]] {
        AllocRegion(kPrimarySegmentSize);
        b = &m_bases.back();

        start = ALIGNED(b->m_offset, alignment);
      }
    }

    b->m_offset = start + size;

    /* Ensure that the returned space is within the bounds of the current
     * segment. */
    qcore_assert((start + size) <= b->m_base + b->m_size);

    return start;
  }

  auto GetSpaceUsed() const -> size_t {
    SmartLock lock(m_mutex);

    size_t total = 0;
    for (const auto &base : m_bases) {
      total += base.m_offset - base.m_base;
    }

    return total;
  }

  auto GetSpaceManaged() const -> size_t {
    SmartLock lock(m_mutex);

    size_t total = 0;
    for (const auto &base : m_bases) {
      total += base.m_size;
    }

    total += m_bases.capacity() * sizeof(Segment);
    total += sizeof(*this);

    return total;
  }

  auto Reset() -> void {
    SmartLock lock(m_mutex);

    for (auto &base : m_bases) {
      delete[] base.m_base;
    }

    m_bases.clear();

    AllocRegion(kPrimarySegmentSize);
  }
};

DynamicArena::DynamicArena() { m_pimpl = new PImpl(); }
DynamicArena::~DynamicArena() { delete m_pimpl; }
auto DynamicArena::GetSpaceUsed() const -> size_t { return m_pimpl->GetSpaceUsed(); }
auto DynamicArena::GetSpaceManaged() const -> size_t { return m_pimpl->GetSpaceManaged(); }
void DynamicArena::Reset() { m_pimpl->Reset(); }

void *DynamicArena::do_allocate(size_t bytes, size_t alignment) { return m_pimpl->Allocate(bytes, alignment); }

void DynamicArena::do_deallocate(void *p, size_t bytes, size_t alignment) {
  (void)p;
  (void)bytes;
  (void)alignment;
}

bool DynamicArena::do_is_equal(const memory_resource &other) const noexcept { return this == &other; }
