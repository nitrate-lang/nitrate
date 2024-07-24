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

#ifndef __QUIX_CORE_ARENA_H__
#define __QUIX_CORE_ARENA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct qcore_arena_impl_t;

/**
 * @brief Quixcc general-purpose arena allocator.
 */
typedef struct qcore_arena_t {
  struct qcore_arena_impl_t *m_impl;
} qcore_arena_t;

/**
 * @brief Initializes an arena allocator.
 *
 * @param arena The arena to initialize.
 *
 * @return The initialized arena.
 *
 * @note If the arena is currently open, it will be reset and all of its memory
 * will be LEAKED.
 * @note This function is thread-safe.
 */
qcore_arena_t *qcore_arena_open(qcore_arena_t *arena);

/**
 * @brief Copy an arena allocator.
 *
 * @param dst The arena to copy to.
 * @param src The arena to copy from.
 *
 * @return The copied arena.
 *
 * @warning This is a shallow byte-by-byte copy.
 * @note Note that the copied arena is a new instance and must be closed separately.
 * @note This function is thread-safe.
 */
qcore_arena_t *qcore_arena_copy(qcore_arena_t *dst, const qcore_arena_t *src);

/**
 * @brief Allocates memory from the arena.
 *
 * @param arena The arena to allocate memory from.
 * @param size The size of the memory to allocate.
 *
 * @return A pointer to the allocated memory.
 *
 * @note The arena is resized automatically when needed.
 * @note Out-of-memory errors will panic.
 * @note The returned pointer is never NULL.
 * @note The returned memory is not initialized.
 * @warning DO NOT free the returned memory manually.
 * @note This function is thread-safe.
 *
 * @warning All pointers are owned by the arena and will be freed
 * when the arena is closed.
 */
void *qcore_arena_alloc(qcore_arena_t *arena, size_t size);

/**
 * @brief Allocates memory from the arena with a specific alignment.
 *
 * @param arena The arena to allocate memory from.
 * @param size The size of the memory to allocate.
 * @param align The alignment of the memory to allocate.
 *
 * @return A pointer to the allocated memory.
 *
 * @note The arena is resized automatically when needed.
 * @note Out-of-memory errors will panic.
 * @note The returned pointer is never NULL.
 * @note The returned memory is not initialized.
 * @warning DO NOT free the returned memory manually.
 * @note This function is thread-safe.
 *
 * @warning All pointers are owned by the arena and will be freed
 * when the arena is closed.
 */
void *qcore_arena_alloc_ex(qcore_arena_t *arena, size_t size, size_t align);

/**
 * @brief Returns the total memory usage of the arena.
 *
 * @param arena The arena to get the size of.
 *
 * @return The total size of the arena.
 *
 * @note This function is thread-safe.
 */
size_t qcore_arena_capacity(qcore_arena_t *arena);

/**
 * @brief Returns the total size of all allocated memory from the arena.
 *
 * @param arena The arena to get the used memory of.
 *
 * @return The amount of memory used in the arena.
 *
 * @note This function is thread-safe.
 */
size_t qcore_arena_used(qcore_arena_t *arena);

/**
 * @brief Closes an arena allocator.
 *
 * @param arena The arena to close.
 *
 * @note If the arena is already closed, this is a no-op.
 * @note This will free all memory allocated from the arena.
 * @note This function is thread-safe.
 */
void qcore_arena_close(qcore_arena_t *arena);

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_CORE_ARENA_H__
