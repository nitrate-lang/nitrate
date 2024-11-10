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

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define QCORE_ALLOC_ALIGN_DEFAULT 16

///=============================================================================

typedef enum {
  QCORE_GBA_V0 = 0,  /* General-purpose bump allocator: [fastest, wastes memory with alignment] */
  QCORE_RIBA_V0 = 1, /* Reverse-iteration bump allocator: [fast, no waste] */

  QCORE_AUTO = 1000, /* Any arena allocator */

  QCORE_GBA = QCORE_GBA_V0,
  QCORE_RIBA = QCORE_RIBA_V0,
} qcore_alloc_mode_t;

typedef uintptr_t qcore_arena_t;

///=============================================================================

/**
 * @brief Open a bump allocator specified by the mode.
 * @param A The context to open the bump allocator.
 * @param mode Type of bump allocator to open.
 * @param is_thread_safe Whether the bump allocator is thread-safe.
 * @return The context of the opened bump allocator.
 *
 * @note This function is thread-safe.
 */
qcore_arena_t *qcore_arena_open_ex(qcore_arena_t *A, qcore_alloc_mode_t mode, bool is_thread_safe);

/**
 * @brief Open a QCORE_AUTO bump allocator.
 * @param A The context to open the bump allocator.
 * @return The context of the opened bump allocator.
 *
 * @warning The returned bump allocator is NOT thread-safe.
 * @note This function is thread-safe.
 */
static inline qcore_arena_t *qcore_arena_open(qcore_arena_t *A) {
  return qcore_arena_open_ex(A, QCORE_AUTO, false);
}

/**
 * @brief Close the bump allocator.
 * @param A The context of the bump allocator to close.
 *
 * @note This function is thread-safe.
 */
void qcore_arena_close(qcore_arena_t *A);

/**
 * @brief Allocate memory from the bump allocator.
 * @param A The context of the bump allocator.
 * @param size The size of the memory to allocate.
 * @param align The alignment of the memory to allocate.
 * @return The allocated memory.
 *
 * @note This function is thread-safe.
 */
void *qcore_arena_alloc_ex(qcore_arena_t *A, size_t size, size_t align);

/**
 * @brief Allocate memory from the bump allocator with default alignment.
 * @param A The context of the bump allocator.
 * @param size The size of the memory to allocate.
 * @return The allocated memory.
 *
 * @note This function is thread-safe.
 */
static inline void *qcore_arena_alloc(qcore_arena_t *A, size_t size) {
  return qcore_arena_alloc_ex(A, size, QCORE_ALLOC_ALIGN_DEFAULT);
}

#ifdef __cplusplus
}
#endif

#endif
