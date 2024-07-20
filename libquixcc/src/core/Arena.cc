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

#include <core/Macro.h>
#include <errno.h>
#include <quixcc/Library.h>
#include <quixcc/core/Arena.h>
#include <stdint.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>
#include <vector>

#define ALLOCATOR_PRINT_VERBOSE_CONF 1

#if ALLOCATOR_PRINT_VERBOSE_CONF && !defined(NDEBUG)
#define ALLOCATOR_PRINT_VERBOSE 1
#else
#define ALLOCATOR_PRINT_VERBOSE 0
#endif

#define REGION_SIZE (1024 * 16)

static inline uintptr_t ALIGNED(uintptr_t ptr, size_t align) {
  if (ptr % align) {
    return ptr + (align - (ptr % align));
  }

  return ptr;
}

class quixcc_arena_impl_t {
  [[noreturn]] void alloc_failed() {
    if (errno == ENOMEM) {
      quixcc_panic("quixcc_arena_alloc: out of memory");
    }

    quixcc_panic("quixcc_arena_alloc: failed to allocate memory");
  }

  struct region_t {
    void *m_base;
    void *m_offset;
    size_t m_size;

    region_t(void *base = nullptr, void *offset = nullptr, size_t size = 0)
        : m_base(base), m_offset(offset), m_size(size) {}
  };

  void alloc_region(size_t size) {
    void *base = malloc(size);

    if (!base) {
      alloc_failed();
    }

    m_bases.push_back({base, base, size});
  }

  public:
  quixcc_arena_impl_t() = default;

  quixcc_arena_impl_t(const quixcc_arena_impl_t &other) {
    /// TODO: Verify this implementation

    size_t used = 0;

    for (const auto &region : other.m_bases) {
      /* Allocate a new memory block */
      void *base = malloc(region.m_size);

      /* If the allocation failed, panic */
      if (!base) {
        alloc_failed();
      }

      /* Calculate the amount of memory used in the old block */
      used = (uintptr_t)region.m_offset - (uintptr_t)region.m_base;

      /* Only copy the used memory */
      std::memcpy(base, region.m_base, used);

      /* Push the new memory block */
      m_bases.push_back({base, (void *)((uintptr_t)base + used), region.m_size});
    }
  }

  ~quixcc_arena_impl_t() {
#if ALLOCATOR_PRINT_VERBOSE
    size_t total = 0;

    std::cerr << "TRACE: quixcc_arena_impl_t::~quixcc_arena_impl_t() -> free([";
#endif

    for (size_t i = 0; i < m_bases.size(); i++) {
#if !defined(NDEBUG)
      memset(m_bases[i].m_base, 'Q', m_bases[i].m_size);
#endif

      free(m_bases[i].m_base);

#if ALLOCATOR_PRINT_VERBOSE
      total += m_bases[i].m_size;

      std::cerr << m_bases[i].m_size;
      if (i + 1 < m_bases.size()) {
        std::cerr << ", ";
      }
#endif
    }

#if ALLOCATOR_PRINT_VERBOSE
    std::cerr << "]) -> " << total << " freed total\n";
#endif
  }

  void *allocate(size_t size, size_t alignment) {
    if (size == 0 || alignment == 0) {
      return nullptr;
    }

    if (size > REGION_SIZE) {
      alloc_region(size);
    }

    { /* Initial region allocation */
      if (m_bases.empty()) {
        alloc_region(REGION_SIZE);
      }
    }

    for (auto it = m_bases.rbegin(); it != m_bases.rend(); ++it) {
      /* Calculate boundaries for hypothetical allocation */
      uintptr_t start = ALIGNED((uintptr_t)it->m_offset, alignment);

      /* Check if the region has enough space */
      if ((start + size) <= (uintptr_t)it->m_base + it->m_size) {
        it->m_offset = (void *)(start + size);
        return (void *)start;
      }
    }

    /* If no region has enough space, allocate a new one */
    alloc_region(REGION_SIZE);

    /* Calculate the start of the allocation */
    uintptr_t start = ALIGNED((uintptr_t)m_bases.back().m_offset, alignment);

    m_bases.back().m_offset = (void *)(start + size);

    /* Return the first region */
    return (void *)start;
  }

  std::vector<region_t> m_bases;
};

LIB_EXPORT quixcc_arena_t *quixcc_arena_open(quixcc_arena_t *arena) {
#if ALLOCATOR_PRINT_VERBOSE
  std::cerr << "TRACE: quixcc_arena_open(" << arena << ")\n";
#endif

  arena->m_impl = new quixcc_arena_impl_t();
  return arena;
}

LIB_EXPORT quixcc_arena_t *quixcc_arena_copy(quixcc_arena_t *dst, const quixcc_arena_t *src) {
#if ALLOCATOR_PRINT_VERBOSE
  std::cerr << "TRACE: quixcc_arena_copy(" << dst << ", " << src << ")\n";
#endif

  dst->m_impl = new quixcc_arena_impl_t(*src->m_impl);
  return dst;
}

LIB_EXPORT void *quixcc_arena_alloc(quixcc_arena_t *arena, size_t size) {
  void *ptr = arena->m_impl->allocate(size, 1);

#if ALLOCATOR_PRINT_VERBOSE
  std::cerr << "TRACE: quixcc_arena_alloc(" << size << ")\t-> " << ptr << "\n";
#endif

  return ptr;
}

LIB_EXPORT void *quixcc_arena_alloc_ex(quixcc_arena_t *arena, size_t size, size_t align) {
  void *ptr = arena->m_impl->allocate(size, align);

#if ALLOCATOR_PRINT_VERBOSE
  std::cerr << "TRACE: quixcc_arena_alloc_ex(" << size << ", " << align << ")\t-> " << ptr << "\n";
#endif

  return ptr;
}

LIB_EXPORT size_t quixcc_arena_capacity(quixcc_arena_t *arena) {
  size_t total = 0;

  for (const auto &region : arena->m_impl->m_bases) {
    total += region.m_size;
  }

#if ALLOCATOR_PRINT_VERBOSE
  std::cerr << "TRACE: quixcc_arena_capacity(" << arena << ")\t-> " << total << "\n";
#endif

  return total;
}

LIB_EXPORT size_t quixcc_arena_used(quixcc_arena_t *arena) {
  size_t total = 0;

  for (const auto &region : arena->m_impl->m_bases) {
    total += (uintptr_t)region.m_offset - (uintptr_t)region.m_base;
  }

#if ALLOCATOR_PRINT_VERBOSE
  std::cerr << "TRACE: quixcc_arena_used(" << arena << ")\t-> " << total << "\n";
#endif

  return total;
}

LIB_EXPORT void quixcc_arena_close(quixcc_arena_t *arena) {
#if ALLOCATOR_PRINT_VERBOSE
  if (arena->m_impl) {
    std::cerr << "TRACE: quixcc_arena_close(" << arena << ")\n";
  } else {
    std::cerr << "TRACE: quixcc_arena_close(" << arena << ") -> nop\n";
  }
#endif

  delete arena->m_impl;
  arena->m_impl = nullptr;
}
