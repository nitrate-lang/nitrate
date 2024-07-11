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

#define UNIT_FACTOR 4

LIB_EXPORT quixcc_arena_t *quixcc_arena_open(quixcc_arena_t *arena) {
#if !defined(NDEBUG)
  if (!arena) {
    quixcc_panic("quixcc_arena_open: arena is NULL");
    __builtin_unreachable();
  }
#endif

  arena->m_base = NULL;
  arena->m_offset = NULL;
  arena->m_size = 0;

  return arena;
}

LIB_EXPORT void *quixcc_arena_alloc(quixcc_arena_t *arena, size_t size) {
  void *ptr, *new_base;
  size_t new_size;

#if !defined(NDEBUG)
  if (!arena) {
    quixcc_panic("quixcc_arena_alloc: arena is NULL");
    __builtin_unreachable();
  }
#endif

  if (size + (uintptr_t)arena->m_offset >
      (uintptr_t)arena->m_base + arena->m_size) {
    new_size = arena->m_size + (size * UNIT_FACTOR);

    new_base = realloc(arena->m_base, new_size);
    if (!new_base) {
      if (errno == ENOMEM) {
        quixcc_panic("quixcc_arena_alloc: out of memory");
      }

      quixcc_panic("quixcc_arena_alloc: realloc() failed to allocate memory");

      __builtin_unreachable();
    }

    arena->m_base = new_base;
    arena->m_offset = (void *)((uintptr_t)arena->m_base + arena->m_size);
    arena->m_size = new_size;
  }

  ptr = arena->m_offset;
  arena->m_offset = (void *)((uintptr_t)arena->m_offset + size);

  return ptr;
}

LIB_EXPORT void quixcc_arena_close(quixcc_arena_t *arena) {
#if !defined(NDEBUG)
  if (!arena) {
    quixcc_panic("quixcc_arena_close: arena is NULL");
    __builtin_unreachable();
  }
#endif

  if (!arena->m_size) {
    return;
  }

  free(arena->m_base);

  arena->m_base = NULL;
  arena->m_offset = NULL;
  arena->m_size = 0;
}
