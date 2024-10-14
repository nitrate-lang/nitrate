////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QCORE_NDEBUG

#include <errno.h>
#include <quix-core/Arena.h>
#include <quix-core/Error.h>
#include <stdint.h>
#include <stdlib.h>

#include <cstring>
#include <numeric>
#include <vector>

#include "LibMacro.h"

#define REGION_SIZE (1024 * 16)

static inline uintptr_t ALIGNED(uintptr_t ptr, size_t align) {
  return (ptr % align) ? (ptr + (align - (ptr % align))) : ptr;
}

struct qcore_arena_impl_t {
private:
  [[noreturn]] void alloc_failed() {
    if (errno == ENOMEM) {
      qcore_panic("qcore_arena_alloc: out of memory");
    } else {
      qcore_panic("qcore_arena_alloc: failed to allocate memory");
    }
  }

  struct region_t {
    void *m_base;
    void *m_offset;
    size_t m_size;

    region_t(void *base, void *offset, size_t size)
        : m_base(base), m_offset(offset), m_size(size) {}
  };

  void alloc_region(size_t size) {
    void *base = new uint8_t[size];

    if (!base) {
      alloc_failed();
    }

    m_bases.push_back({base, base, size});
  }

public:
  qcore_arena_impl_t() = default;

  qcore_arena_impl_t(const qcore_arena_impl_t &other) {
    qcore_debug("TRACE: qcore_arena_impl_t::qcore_arena_impl_t(const qcore_arena_impl_t &)\n");

    size_t used = 0;

    for (const auto &region : other.m_bases) {
      void *base = new uint8_t[region.m_size];

      if (!base) {
        alloc_failed();
      }

      /* Calculate the amount of memory used in the old block */
      used =
          reinterpret_cast<uintptr_t>(region.m_offset) - reinterpret_cast<uintptr_t>(region.m_base);

      /* Only copy the used memory */
      std::memcpy(base, region.m_base, used);

      /* Push the new memory block */
      m_bases.push_back({base, (void *)((uintptr_t)base + used), region.m_size});
    }
  }

  ~qcore_arena_impl_t() {
    for (size_t i = 0; i < m_bases.size(); i++) {
      delete[] static_cast<uint8_t *>(m_bases[i].m_base);
    }

    qcore_debugf("TRACE: qcore_arena_impl_t::~qcore_arena_impl_t() -> free()'d total %zu\n",
                 std::accumulate(
                     m_bases.begin(), m_bases.end(), 0,
                     [](size_t total, const region_t &region) { return total + region.m_size; }));
  }

  void *allocate(size_t size, size_t alignment) {
    uintptr_t start;

    if (size == 0 || alignment == 0) {
      return nullptr;
    }

    if (size > REGION_SIZE) {
      alloc_region(size);
    } else if (m_bases.empty()) {
      alloc_region(REGION_SIZE);
    }

    for (auto it = m_bases.rbegin(); it != m_bases.rend(); ++it) {
      /* Calculate boundaries for hypothetical allocation */
      start = ALIGNED((uintptr_t)it->m_offset, alignment);

      /* Check if the region has enough space */
      if ((start + size) <= (uintptr_t)it->m_base + it->m_size) {
        it->m_offset = (void *)(start + size);
        return (void *)start;
      }
    }

    /* If no region has enough space, allocate a new one */
    alloc_region(REGION_SIZE);

    /* Calculate the start of the allocation */
    start = ALIGNED((uintptr_t)m_bases.back().m_offset, alignment);

    m_bases.back().m_offset = (void *)(start + size);

    /* Return the first region */
    return (void *)start;
  }

  std::vector<region_t> m_bases;
};

LIB_EXPORT qcore_arena_t *qcore_arena_open(qcore_arena_t *arena) {
  qcore_assert(arena, "qcore_arena_open: arena is null");
  qcore_debugf("TRACE: qcore_arena_open(%p)\n", arena);

  arena->m_impl = new qcore_arena_impl_t();

  return arena;
}

LIB_EXPORT qcore_arena_t *qcore_arena_copy(qcore_arena_t *dst, const qcore_arena_t *src) {
  qcore_assert(src && dst, "qcore_arena_copy: src or dst is null");
  qcore_debugf("TRACE: qcore_arena_copy(%p, %p)\n", dst->m_impl, src->m_impl);

  dst->m_impl = new qcore_arena_impl_t(*src->m_impl);

  return dst;
}

LIB_EXPORT void *qcore_arena_alloc(qcore_arena_t *arena, size_t size) {
  void *ptr;

  qcore_assert(arena, "qcore_arena_alloc: arena is null");

  ptr = arena->m_impl->allocate(size, 1);
  qcore_debugf("TRACE: qcore_arena_alloc(%p, %zu)\t-> %p\n", arena->m_impl, size, ptr);

  return ptr;
}

LIB_EXPORT void *qcore_arena_alloc_ex(qcore_arena_t *arena, size_t size, size_t align) {
  void *ptr;

  qcore_assert(arena, "qcore_arena_alloc_ex: arena is null");

  ptr = arena->m_impl->allocate(size, align);
  qcore_debugf("TRACE: qcore_arena_alloc_ex(%p, %zu, %zu)\t-> %p\n", arena->m_impl, size, align,
               ptr);

  return ptr;
}

LIB_EXPORT size_t qcore_arena_capacity(qcore_arena_t *arena) {
  qcore_assert(arena, "qcore_arena_capacity: arena is null");

  size_t total =
      std::accumulate(arena->m_impl->m_bases.begin(), arena->m_impl->m_bases.end(), 0,
                      [](size_t total, const auto &region) { return total + region.m_size; });

  qcore_debugf("TRACE: qcore_arena_capacity(%p)\t-> %zu\n", arena->m_impl, total);

  return total;
}

LIB_EXPORT size_t qcore_arena_used(qcore_arena_t *arena) {
  qcore_assert(arena, "qcore_arena_used: arena is null");

  size_t total =
      std::accumulate(arena->m_impl->m_bases.begin(), arena->m_impl->m_bases.end(), 0,
                      [](size_t total, const auto &region) {
                        return total + (uintptr_t)region.m_offset - (uintptr_t)region.m_base;
                      });

  qcore_debugf("TRACE: qcore_arena_used(%p)\t-> %zu\n", arena->m_impl, total);

  return total;
}

LIB_EXPORT void qcore_arena_reset(qcore_arena_t *arena) {
  qcore_assert(arena, "qcore_arena_reset: arena is null");

  for (auto &region : arena->m_impl->m_bases) {
    region.m_offset = region.m_base;
  }

  qcore_debugf("TRACE: qcore_arena_reset(%p)\n", arena->m_impl);
}

LIB_EXPORT void qcore_arena_close(qcore_arena_t *arena) {
  qcore_assert(arena, "qcore_arena_close: arena is null");

  if (arena->m_impl) {
    qcore_debugf("TRACE: qcore_arena_close(%p)\n", arena->m_impl);
  } else {
    qcore_debugf("TRACE: qcore_arena_close(%p) -> nop\n", arena->m_impl);
  }

  delete arena->m_impl;
  arena->m_impl = nullptr;
}
